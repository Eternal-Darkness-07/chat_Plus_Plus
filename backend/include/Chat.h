#pragma once

#include <drogon/HttpController.h>

using namespace drogon;
using namespace std;

class Chat: public drogon::HttpController<Chat>
{
  public:
    METHOD_LIST_BEGIN
      METHOD_ADD(Chat::CreateRoomId, "/CreateRoomId", Post, Options, "CorsFilter"); 
      METHOD_ADD(Chat::ActiveRoomId, "/ActiveRoomId", Post, Options, "CorsFilter"); 
    METHOD_LIST_END

    void CreateRoomId(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback);
    void ActiveRoomId(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback);
    
    static void removeRoom(const string &roomId);

  private:
    static set<string> usedRoomIds_;
};