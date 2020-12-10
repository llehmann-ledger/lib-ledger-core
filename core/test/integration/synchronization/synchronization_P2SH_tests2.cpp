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

class BitcoinLikeWalletP2SHSynchronization2 : public BaseFixture {

};

TEST_F(BitcoinLikeWalletP2SHSynchronization2, EraseDataSinceAfterSynchronization) {
    mockHttp("BitcoinLikeWalletP2SHSynchronization2.EraseDataSinceAfterSynchronization");
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    {
        //Set configuration
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                                 "49'/<coin_type>'/<account>'/<node>/<address>");
        //Create wallet
        auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "bitcoin_testnet", configuration));
        //Create account
        auto account = createBitcoinLikeAccount(wallet, 0, P2SH_XPUB_INFO);
        //Sync account
        auto bus = account->synchronize();
        auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
            fmt::print("Received event {}\n", api::to_string(event->getCode()));
            if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                return;

            EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
            dispatcher->stop();
        });
        bus->subscribe(getTestExecutionContext(),receiver);
        dispatcher->waitUntilStopped();

        auto accountCount = uv::wait(wallet->getAccountCount());
        EXPECT_EQ(accountCount, 1);
        auto accountFromWallet = uv::wait(wallet->getAccount(0));
        EXPECT_EQ(account, accountFromWallet);

        auto date = "2000-03-27T09:10:22Z";
        auto formatedDate = DateUtils::fromJSON(date);

        //Delete account
        auto code = uv::wait(wallet->eraseDataSince(formatedDate));
        EXPECT_EQ(code, api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

        //Check if account was successfully deleted
        auto newAccountCount = uv::wait(wallet->getAccountCount());
        EXPECT_EQ(newAccountCount, 0);
        {
            soci::session sql(pool->getDatabaseSessionPool()->getPool());
            BitcoinLikeAccountDatabaseEntry entry;
            auto result = BitcoinLikeAccountDatabaseHelper::queryAccount(sql, account->getAccountUid(), entry);
            EXPECT_EQ(result, false);
        }

        //Delete wallet
        auto walletCode = uv::wait(pool->eraseDataSince(formatedDate));
        EXPECT_EQ(walletCode, api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

        //Check if wallet was successfully deleted
        auto walletCount = uv::wait(pool->getWalletCount());
        EXPECT_EQ(walletCount, 0);
    }
}
TEST_F(BitcoinLikeWalletP2SHSynchronization2, TestNetSynchronization) {
    mockHttp("BitcoinLikeWalletP2SHSynchronization2.TestNetSynchronization");
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");
        auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "bitcoin_testnet",configuration));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = uv::wait(wallet->getNextAccountIndex());
            auto info = uv::wait(wallet->getNextExtendedKeyAccountCreationInfo());
            info.extendedKeys.push_back("tpubDCcvqEHx7prGddpWTfEviiew5YLMrrKy4oJbt14teJZenSi6AYMAs2SNXwYXFzkrNYwECSmobwxESxMCrpfqw4gsUt88bcr8iMrJmbb8P2q");
            EXPECT_EQ(nextIndex, 0);
            auto account = createBitcoinLikeAccount(wallet, nextIndex, info);
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
                dispatcher->stop();
            });

            auto bus = account->synchronize();
            bus->subscribe(getTestExecutionContext(),receiver);
            dispatcher->waitUntilStopped();
        }
    }
}

TEST_F(BitcoinLikeWalletP2SHSynchronization2, DecredParsingAndSerialization) {
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    {
        auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "decred",DynamicObject::newInstance()));
        auto strTx = "01000000016b9b4d4cdd2cf78907e62cddf31911ae4d4af1d89228ae4afc4459edee6a60c40100000000ffffff000240420f000000000000001976a9141d19445f397f6f0d3e2e6d741f61ba66b53886cf88acf0d31d000000000000001976a91415101bac61dca29add75996a0836a469dc8eee0788ac00000000ffffffff01000000000000000000000000ffffffff6a47304402200466bbc2aa8a742e85c3b68911502e73cdcb620ceaaa7a3cd199dbb4f8e9b969022063afeedd37d05e44b655a9de92eb36124acc045baf7b9e2941f81e41af91f1150121030ac79bab351084fdc82b4fa46eaa6a9cd2b5eb97ee93e367422bf47219b54a14";
        auto tx = BitcoinLikeTransactionApi::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(strTx), 0);
        EXPECT_EQ(hex::toString(tx->serialize()), strTx);
    }
}
