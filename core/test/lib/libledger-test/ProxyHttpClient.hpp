#pragma once
#include "api/HttpClient.hpp"
#include <vector>
#include "FakeUrlConnection.hpp"

namespace ledger {
    namespace core {
        namespace test {

            class ProxyHttpClient : public api::HttpClient {
            public:
                ProxyHttpClient(std::shared_ptr<api::HttpClient> httpClient);
                void execute(const std::shared_ptr<api::HttpRequest>& request) override;
                void addCache(const std::string& url, const std::string& parameter, const std::string& body);
            private:
                struct Cache {
                    Cache(const std::string& url_, const std::string& parameter_, std::shared_ptr<FakeUrlConnection> response_):
                        url(url_), parameter(parameter_), response(response_) 
                    {}
                    std::string url; 
                    std::string parameter;
                    std::shared_ptr<FakeUrlConnection> response; 
                };
                std::vector<Cache> _cache;
                std::shared_ptr<api::HttpClient> _httpClient;
            };

        }
    }
}
