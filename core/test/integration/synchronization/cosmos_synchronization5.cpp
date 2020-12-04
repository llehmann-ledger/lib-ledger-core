/*
 *
 * cosmos_synchronization.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/06/2019.
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

#include <iostream>
#include <chrono>
#include <set>
#include <numeric>

#include <test/cosmos/Fixtures.hpp>
#include "../BaseFixture.h"

#include <gtest/gtest.h>

#include <api/Configuration.hpp>
#include <api/KeychainEngines.hpp>
#include <api/PoolConfiguration.hpp>
#include <utils/DateUtils.hpp>
#include <utils/hex.h>
#include <collections/DynamicObject.hpp>
#include <math/BigInt.h>

#include <wallet/cosmos/explorers/GaiaCosmosLikeBlockchainExplorer.hpp>
#include <wallet/cosmos/CosmosNetworks.hpp>
#include <api/CosmosConfigurationDefaults.hpp>
#include <cosmos/CosmosLikeExtendedPublicKey.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <wallet/cosmos/transaction_builders/CosmosLikeTransactionBuilder.hpp>
#include <wallet/cosmos/CosmosLikeWallet.hpp>
#include <wallet/cosmos/CosmosLikeOperationQuery.hpp>
#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <cosmos/bech32/CosmosBech32.hpp>
#include <Uuid.hpp>

#include <wallet/cosmos/database/CosmosLikeOperationDatabaseHelper.hpp>

using namespace std;
using namespace ledger::core;
using namespace ledger::testing::cosmos;

extern api::CosmosLikeNetworkParameters COSMOS_PARAMS;

class CosmosLikeWalletSynchronization5 : public BaseFixture {
public:
    void SetUp() override {
        BaseFixture::SetUp();
        auto worker = dispatcher->getSerialExecutionContext("worker");
        auto client = std::make_shared<HttpClient>(
            api::CosmosConfigurationDefaults::COSMOS_DEFAULT_API_ENDPOINT, http, worker);

#ifdef PG_SUPPORT
    const bool usePostgreSQL = true;
    auto poolConfig = DynamicObject::newInstance();
    poolConfig->putString(api::PoolConfiguration::DATABASE_NAME, "postgres://localhost:5432/test_db");
    pool = newDefaultPool(uuid::generate_uuid_v4(), "", poolConfig, usePostgreSQL);
#else
    pool = newDefaultPool(uuid::generate_uuid_v4());
#endif

        explorer = std::make_shared<GaiaCosmosLikeBlockchainExplorer>(
            worker,
            client,
            COSMOS_PARAMS,
            std::make_shared<DynamicObject>());
    }

    void setupTest(std::shared_ptr<CosmosLikeAccount>& account,
                   std::shared_ptr<AbstractWallet>& wallet,
                   const std::string& pubKey) {

        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'/<node>/<address>");
        wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "cosmos", configuration));

        auto accountInfo = uv::wait(wallet->getNextAccountCreationInfo());
        EXPECT_EQ(accountInfo.index, 0);
        accountInfo.publicKeys.push_back(hex::toByteArray(pubKey));

        account = ledger::testing::cosmos::createCosmosLikeAccount(wallet, accountInfo.index, accountInfo);
    }

    void performSynchro(const std::shared_ptr<CosmosLikeAccount>& account) {
        auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
            fmt::print("Received event {}\n", api::to_string(event->getCode()));
            if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED) {
                return;
            }
            EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);

            auto balance = uv::wait(account->getBalance());
            fmt::print("Balance: {} uatom\n", balance->toString());

            auto block = uv::wait(account->getLastBlock());
            fmt::print("Block height: {}\n", block.height);
            EXPECT_GT(block.height, 0);

            getTestExecutionContext()->stop();
        });

        auto bus = account->synchronize();
        bus->subscribe(getTestExecutionContext(), receiver);
        getTestExecutionContext()->waitUntilStopped();
     }

    void TearDown() override {
        uv::wait(pool->eraseDataSince(std::chrono::time_point<std::chrono::system_clock>{}));
        BaseFixture::TearDown();
    }

    std::shared_ptr<WalletPool> pool;
    std::shared_ptr<GaiaCosmosLikeBlockchainExplorer> explorer;
};

TEST_F(CosmosLikeWalletSynchronization5, GetAccountDelegations) {
    mockHttp("CosmosLikeWalletSynchronization5.GetAccountDelegations");

    std::string hexPubKey = "0388459b2653519948b12492f1a0b464720110c147a8155d23d423a5cc3c21d89a"; // Obelix

    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, hexPubKey);

    auto delegations = uv::wait(account->getDelegations());
    EXPECT_GE(delegations.size(), 2);

    BigInt delegatedAmount;
    for (auto& delegation : delegations) {
        delegatedAmount = delegatedAmount + *(std::dynamic_pointer_cast<ledger::core::Amount>(delegation->getDelegatedAmount())->value());
    }
    EXPECT_GE(delegatedAmount.toUint64(), 1000000UL); // 1 ATOM

}

TEST_F(CosmosLikeWalletSynchronization5, GetAccountPendingRewards) {
    mockHttp("CosmosLikeWalletSynchronization5.GetAccountPendingRewards");

    std::string hexPubKey = "0388459b2653519948b12492f1a0b464720110c147a8155d23d423a5cc3c21d89a"; // Obelix

    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, hexPubKey);

    auto rewards = uv::wait(account->getPendingRewards());
    EXPECT_GE(rewards.size(), 2);

    BigInt pendingReward;
    for (auto& reward : rewards) {
        pendingReward = pendingReward + *(std::dynamic_pointer_cast<ledger::core::Amount>(reward->getRewardAmount())->value());
    }
    EXPECT_GE(pendingReward.toUint64(), 1000UL); // 1000 uATOM

}

//Disabled because even with http anwsers being cached, the test fail. Maybe the address hasn't any pending unboundings anymore ?
TEST_F(CosmosLikeWalletSynchronization5, DISABLED_PendingUnbondings) {
    mockHttp("CosmosLikeWalletSynchronization5.PendingUnbondings");

    std::string hexPubKey =
        "0388459b2653519948b12492f1a0b464720110c147a8155d23d423a5cc3c21d89a";  // Obelix

    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, hexPubKey);
    const std::string address = account->getKeychain()->getAddress()->toBech32();
    const std::string mintscanExplorer = fmt::format("https://www.mintscan.io/account/{}", address);

    // First synchro
    performSynchro(account);

    auto unbondings = uv::wait(account->getUnbondings());
    EXPECT_GE(unbondings.size(), 1) << fmt::format(
        "Expecting at least 1 unbonding here for Obelix (explorer link : {}).", mintscanExplorer);
}