#include "CorsFilter.h"

void CorsFilter::doFilter(const HttpRequestPtr &req,
                          FilterCallback &&callback,
                          FilterChainCallback &&chainCallback)
{
    auto resp = HttpResponse::newHttpResponse();
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");

    if (req->method() == HttpMethod::Options)
    {
        callback(resp);  // Preflight request ends here
    }
    else
    {
        chainCallback(); // Continue normal route handling
    }
}
