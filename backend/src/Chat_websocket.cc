#include "Chat_websocket.h"
#include <drogon/drogon.h>
#include <jsoncpp/json/json.h>
#include <codecvt>
#include <locale>
#include <regex>

using namespace std;

const size_t MAX_MESSAGE_LENGTH = 1000;
const size_t MAX_MESSAGE_PER_MINUTE = 20;
const size_t MAX_FILE_SIZE = 50 * 1024 * 1024; // 50 MB

// Static member definitions
unordered_map<string, set<WebSocketConnectionPtr>> ChatWebSocket::roomMap_;
unordered_map<WebSocketConnectionPtr, string> ChatWebSocket::connToRoom_;
unordered_map<WebSocketConnectionPtr, string> ChatWebSocket::connToUser_;
unordered_map<string, WebSocketConnectionPtr> ChatWebSocket::userToConn_;
unordered_map<WebSocketConnectionPtr, deque<chrono::steady_clock::time_point>> ChatWebSocket::rateLimitMap_;
mutex ChatWebSocket::roomMutex_;

// Helper struct: Validate incoming message (text or file)
struct ValidateMessage
{
    bool success;
    string type; // "text" or "file"
    string message;
    string fileType;
    string fileData;
    string error;
};

// helper: Check if a string is a valid UTF-8
bool isValideUTF8(const string &str)
{
    try
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        converter.from_bytes(str);
        return true;
    }
    catch (const std::range_error &)
    {
        return false;
    }
}

ValidateMessage validateIncomingMessage(const string &raw)
{
    Json::Value msgJson;
    Json::Reader reader;
    bool isJson = reader.parse(raw, msgJson);

    string msgType = "chat";
    if (isJson && msgJson.isMember("type"))
    {
        msgType = msgJson["type"].asString();
    }

    if (msgType == "file")
    {
        // File message: must have fileType and data (base64)
        if (!msgJson.isMember("fileType") || !msgJson.isMember("data"))
        {
            return {false, "file", "", "", "", "Invalid file message format"};
        }
        std::string fileType = msgJson["fileType"].asString();
        std::string fileData = msgJson["data"].asString();
        if (fileData.empty() || fileData.size() > MAX_FILE_SIZE)
        {
            return {false, "file", "", "", "", "File is empty or too large"};
        }
        // Basic base64 check (allows newlines)
        static const std::regex base64_regex("^[A-Za-z0-9+/=\\r\\n]+$");
        if (!std::regex_match(fileData, base64_regex))
        {
            return {false, "file", "", "", "", "Invalid base64 data"};
        }
        return {true, "file", "", fileType, fileData, ""};
    }
    else if (msgType == "chat")
    {
        // Text message
        std::string msgContent;
        if (isJson && msgJson.isMember("message"))
        {
            msgContent = msgJson["message"].asString();
        }
        else
        {
            msgContent = raw;
        }
        if (msgContent.empty() || msgContent.size() > MAX_MESSAGE_LENGTH)
        {
            return {false, "chat", "", "", "", "Message must be 1-" + std::to_string(MAX_MESSAGE_LENGTH) + " characters"};
        }
        if (!isValideUTF8(msgContent))
        {
            return {false, "chat", "", "", "", "Message is not valid UTF-8"};
        }
        return {true, "chat", msgContent, "", "", ""};
    }
    else if (msgType == "leave")
    {
        string roomId = msgJson.isMember("room") ? msgJson["room"].asString() : "";
        string username = msgJson.isMember("user") ? msgJson["user"].asString() : "";
        if (roomId.empty() || username.empty())
        {
            return {false, "leave", "", "", "", "Room and username are required for leave message"};
        }
        // Leave message is valid, just return success
        return {true, "leave", "", "", "", ""};
    }

    return {false, "", "", "", "", "Unknown message type: " + msgType};
}

void ChatWebSocket::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr &wsConn)
{
    bool reconnect = false;
    auto room = req->getParameter("roomId");
    auto username = req->getParameter("username");

    LOG_INFO << "[WS] Received connection request: roomId=" << room << ", username=" << username;

    if (room.empty() || username.empty())
    {
        LOG_ERROR << "RoomId or UserName not recieved";
        sendError(wsConn, "Room and username parameters are required.", 400);
        wsConn->shutdown();
        return;
    }

    {
        lock_guard<mutex> lock(roomMutex_);

        if (userToConn_.count(username))
        {
            auto oldConn = userToConn_[username];
            sendError(oldConn, "You have been disconnected due to a new connection.", 409);
            oldConn->shutdown();

            // clean up old connection
            auto oldRoom = connToRoom_[oldConn];
            roomMap_[oldRoom].erase(oldConn);
            connToRoom_.erase(oldConn);
            connToUser_.erase(oldConn);
            userToConn_.erase(username);
            rateLimitMap_.erase(oldConn);
            reconnect = true;
            LOG_INFO << "User " << username << " reconnected to room " << room;
        }

        roomMap_[room].insert(wsConn);
        connToRoom_[wsConn] = room;
        connToUser_[wsConn] = username;
        userToConn_[username] = wsConn;
        rateLimitMap_[wsConn] = deque<chrono::steady_clock::time_point>();
    }

    if(reconnect) return;

    // Notify other users in the room about the new user joining
    Json::Value joinMsg;
    joinMsg["type"] = "info";
    joinMsg["event"] = "join";
    joinMsg["user"] = username;
    joinMsg["timestamp"] = static_cast<Json::Int64>(chrono::system_clock::to_time_t(chrono::system_clock::now()));
    broadcastToRoom(room, joinMsg, wsConn);

    // Send confirmation message to the new user
    Json::Value confirMsg;
    confirMsg["type"] = "info";
    confirMsg["event"] = "joined";
    confirMsg["room"] = room;
    wsConn->sendJson(confirMsg);
}

