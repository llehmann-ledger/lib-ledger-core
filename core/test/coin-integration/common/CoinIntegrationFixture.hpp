/*
 *
 * CoinIntegrationFixture.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 18/02/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef LEDGER_CORE_COININTEGRATIONFIXTURE_HPP
#define LEDGER_CORE_COININTEGRATIONFIXTURE_HPP

#include <gtest/gtest.h>
#include <UvThreadDispatcher.hpp>
#include <src/database/DatabaseSessionPool.hpp>
#include <NativePathResolver.hpp>
#include <unordered_set>
#include <src/wallet/pool/WalletPool.hpp>
#include <CoutLogPrinter.hpp>
#include <src/api/DynamicObject.hpp>
#include <wallet/common/CurrencyBuilder.hpp>
#include <wallet/bitcoin/BitcoinLikeWallet.hpp>
#include <wallet/bitcoin/database/BitcoinLikeWalletDatabase.h>
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <wallet/common/database/AccountDatabaseHelper.h>
#include <wallet/pool/database/PoolDatabaseHelper.hpp>
#include <utils/JSONUtils.h>
#include <wallet/bitcoin/explorers/api/TransactionParser.hpp>
#include <wallet/bitcoin/BitcoinLikeAccount.hpp>
#include <wallet/ethereum/EthereumLikeAccount.h>
#include <api/BitcoinLikeOperation.hpp>
#include <api/BitcoinLikeTransaction.hpp>
#include <api/BitcoinLikeInput.hpp>
#include <api/BitcoinLikeOutput.hpp>
#include <api/BigInt.hpp>
#include <CppHttpLibClient.hpp>
#include <ProxyHttpClient.hpp>
#include <soci.h>
#include <api/Account.hpp>
#include <api/BitcoinLikeAccount.hpp>
#include <FakeWebSocketClient.h>
#include <OpenSSLRandomNumberGenerator.hpp>
#include <utils/FilesystemUtils.hpp>
#include <utils/hex.h>
#include "../integration/IntegrationEnvironment.h"
#include <events/EventPublisher.hpp>
#include <MemPreferencesBackend.hpp>
#include <experimental/filesystem>
#include <debug/logger.hpp>

namespace fs = std::experimental::filesystem::v1;
using namespace ledger::core; // don't do this at home. Only for testing contexts
using namespace ledger::core::test;

class LambdaEventReceiver : public api::EventReceiver {
public:
    LambdaEventReceiver(std::function<void (const std::shared_ptr<api::Event> &)> f,
                                            std::shared_ptr<uv::SequentialExecutionContext> &exec_context,
                                            std::shared_ptr<NativePathResolver> &resolver,
                                            std::shared_ptr<CoutLogPrinter> &printer) {
        _function = f;
        _exec_context = exec_context;
        _resolver = resolver;
        _printer = printer;
    }
    void onEvent(const std::shared_ptr<api::Event> &event) override {
        try {
            _function(event);
        } catch (Exception& e) {
            std::shared_ptr<spdlog::logger> logger = logger::create(
                "lambdaevent-l",
                _exec_context,
                _resolver,
                _printer,
                logger::DEFAULT_MAX_SIZE,
                true
            );
            logger->error(e.getMessage());
            _exec_context->stop();
        }
    }

private:
    std::function<void (const std::shared_ptr<api::Event> &)> _function;
    std::shared_ptr<uv::SequentialExecutionContext> _exec_context;
    std::shared_ptr<NativePathResolver> _resolver;
    std::shared_ptr<CoutLogPrinter> _printer;
};

enum SynchronizationResult {OLD_ACCOUNT, NEW_ACCOUNT};

template <typename Wallet, typename Account>
class CoinIntegrationFixture : public ::testing::Test {
public:
    void SetUp() override {
        ::testing::Test::SetUp();
        ledger::qt::FilesystemUtils::clearFs(IntegrationEnvironment::getInstance()->getApplicationDirPath());
        dispatcher = std::make_shared<uv::UvThreadDispatcher>();
        resolver = std::make_shared<NativePathResolver>(IntegrationEnvironment::getInstance()->getApplicationDirPath());
        backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
        printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
        auto client = std::make_shared<CppHttpLibClient>(dispatcher->getMainExecutionContext());
        client->setGenerateCacheFile(true);
        http = std::make_shared<ProxyHttpClient>(client);
        ws = std::make_shared<FakeWebSocketClient>();
        rng = std::make_shared<OpenSSLRandomNumberGenerator>();
    }

    void TearDown() override {
        ::testing::Test::TearDown();
        resolver->clean();
    }

    virtual std::shared_ptr<WalletPool> newPool(std::string poolName = "my_pool") {
        return WalletPool::newInstance(
                poolName,
                "",
                http,
                ws,
                resolver,
                printer,
                dispatcher,
                rng,
                backend,
                api::DynamicObject::newInstance(),
                std::make_shared<MemPreferencesBackend>(),
                std::make_shared<MemPreferencesBackend>()
        );
    }

    std::shared_ptr<uv::SequentialExecutionContext> getTestExecutionContext()
    {
        return std::dynamic_pointer_cast<uv::SequentialExecutionContext>(dispatcher->getSerialExecutionContext("__test__"));
    }

    void injectCurrency(const std::shared_ptr<WalletPool>& pool, const api::Currency& currency) {
        uv::wait(pool->addCurrency(currency));
    }

    std::shared_ptr<Wallet> newWallet(  const std::shared_ptr<WalletPool>& pool,
                                        const std::string& walletName,
                                        const std::string& currencyName,
                                        const std::shared_ptr<api::DynamicObject> &configuration
                                     ) {
        return std::dynamic_pointer_cast<Wallet>(uv::wait(pool->createWallet(walletName, currencyName, configuration)));
    }

    std::shared_ptr<Account> newAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                                 int32_t index,
                                                                 const api::AccountCreationInfo &info) {
        auto i = info;
        i.index = index;
        return std::dynamic_pointer_cast<Account>(uv::wait(wallet->newAccountWithInfo(i)));
    }

    std::shared_ptr<Account> newAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                                 int32_t index,
                                                                 const api::ExtendedKeyAccountCreationInfo& info) {
        auto i = info;
        i.index = index;
        return std::dynamic_pointer_cast<Account>(uv::wait(wallet->newAccountWithExtendedKeyInfo(i)));
    }


    Try<SynchronizationResult> synchronizeAccount(const std::shared_ptr<AbstractAccount>& account) {
        Promise<SynchronizationResult> p;
        auto bus = account->synchronize();
        bus->subscribe(dispatcher->getSerialExecutionContext("callback"),
                       make_receiver([=](const std::shared_ptr<api::Event> &event) mutable {
                           if (event->getCode() == api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT) {
                               p.success(SynchronizationResult::NEW_ACCOUNT);
                           } else if (event->getCode() == api::EventCode::SYNCHRONIZATION_SUCCEED) {
                               p.success(SynchronizationResult::OLD_ACCOUNT);
                           } else if (event->getCode() == api::EventCode::SYNCHRONIZATION_FAILED) {
                               api::ErrorCode code = api::ErrorCode::UNKNOWN;
                               std::string reason = "Unknown error";
                               auto payload = event->getPayload();
                               if (payload && payload->getString(api::Account::EV_SYNC_ERROR_CODE)) {
                                   code = api::from_string<api::ErrorCode>(payload->getString(api::Account::EV_SYNC_ERROR_CODE).value());
                               }
                               if (payload && payload->getString(api::Account::EV_SYNC_ERROR_MESSAGE)) {
                                   reason = payload->getString(api::Account::EV_SYNC_ERROR_MESSAGE).value();
                               }
                               p.failure(make_exception(code, reason));
                           }
                       }));
        return uv::wait(p.getFuture());
    }

    void mockHttp(const std::string& testname) {
        const char* env = std::getenv("MOCK_HTTP_FOLDER");
        if (env != nullptr) { 
            http->clearCache();
            std::string path = env;
            path += "/";
            path += testname;
            if (fs::is_directory(path)) {
                for (const auto& file : fs::recursive_directory_iterator(path)) {
                    std::string currentFile = file.path();
                    if (!fs::is_directory(currentFile) && currentFile.find(".txt") != std::string::npos) {
                        std::ifstream input(currentFile);    
                        std::string url, parameter, body;
                        std::getline(input, url);
                        std::getline(input, parameter);
                        std::string line;
                        while (std::getline(input, line)) {
                            body += line;
                        }
                        http->addCache(url, parameter, body);
                    }
                }
            }
        }
    }

    std::shared_ptr<LambdaEventReceiver> make_receiver(std::function<void (const std::shared_ptr<api::Event> &)> f) {
        auto exec_context = std::dynamic_pointer_cast<uv::SequentialExecutionContext>(dispatcher->getMainExecutionContext());
        return std::make_shared<LambdaEventReceiver>(f, exec_context, resolver, printer);
    }

    std::shared_ptr<LambdaEventReceiver> make_receiver(std::function<void (const std::shared_ptr<api::Event> &)> f, std::shared_ptr<uv::SequentialExecutionContext> exec_context) {
        return std::make_shared<LambdaEventReceiver>(f, exec_context, resolver, printer);
    }

    std::shared_ptr<LambdaEventReceiver> make_promise_receiver(
            Promise<Unit>& promise,
            const std::vector<api::EventCode> &successCodes,
            const std::vector<api::EventCode> &failureCodes
    ) {
        return make_receiver([=] (const std::shared_ptr<api::Event> &event) mutable {
            for (const auto& code : successCodes) {
                if (code == event->getCode()) {
                    promise.success(unit);
                    return;
                }
            }
            for (const auto& code : failureCodes) {
                if (code == event->getCode()) {
                    promise.failure(make_exception(api::ErrorCode::RUNTIME_ERROR, event->getPayload()->dump()));
                    return;
                }
            }
        });
    }

    std::shared_ptr<uv::UvThreadDispatcher> dispatcher;
    std::shared_ptr<NativePathResolver> resolver;
    std::shared_ptr<DatabaseBackend> backend;
    std::shared_ptr<CoutLogPrinter> printer;
    std::shared_ptr<ProxyHttpClient> http;
    std::shared_ptr<FakeWebSocketClient> ws;
    std::shared_ptr<OpenSSLRandomNumberGenerator> rng;

};

#endif //LEDGER_CORE_COININTEGRATIONFIXTURE_HPP
