#include "ProxyHttpClient.hpp"
#include "api/HttpRequest.hpp"
#include "api/HttpMethod.hpp"
#include <memory>
#include <sstream>
#include "api/Error.hpp"

namespace ledger {
    namespace core {
        namespace test {
            ProxyHttpClient::ProxyHttpClient(std::shared_ptr<api::HttpClient> httpClient) 
                : _httpClient(httpClient)
            {}

            void ProxyHttpClient::execute(const std::shared_ptr<api::HttpRequest>& request) {

                for (auto& cache : _cache ) {
                    bool matching = (request->getUrl() == cache.url);
                    if (matching && request->getMethod() == ledger::core::api::HttpMethod::POST) {
                        std::string body;
                        if (request->getBody().size() > 0) {
                            std::stringstream bodyStream;
                            for (auto byte : request->getBody()) {
                                bodyStream << (char )byte;
                            }
                            body = bodyStream.str();
                        }
                        matching = (body == cache.parameter);
                    }
                    if(matching){
                        std::cout << "get response from cache : " << request->getUrl() << std::endl;
                        request->complete(cache.response, std::experimental::nullopt);
                        return;
                    }
                }
                _httpClient->execute(request);
            }

            void ProxyHttpClient::addCache(const std::string& url, const std::string& parameter, const std::string& body) {
                _cache.push_back(Cache(url, parameter, FakeUrlConnection::fromString(body)));
            }

            void ProxyHttpClient::clearCache() {
                _cache.clear();
            }

            
        }
    }
}