void ChatWebSocket::handleNewMessage(const WebSocketConnectionPtr &wsConn, string &&message, const WebSocketMessageType &type)
{
    string roomId, username;

    {
        lock_guard<mutex> lock(roomMutex_);
        if (connToRoom_.find(wsConn) == connToRoom_.end() || connToUser_.find(wsConn) == connToUser_.end())
        {
            sendError(wsConn, "Not in any room", 400);
            return;
        }

        roomId = connToRoom_[wsConn];
        username = connToUser_[wsConn];
    }

    // Rate limiting
    {
        lock_guard<mutex> lock(roomMutex_);
        auto &time_stamps = rateLimitMap_[wsConn];
        auto now = chrono::steady_clock::now();
        while (!time_stamps.empty() &&
               chrono::duration_cast<chrono::minutes>(now - time_stamps.front()).count() >= 1)
        {
            time_stamps.pop_front();
        }

        if (time_stamps.size() >= MAX_MESSAGE_PER_MINUTE)
        {
            sendError(wsConn, "Rate limit exceeded. Please wait before sending more messages.", 429);
            return;
        }

        time_stamps.push_back(now);
    }

    // Message validation
    auto validation = validateIncomingMessage(message);
    if (!validation.success)
    {
        sendError(wsConn, validation.error, 400);
        return;
    }

    if (validation.type == "leave")
    {
        LOG_INFO << "User " << username << " has left room " << roomId;
        roomMap_[roomId].erase(wsConn);

        if (roomMap_[roomId].empty())
        {
            LOG_INFO << "Room " << roomId << " is now empty, removing it.";
            roomMap_.erase(roomId);
            Chat::removeRoom(roomId);
        }

        // Notify other users in the room about the user leaving
        Json::Value leaveMsg;
        leaveMsg["type"] = "info";
        leaveMsg["event"] = "leave";
        leaveMsg["user"] = username;
        leaveMsg["timestamp"] = static_cast<Json::Int64>(chrono::system_clock::to_time_t(chrono::system_clock::now()));
        broadcastToRoom(roomId, leaveMsg, nullptr);
        return;
    }

    Json::Value chatMsg;
    chatMsg["type"] = validation.type;
    chatMsg["user"] = username;
    chatMsg["timestamp"] = static_cast<Json::Int64>(chrono::system_clock::to_time_t(chrono::system_clock::now()));

    if (validation.type == "file")
    {
        chatMsg["fileType"] = validation.fileType;
        chatMsg["data"] = validation.fileData; // Base64 encoded data
    }
    else
    {
        LOG_INFO << "User " << username << " sent message in room " << roomId;
        chatMsg["message"] = validation.message; // Text message
    }

    broadcastToRoom(roomId, chatMsg, nullptr);
}

void ChatWebSocket::handleConnectionClosed(const WebSocketConnectionPtr &wsConn)
{
    string roomId, username;

    {
        lock_guard<mutex> lock(roomMutex_);

        auto it = connToRoom_.find(wsConn);
        auto uit = connToUser_.find(wsConn);

        if (it != connToRoom_.end() && uit != connToUser_.end())
        {
            roomId = it->second;
            username = uit->second;

            connToRoom_.erase(it);
            connToUser_.erase(uit);
        }
        else
        {
            return;
        }
    }
}

void ChatWebSocket::sendError(const WebSocketConnectionPtr &wsConn, const string &errorMsg, int ErrorCode)
{
    LOG_ERROR << errorMsg;
    Json::Value errorMsgJson;
    errorMsgJson["type"] = "error";
    errorMsgJson["message"] = errorMsg;
    errorMsgJson["code"] = ErrorCode;
    wsConn->sendJson(errorMsgJson);
}

void ChatWebSocket::broadcastToRoom(const string &roomId, const Json::Value &msg, const WebSocketConnectionPtr &exclude)
{
    if (roomId.empty())
    {
        LOG_ERROR << "Room ID is empty, cannot broadcast message.";
        return;
    }
    lock_guard<mutex> lock(roomMutex_);
    auto it = roomMap_.find(roomId);
    if (it != roomMap_.end())
    {
        for (const auto &conn : it->second)
        {
            if (conn != exclude)
            {
                conn->sendJson(msg);
            }
        }
    }
}

void ChatWebSocket::broadcastUserList(const string &roomId)
{
    if (roomId.empty())
    {
        LOG_ERROR << "Room ID is empty, cannot broadcast user list.";
        return;
    }
    lock_guard<mutex> lock(roomMutex_);
    auto it = roomMap_.find(roomId);
    if (it == roomMap_.end())
    {
        LOG_ERROR << "Room not found: " << roomId;
        return;
    }

    Json::Value userList;
    userList["type"] = "info";
    userList["event"] = "user_list";
    userList["users"] = Json::arrayValue;

    for (const auto &conn : it->second)
    {
        userList["users"].append(connToUser_[conn]);
    }

    broadcastToRoom(roomId, userList, nullptr);
}