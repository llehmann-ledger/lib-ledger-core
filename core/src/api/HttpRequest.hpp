// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from http_client.djinni

#pragma once

#include "../utils/optional.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace ledger { namespace core { namespace api {

class HttpUrlConnection;
enum class HttpMethod;
struct Error;

class HttpRequest {
public:
    virtual ~HttpRequest() {}

    virtual HttpMethod getMethod() = 0;

    virtual std::unordered_map<std::string, std::string> getHeaders() = 0;

    virtual std::vector<uint8_t> getBody() = 0;

    virtual std::string getUrl() = 0;

    virtual void complete(const std::shared_ptr<HttpUrlConnection> & response, const std::experimental::optional<Error> & error) = 0;
};

} } }  // namespace ledger::core::api
