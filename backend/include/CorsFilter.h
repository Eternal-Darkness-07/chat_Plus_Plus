#pragma once

#include <drogon/HttpFilter.h>
using namespace drogon;

class CorsFilter : public HttpFilter<CorsFilter>
{
public:
    static constexpr bool isAutoCreation = true;

    void doFilter(const HttpRequestPtr &req,
                  FilterCallback &&callback,
                  FilterChainCallback &&chainCallback) override;
};
