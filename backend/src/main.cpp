#include <drogon/drogon.h>

using namespace drogon;

int main() {
    app()
        .setLogLocalTime(true)
        .setLogLevel(trantor::Logger::kInfo)
        .setStaticFileHeaders({
            {"Access-Control-Allow-Origin", "*"},
            {"Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS"},
            {"Access-Control-Allow-Headers", "Content-Type, Authorization"},
        })
        .addListener("127.0.0.1", 8080);

    LOG_INFO << "Chat application started on http://127.0.0.1:8080";
    app().run();
    return 0;
}