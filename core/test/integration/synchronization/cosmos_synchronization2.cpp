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

class CosmosLikeWalletSynchronization2 : public BaseFixture {
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

TEST_F(CosmosLikeWalletSynchronization2, GetCurrentBlockWithExplorer) {
    mockHttp("CosmosLikeWalletSynchronization2.GetCurrentBlockWithExplorer");
    std::string address = "cosmos16xkkyj97z7r83sx45xwk9uwq0mj0zszlf6c6mq";

    auto block = uv::wait(explorer->getCurrentBlock());
    EXPECT_TRUE(block->hash.size() > 0);
    EXPECT_TRUE(block->height > 0);
}

TEST_F(CosmosLikeWalletSynchronization2, MediumXpubSynchronization) {
    mockHttp("CosmosLikeWalletSynchronization2.MediumXpubSynchronization");
    auto walletName = uuid::generate_uuid_v4();
#ifdef PG_SUPPORT
    const bool usePostgreSQL = true;
    auto poolConfig = DynamicObject::newInstance();
    poolConfig->putString(api::PoolConfiguration::DATABASE_NAME, "postgres://localhost:5432/test_db");
    auto pool = newDefaultPool(uuid::generate_uuid_v4(), "", poolConfig, usePostgreSQL);
#else
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
#endif

    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                                 "44'/<coin_type>'/<account>'/<node>/<address>");
        auto wallet = uv::wait(pool->createWallet(walletName, currencies::ATOM.name, configuration));
        std::set<std::string> emittedOperations;
        {
            auto accountInfo = uv::wait(wallet->getNextAccountCreationInfo());
            EXPECT_EQ(accountInfo.index, 0);

            accountInfo.publicKeys.push_back(hex::toByteArray(DEFAULT_HEX_PUB_KEY));
            auto account = ledger::testing::cosmos::createCosmosLikeAccount(
                wallet, accountInfo.index, accountInfo);

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });
            auto address = uv::wait(account->getFreshPublicAddresses())[0]->toString();
            EXPECT_EQ(address, DEFAULT_ADDRESS);
            auto eventBus = pool->getEventBus();
            eventBus->subscribe(getTestExecutionContext(), receiver);

            receiver.reset();
            receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);

                auto balance = uv::wait(account->getBalance());
                fmt::print("Balance: {} uatom\n", balance->toString());
                auto txBuilder = std::dynamic_pointer_cast<CosmosLikeTransactionBuilder>(account->buildTransaction());
                getTestExecutionContext()->stop();
            });

            auto restoreKey = account->getRestoreKey();
            auto bus = account->synchronize();
            bus->subscribe(getTestExecutionContext(), receiver);

            getTestExecutionContext()->waitUntilStopped();

            auto block = uv::wait(account->getLastBlock());
            fmt::print("Block height: {}\n", block.height);
            EXPECT_GT(block.height, 0);

            auto ops = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
            fmt::print("Ops: {}\n", ops.size());
            ASSERT_GT(ops.size(), 0);
            ASSERT_TRUE(ops[0]->getBlockHeight())
                << "The first operation should have a block height.";
            EXPECT_GT(ops[0]->getBlockHeight().value(), 0)
                << "The first operation should have a non 0 height.";
            EXPECT_LT(ops[0]->getBlockHeight().value(), block.height)
                << "The first operation should not have the same height as the last block.";

            const auto sequenceNumber = account->getInfo().sequence;
            const int sequence = std::atoi(sequenceNumber.c_str());
            EXPECT_GE(sequence, 1226) << "Sequence was at 1226 on 2020-03-26";

            const auto accountNumber = account->getInfo().accountNumber;
            EXPECT_EQ(accountNumber, "12850") << "Account number is a network constant for a given address";
        }
    }
}

TEST_F(CosmosLikeWalletSynchronization2, Balances) {
    mockHttp("CosmosLikeWalletSynchronization2.Balances");
    std::string hexPubKey =
        "0388459b2653519948b12492f1a0b464720110c147a8155d23d423a5cc3c21d89a";  // Obelix

    std::shared_ptr<AbstractWallet> wallet;
    std::shared_ptr<CosmosLikeAccount> account;

    setupTest(account, wallet, hexPubKey);

    const std::string address = account->getKeychain()->getAddress()->toBech32();
    const std::string mintscanExplorer = fmt::format("https://www.mintscan.io/account/{}", address);

    const auto totalBalance = uv::wait(account->getTotalBalance())->toLong();
    const auto delegatedBalance = uv::wait(account->getDelegatedBalance())->toLong();
    const auto pendingRewards = uv::wait(account->getPendingRewardsBalance())->toLong();
    const auto unbondingBalance = uv::wait(account->getUnbondingBalance())->toLong();
    const auto spendableBalance = uv::wait(account->getSpendableBalance())->toLong();

    EXPECT_LE(delegatedBalance, totalBalance)
        << "Delegated Coins fall under Total Balance, so delegatedBalance <= totalBalance";
    // Strict comparison here because
    // it's impossible to have pending rewards without
    // at least some delegatedBalance or unbonding balance
    EXPECT_LT(pendingRewards, totalBalance)
        << "Pending rewards fall under Total Balance, so pendingRewards < totalBalance";
    EXPECT_LE(unbondingBalance, totalBalance)
        << "Unbondings fall under Total Balance, so unbondingBalance <= totalBalance";
    EXPECT_LE(spendableBalance, totalBalance)
        << "Spendable Coins fall under Total Balance, so spendableBalance <= totalBalance";

    EXPECT_GE(pendingRewards, 1) << fmt::format(
        "Check {}  to see if the account really has <1 uatom of pending rewards", mintscanExplorer);
    EXPECT_GE(totalBalance, 1000000) << fmt::format(
        "Check {}  to see if the account really has <1 ATOM of total balance", mintscanExplorer);
    EXPECT_GE(delegatedBalance, 2800) << fmt::format(
        "Check {}  to see if the account really has <0.002800 ATOM of total delegations",
        mintscanExplorer);
    EXPECT_GE(spendableBalance, 1000) << fmt::format(
        "Check {}  to see if the account really has <0.001 ATOM of available (spendable) balance",
        mintscanExplorer);
    // Unbondings are moving too much to assert the amount.
}