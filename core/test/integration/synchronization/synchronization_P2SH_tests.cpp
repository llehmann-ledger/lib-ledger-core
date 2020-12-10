/*
 *
 * synchronization_tests
 * ledger-core
 *
 * Created by Khalil Bellakrid on 15/05/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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

#include <gtest/gtest.h>
#include "../BaseFixture.h"
#include <set>
#include <api/KeychainEngines.hpp>
#include <utils/DateUtils.hpp>
#include <wallet/bitcoin/database/BitcoinLikeAccountDatabaseHelper.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>
#include <Uuid.hpp>

class BitcoinLikeWalletP2SHSynchronization : public BaseFixture {

};

TEST_F(BitcoinLikeWalletP2SHSynchronization, MediumXpubSynchronization) {
    mockHttp("BitcoinLikeWalletP2SHSynchronization.MediumXpubSynchronization");
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");

        auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "bitcoin_testnet", configuration));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = uv::wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto account = createBitcoinLikeAccount(wallet, nextIndex, P2SH_XPUB_INFO);

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            auto eventBus = pool->getEventBus();
            eventBus->subscribe(getTestExecutionContext(),receiver);

            receiver.reset();
            receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                EXPECT_EQ(event->getCode(),
                          api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT);
                dispatcher->stop();
            });

            auto bus = account->synchronize();
            bus->subscribe(getTestExecutionContext(),receiver);

            dispatcher->waitUntilStopped();

            auto block = uv::wait(account->getLastBlock());
            auto blockHash = block.blockHash;
        }
    }
}

TEST_F(BitcoinLikeWalletP2SHSynchronization, SynchronizeOnceAtATime) {
    mockHttp("BitcoinLikeWalletP2SHSynchronization.SynchronizeOnceAtATime");
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");

        auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "bitcoin_testnet",configuration));
        {
            auto nextIndex = uv::wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);
            auto account = createBitcoinLikeAccount(wallet, nextIndex, P2SH_XPUB_INFO);
            auto eventBus = pool->getEventBus();
            eventBus->subscribe(getTestExecutionContext(),
                                           make_receiver([](const std::shared_ptr<api::Event> &event) {
                                               fmt::print("Received event {}\n", api::to_string(event->getCode()));
                                           }));
            auto bus = account->synchronize();
            bus->subscribe(getTestExecutionContext(),
                                              make_receiver([=](const std::shared_ptr<api::Event> &event) {
                                                  fmt::print("Received event {}\n", api::to_string(event->getCode()));
                                                  if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                                                      return;
                                                  EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                                                  EXPECT_EQ(event->getCode(),
                                                            api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT);
                                                  dispatcher->stop();
                                              }));
            EXPECT_EQ(bus, account->synchronize());
            dispatcher->waitUntilStopped();
        }
    }
}

TEST_F(BitcoinLikeWalletP2SHSynchronization, SynchronizeFromLastBlock) {
    mockHttp("BitcoinLikeWalletP2SHSynchronization.SynchronizeFromLastBlock");
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");

        auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "bitcoin_testnet",configuration));
        createBitcoinLikeAccount(wallet, 0, P2SH_XPUB_INFO);
        auto synchronize = [wallet, pool, this] (bool expectNewOp) {
            auto account = uv::wait(wallet->getAccount(0));
            auto numberOfOp = 0;

            auto receiverNumberOp = make_receiver([&numberOfOp](const std::shared_ptr<api::Event> &event) {
                numberOfOp += 1;
            });

            auto eventBus = pool->getEventBus();
            eventBus->subscribe(getTestExecutionContext(),receiverNumberOp);
            auto bus = account->synchronize();

            auto receiver = make_receiver([=, &numberOfOp](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;

                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                EXPECT_EQ(expectNewOp, (numberOfOp > 0));
                dispatcher->stop();
            });

            bus->subscribe(getTestExecutionContext(),receiver);
            auto newBus = account->synchronize();
            EXPECT_EQ(bus, newBus);
            dispatcher->waitUntilStopped();
            return bus;
        };

        auto b1 = synchronize(true);
        auto b2 = synchronize(false);
        EXPECT_NE(b1, b2);
    }
}