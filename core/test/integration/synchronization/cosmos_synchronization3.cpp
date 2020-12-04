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

class CosmosLikeWalletSynchronization3 : public BaseFixture {
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

TEST_F(CosmosLikeWalletSynchronization3, AllTransactionsSynchronization) {
    mockHttp("CosmosLikeWalletSynchronization3.AllTransactionsSynchronization");

    // FIXME Use an account that has all expected types of transactions
    std::string hexPubKey = "0388459b2653519948b12492f1a0b464720110c147a8155d23d423a5cc3c21d89a"; // Obelix

    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, hexPubKey);

    performSynchro(account);

    auto ops = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
    fmt::print("Ops: {}\n", ops.size());
    EXPECT_GT(ops.size(), 0);

    auto foundMsgSend = false;
    auto foundMsgDelegate = false;
    auto foundMsgBeginRedelegate = false;
    auto foundMsgUndelegate = false;
    auto foundMsgSubmitProposal = false;
    auto foundMsgVote = false;
    auto foundMsgDeposit = false;
    auto foundMsgWithdrawDelegationReward = false;
    auto foundMsgMultiSend = false;
    auto foundMsgCreateValidator = false;
    auto foundMsgEditValidator = false;
    auto foundMsgSetWithdrawAddress = false;
    auto foundMsgWithdrawDelegatorReward = false;
    auto foundMsgWithdrawValidatorCommission = false;
    auto foundMsgUnjail = false;

    for (auto op : ops) {
        auto cosmosOp = op->asCosmosLikeOperation();

        std::cout << "Found operation type: " << cosmosOp->getMessage()->getRawMessageType() << std::endl;

        switch (cosmosOp->getMessage()->getMessageType()) {
            case api::CosmosLikeMsgType::MSGSEND: foundMsgSend = true; break;
            case api::CosmosLikeMsgType::MSGDELEGATE: foundMsgDelegate = true; break;
            case api::CosmosLikeMsgType::MSGBEGINREDELEGATE: foundMsgBeginRedelegate = true; break;
            case api::CosmosLikeMsgType::MSGUNDELEGATE: foundMsgUndelegate = true; break;
            case api::CosmosLikeMsgType::MSGSUBMITPROPOSAL: foundMsgSubmitProposal = true; break;
            case api::CosmosLikeMsgType::MSGVOTE: foundMsgVote = true; break;
            case api::CosmosLikeMsgType::MSGDEPOSIT: foundMsgDeposit = true; break;
            case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATIONREWARD: foundMsgWithdrawDelegationReward = true; break;
            case api::CosmosLikeMsgType::MSGMULTISEND: foundMsgMultiSend = true; break;
            case api::CosmosLikeMsgType::MSGCREATEVALIDATOR: foundMsgCreateValidator = true; break;
            case api::CosmosLikeMsgType::MSGEDITVALIDATOR: foundMsgEditValidator = true; break;
            case api::CosmosLikeMsgType::MSGSETWITHDRAWADDRESS: foundMsgSetWithdrawAddress = true; break;
            case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATORREWARD: foundMsgWithdrawDelegatorReward = true; break;
            case api::CosmosLikeMsgType::MSGWITHDRAWVALIDATORCOMMISSION: foundMsgWithdrawValidatorCommission = true; break;
            case api::CosmosLikeMsgType::MSGUNJAIL: foundMsgUnjail = true; break;
            case api::CosmosLikeMsgType::UNSUPPORTED: break;
        }
    }

    EXPECT_TRUE(foundMsgSend);
    EXPECT_TRUE(foundMsgDelegate);
    EXPECT_TRUE(foundMsgBeginRedelegate);
    EXPECT_TRUE(foundMsgUndelegate);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgSubmitProposal);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgVote);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgDeposit);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgWithdrawDelegationReward);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgMultiSend);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgCreateValidator);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgEditValidator);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgSetWithdrawAddress);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgWithdrawDelegatorReward);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgWithdrawValidatorCommission);
    // Deactivated because target account does not have this type of operation
    // EXPECT_TRUE(foundMsgUnjail);
}

TEST_F(CosmosLikeWalletSynchronization3, ValidatorSet) {
    mockHttp("CosmosLikeWalletSynchronization3.ValidatorSet");

    // This test assumes that HuobiPool and BinanceStaking are always in the validator set
    const auto huobi_pool_address = "cosmosvaloper1kn3wugetjuy4zetlq6wadchfhvu3x740ae6z6x";
    bool foundHuobi = false;
    const auto binance_staking_pub_address = "cosmosvalconspub1zcjduepqtw8862dhw8uty58d6t2szfd6kqram2t234zjteaaeem6l45wclaq8l60gn";
    bool foundBinance = false;

    auto set = uv::wait(explorer->getActiveValidatorSet());

    EXPECT_EQ(set.size(), 125) << "currently cosmoshub-3 has 125 active validators";

    for (const auto & validator : set) {
        if (validator.consensusPubkey == binance_staking_pub_address) {
            foundBinance = true;
        } else if (validator.operatorAddress == huobi_pool_address){
            foundHuobi = true;
        } else {
            continue;
        }
    }

    EXPECT_TRUE(foundHuobi) << "Huobi Pool is expected to always be in the validator set";
    EXPECT_TRUE(foundBinance) << "Binance Staking is expected to always be in the validator set";
}

