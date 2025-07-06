#pragma once

#include <drogon/WebSocketController.h>
#include "Chat.h"

using namespace drogon;

class ChatWebSocket : public drogon::WebSocketController<ChatWebSocket>
{
public:

    void handleNewMessage(const WebSocketConnectionPtr &, std::string &&, const WebSocketMessageType &) override;
    void handleNewConnection(const HttpRequestPtr &, const WebSocketConnectionPtr &) override;
    void handleConnectionClosed(const WebSocketConnectionPtr &) override;
    WS_PATH_LIST_BEGIN
        WS_PATH_ADD("/ws/chat", "CorsFilter");
    WS_PATH_LIST_END

    void sendError(const WebSocketConnectionPtr &wsConn, const string &errorMsg, int errorCode);
    void broadcastToRoom(const string &roomId, const Json::Value &message, const WebSocketConnectionPtr &excludeConn);
    void broadcastUserList(const string &roomId);

private:
    static unordered_map<string, set<WebSocketConnectionPtr>> roomMap_;
    static unordered_map<WebSocketConnectionPtr, string> connToRoom_;
    static unordered_map<WebSocketConnectionPtr, string> connToUser_;
    static unordered_map<string, WebSocketConnectionPtr> userToConn_;
    static unordered_map<WebSocketConnectionPtr, deque<chrono::steady_clock::time_point>> rateLimitMap_;
    static mutex roomMutex_;
};