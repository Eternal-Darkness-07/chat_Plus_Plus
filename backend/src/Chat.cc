#include "Chat.h"
#include <jsoncpp/json/json.h>
#include <string>
#include <random>

set<string> Chat::usedRoomIds_;

void Chat::removeRoom(const string &roomId) {
    usedRoomIds_.erase(roomId);
    LOG_INFO << "Removed room ID: " << roomId << " from usedRoomIds_";
}

// Utility function to add CORS headers
static void addCORS(HttpResponsePtr &res, const std::string &methods = "POST, DELETE, OPTIONS") {
    res->addHeader("Access-Control-Allow-Origin", "*");
    res->addHeader("Access-Control-Allow-Methods", methods);
    res->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

void Chat::CreateRoomId(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    random_device rd;
    mt19937 rand(rd());

    string base = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    string roomId;

    do {
        roomId.clear();
        for (int i = 0; i < 16; ++i) {
            roomId += base[rand() % base.size()];
        }
    } while (usedRoomIds_.count(roomId));

    LOG_INFO << "Generated room ID: " << roomId;

    Json::Value response;
    response["roomId"] = roomId;

    HttpResponsePtr res = HttpResponse::newHttpJsonResponse(response);
    res->setStatusCode(k200OK);
    addCORS(res, "POST, OPTIONS");
    callback(res);
}

void Chat::ActiveRoomId(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();

    HttpResponsePtr res;
    if (!json || !(*json).isMember("roomId") || !(*json)["roomId"].isString()) {
        Json::Value error;
        error["error"] = "Invalid or missing 'roomId'";
        res = HttpResponse::newHttpJsonResponse(error);
        res->setStatusCode(k400BadRequest);
        addCORS(res, "POST, OPTIONS");
        callback(res);
        return;
    }

    string roomId = (*json)["roomId"].asString();
    usedRoomIds_.insert(roomId);

    LOG_INFO << "Activated room ID: " << roomId;

    Json::Value response;
    response["message"] = "Room ID activated successfully.";
    res = HttpResponse::newHttpJsonResponse(response);
    res->setStatusCode(k200OK);
    addCORS(res, "POST, OPTIONS");
    callback(res);
}