//Internal Server Error at the time of writing, thus could not cache the http answer
TEST_F(CosmosLikeWalletSynchronization3, DISABLED_ValidatorInfo) {
    
    // This test assumes that HuobiPool and BinanceStaking are always in the validator set
    const auto bisonTrailsAddress = "cosmosvaloper1uxh465053nq3at4dn0jywgwq3s9sme3la3drx6";
    const auto bisonTrailsValConsPubAddress = "cosmosvalconspub1zcjduepqc5y2du793cjut0cn6v7thp3xlvphggk6rt2dhw9ekjla5wtkm7nstmv5vy";
    const auto mintscanAddress = fmt::format("https://www.mintscan.io/validators/{}", bisonTrailsAddress);


    auto valInfo = uv::wait(explorer->getValidatorInfo(bisonTrailsAddress));
    ASSERT_EQ(valInfo.operatorAddress, bisonTrailsAddress) << "We should fetch the expected validator";
    ASSERT_EQ(valInfo.consensusPubkey, bisonTrailsValConsPubAddress) << "We should fetch the expected validator";


    EXPECT_EQ(valInfo.validatorDetails.moniker, "Bison Trails");
    ASSERT_TRUE(valInfo.validatorDetails.identity);
    EXPECT_EQ(valInfo.validatorDetails.identity.value(), "A296556FF603197C");
    ASSERT_TRUE(valInfo.validatorDetails.website);
    EXPECT_EQ(valInfo.validatorDetails.website.value(), "https://bisontrails.co");
    ASSERT_TRUE(valInfo.validatorDetails.details);
    EXPECT_EQ(valInfo.validatorDetails.details.value(), "Bison Trails is the easiest way to run infrastructure on multiple blockchains.");

    EXPECT_EQ(valInfo.commission.rates.maxRate, "0.500000000000000000");
    EXPECT_EQ(valInfo.commission.rates.maxChangeRate, "0.010000000000000000");
    EXPECT_GE(valInfo.commission.updateTime, DateUtils::fromJSON("2019-03-13T23:00:00Z")) <<
        "As of this test writing, last update was on 2019-03-13T23:00:00Z. So updateTime should be at least as recent as this timestamp.";
    // Specify locale for std::stof -- Also, why we just pass strings instead of parsing as much as possible
    setlocale(LC_NUMERIC, "C");
    EXPECT_LE(std::stof(valInfo.commission.rates.rate), std::stof(valInfo.commission.rates.maxRate));

    EXPECT_EQ(valInfo.unbondingHeight, 0) <<
        fmt::format("Expecting BisonTrails to never have been jailed. Check {} to see if the assertion holds", mintscanAddress);
    EXPECT_FALSE(valInfo.unbondingTime) <<
        fmt::format("Expecting BisonTrails to never have been jailed. Check {} to see if the assertion holds", mintscanAddress);
    EXPECT_EQ(valInfo.minSelfDelegation, "1") <<
        fmt::format("Expecting BisonTrails to have '1' minimum self delegation. Check {} to see if the assertion holds", mintscanAddress);
    EXPECT_FALSE(valInfo.jailed) <<
        fmt::format("Expecting BisonTrails to never have been jailed. Check {} to see if the assertion holds", mintscanAddress);
    //EXPECT_GE(BigInt::fromString(valInfo.votingPower).toUint64(), BigInt::fromString("400000000000").toUint64()) <<
    //    fmt::format("Expecting BisonTrails voting power to be > 400_000 ATOM. Check {} to see if the assertion holds", mintscanAddress);
    EXPECT_EQ(valInfo.activeStatus, 2) <<
        fmt::format("Expecting BisonTrails to be active (and that currently the explorer returns 2 for this status). Check {} to see if the assertion holds", mintscanAddress);

    EXPECT_FALSE(valInfo.distInfo.validatorCommission.empty()) << "Bison Trails should have accumulated at least *some* validator commission";
    EXPECT_FALSE(valInfo.distInfo.selfBondRewards.empty()) << "Bison Trails should have accumulated at least *some* self delegation rewards";

    EXPECT_FALSE(valInfo.signInfo.tombstoned) << "Bison Trails is not expected to be tombstoned";
    EXPECT_GE(valInfo.signInfo.missedBlocksCounter, 0) << "This value cannot be negative";
    EXPECT_GE(valInfo.signInfo.jailedUntil, DateUtils::fromJSON("1970-01-01T00:00:00Z")) << "This value cannot be before epoch";
}

TEST_F(CosmosLikeWalletSynchronization3, BalanceHistoryOperationQuery) {
    mockHttp("CosmosLikeWalletSynchronization3.BalanceHistoryOperationQuery");

    std::string hexPubKey = "0388459b2653519948b12492f1a0b464720110c147a8155d23d423a5cc3c21d89a"; // Obelix

    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, hexPubKey);

    performSynchro(account);

    const auto &uid = account->getAccountUid();
    soci::session sql(wallet->getDatabase()->getPool());
    std::vector<Operation> operations;

    auto keychain = account->getKeychain();
    std::function<bool(const std::string &)> filter = [&keychain](const std::string addr) -> bool {
        return keychain->contains(addr);
    };

    //Get operations related to an account
    CosmosLikeOperationDatabaseHelper::queryOperations(
        sql,
        uid,
        operations,
        filter);

    ASSERT_GE(operations.size(), 17) << "As of 2020-03-19, there are 17 operations picked up by the query";
}