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

api::CosmosLikeNetworkParameters COSMOS_PARAMS = networks::getCosmosLikeNetworkParameters("atom");

class CosmosLikeWalletSynchronization : public BaseFixture {
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

TEST_F(CosmosLikeWalletSynchronization, GetAccountWithExplorer) {
    mockHttp("CosmosLikeWalletSynchronization.GetAccountWithExplorer");
    auto account = uv::wait(explorer->getAccount(DEFAULT_ADDRESS));
    EXPECT_EQ(account->address, DEFAULT_ADDRESS);
    EXPECT_EQ(account->accountNumber, "12850");
    EXPECT_EQ(account->withdrawAddress, DEFAULT_ADDRESS)
        << "Withdraw address has not been modified on this address";
}


TEST_F(CosmosLikeWalletSynchronization, InternalFeesMessageInTransaction) {
    mockHttp("CosmosLikeWalletSynchronization.InternalFeesMessageInTransaction");
    /// This transaction contains 2 messages. One internal message to store fees
    /// is added after the transaction is retrieved from the network, hence the
    /// transaction should contain 3 messages, the last one being the one added
    /// specifically for the fees.
    const auto transaction = uv::wait(explorer->getTransactionByHash("0DBFC4E8E9E5A64C2C9B5EAAAA0422D99A61CFC5354E15002A061E91200DC2D6"));

    ASSERT_NE(transaction, nullptr);
    EXPECT_EQ(transaction->messages.size(), 3);
    EXPECT_EQ(transaction->logs.size(), 3);
    const auto feeMessageIndex = transaction->messages.size() - 1;
    const auto& feeMessage = transaction->messages[feeMessageIndex];
    const auto& feeMessageLog = transaction->logs[feeMessageIndex];

    auto reduceAmounts = [](const std::vector<cosmos::Coin>& coins) {
        return std::accumulate(std::begin(coins), std::end(coins), BigInt::ZERO,
                [](BigInt s, const cosmos::Coin& coin) {
                    return s + BigInt::fromString(coin.amount);
                });
    };

    // check the internal fees message content
    ASSERT_EQ(feeMessage.type, cosmos::constants::kMsgFees);
    const auto& feeMessageContent = boost::get<cosmos::MsgFees>(feeMessage.content);
    EXPECT_EQ(reduceAmounts(transaction->fee.amount), BigInt::fromString(feeMessageContent.fees.amount));
    EXPECT_EQ(feeMessageContent.payerAddress, DEFAULT_ADDRESS);

    // check the logs for the fees message
    EXPECT_EQ(feeMessageLog.messageIndex, feeMessageIndex);
    EXPECT_EQ(feeMessageLog.success, true);
    EXPECT_EQ(feeMessageLog.log, "");
}


TEST_F(CosmosLikeWalletSynchronization, GetWithdrawDelegationRewardWithExplorer) {
    mockHttp("CosmosLikeWalletSynchronization.GetWithdrawDelegationRewardWithExplorer");
    auto filter = GaiaCosmosLikeBlockchainExplorer::fuseFilters({
        GaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
            cosmos::constants::kEventTypeTransfer,
            cosmos::constants::kAttributeKeyRecipient,
            DEFAULT_ADDRESS)
    });
    auto bulk = uv::wait(explorer->getTransactions(filter, 1, 10));
    auto transactions = bulk->transactions;
    ASSERT_TRUE(transactions.size() >= 1) << "At least 1 transaction must be fetched looking at the REST response manually.";
    bool foundTx = false;
    for (const auto& tx : transactions) {
        if (tx.hash == "0DBFC4E8E9E5A64C2C9B5EAAAA0422D99A61CFC5354E15002A061E91200DC2D6") {
            foundTx = true;
            EXPECT_EQ(tx.block->height, 237691);
            EXPECT_EQ(tx.logs.size(), 3);
            size_t withdraw_msg_index = 2;
            if (tx.messages[0].type == cosmos::constants::kMsgWithdrawDelegationReward) {
                withdraw_msg_index = 0;
            } else if (tx.messages[1].type == cosmos::constants::kMsgWithdrawDelegationReward) {
                withdraw_msg_index = 1;
            } else {
                FAIL() << cosmos::constants::kMsgWithdrawDelegationReward << " message not found in tx";
            }
            EXPECT_TRUE(tx.logs[withdraw_msg_index].success);
            EXPECT_TRUE(tx.messages[withdraw_msg_index].log.success);
            EXPECT_EQ(tx.messages[withdraw_msg_index].type, cosmos::constants::kMsgWithdrawDelegationReward);
            const cosmos::MsgWithdrawDelegationReward& msg = boost::get<cosmos::MsgWithdrawDelegationReward>(tx.messages[0].content);
            EXPECT_EQ(msg.delegatorAddress, DEFAULT_ADDRESS);
            EXPECT_EQ(msg.validatorAddress, "cosmosvaloper1sd4tl9aljmmezzudugs7zlaya7pg2895ws8tfs");
            EXPECT_EQ(tx.fee.gas.toInt64(), 200000);
            EXPECT_EQ(tx.fee.amount[withdraw_msg_index].denom, "uatom");
            EXPECT_EQ(tx.fee.amount[withdraw_msg_index].amount, "5000");
            EXPECT_EQ(tx.gasUsed, Option<std::string>("104477"));
            break;
        }
    }
    ASSERT_TRUE(foundTx) << "The transaction we need to test has not been found in the REST request.";
}


TEST_F(CosmosLikeWalletSynchronization, GetErrorTransaction) {
    mockHttp("CosmosLikeWalletSynchronization.GetErrorTransaction");
    auto tx_hash = "4A7823F0F2899AA6EC1DCB2E242C541EDAF90419A3DE03ED885E438FEDB779D4";
    auto validator = "cosmosvaloper1clpqr4nrk4khgkxj78fcwwh6dl3uw4epsluffn";
    auto delegator = "cosmos1k3kg9w60dd5x56vve2s28v3xjp7fp2vn2hjjsa";

    auto tx = uv::wait(explorer->getTransactionByHash(tx_hash));
    ASSERT_EQ(tx->hash, tx_hash);
    EXPECT_EQ(tx->block->height, 768780);
    EXPECT_EQ(tx->logs.size(), 2);
    // Logs are stored twice now, but only in serialized types
    // DB will only ever store logs in cosmos_messages table anyway
    EXPECT_FALSE(tx->logs[0].success);
    EXPECT_EQ(tx->logs[0].log, "{\"codespace\":\"sdk\",\"code\":10,\"message\":\"insufficient account funds; 2412592uatom < 2417501uatom\"}");
    EXPECT_FALSE(tx->messages[0].log.success);
    EXPECT_EQ(tx->messages[0].log.log, R"esc({"codespace":"sdk","code":10,"message":"insufficient account funds; 2412592uatom < 2417501uatom"})esc");
    EXPECT_EQ(tx->messages[0].type, cosmos::constants::kMsgDelegate);
    const cosmos::MsgDelegate& msg = boost::get<cosmos::MsgDelegate>(tx->messages[0].content);
    EXPECT_EQ(msg.delegatorAddress, delegator);
    EXPECT_EQ(msg.validatorAddress, validator);
    EXPECT_EQ(msg.amount.amount, "2417501");
    EXPECT_EQ(msg.amount.denom, "uatom");
    EXPECT_EQ(tx->fee.gas.toInt64(), 200000);
    EXPECT_EQ(tx->fee.amount[0].denom, "uatom");
    EXPECT_EQ(tx->fee.amount[0].amount, "5000");
    EXPECT_EQ(tx->gasUsed, Option<std::string>("47235"));
}


TEST_F(CosmosLikeWalletSynchronization, GetSendWithExplorer) {
    mockHttp("CosmosLikeWalletSynchronization.GetSendWithExplorer");
    auto tx_hash = "F4B8CB550B498F744CCC420907B80D0B068250972F975354A873CD1CCF9B000A";
    auto receiver = "cosmos1xxkueklal9vejv9unqu80w9vptyepfa95pd53u";
    // Note : the sender of the message is also the sender of the funds in this transaction.
    auto sender = "cosmos15v50ymp6n5dn73erkqtmq0u8adpl8d3ujv2e74";

    auto tx = uv::wait(explorer->getTransactionByHash(tx_hash));
    ASSERT_EQ(tx->hash, tx_hash);
    EXPECT_EQ(tx->block->height, 453223);
    EXPECT_EQ(tx->logs.size(), 2);
    EXPECT_TRUE(tx->logs[0].success);
    EXPECT_TRUE(tx->messages[0].log.success);
    EXPECT_EQ(tx->messages[0].type, cosmos::constants::kMsgSend);
    const cosmos::MsgSend& msg = boost::get<cosmos::MsgSend>(tx->messages[0].content);
    EXPECT_EQ(msg.fromAddress, sender);
    EXPECT_EQ(msg.toAddress, receiver);
    EXPECT_EQ(msg.amount[0].amount, "270208360");
    EXPECT_EQ(msg.amount[0].denom, "uatom");
    EXPECT_EQ(tx->fee.gas.toInt64(), 200000);
    EXPECT_EQ(tx->fee.amount[0].denom, "uatom");
    EXPECT_EQ(tx->fee.amount[0].amount, "30");
    EXPECT_EQ(tx->gasUsed, Option<std::string>("41014"));
}

TEST_F(CosmosLikeWalletSynchronization, GetDelegateWithExplorer) {
    mockHttp("CosmosLikeWalletSynchronization.GetDelegateWithExplorer");
    auto delegator = "cosmos1ytpz9gt59hssp5m5sknuzrwse88glqhgcrypxj";
    auto validator = "cosmosvaloper1ey69r37gfxvxg62sh4r0ktpuc46pzjrm873ae8";

    auto filter = GaiaCosmosLikeBlockchainExplorer::fuseFilters(
        {GaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
             cosmos::constants::kEventTypeMessage,
             cosmos::constants::kAttributeKeyAction,
             cosmos::constants::kEventTypeDelegate),
         GaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
             cosmos::constants::kEventTypeMessage,
             cosmos::constants::kAttributeKeySender,
             delegator)});
    auto bulk = uv::wait(explorer->getTransactions(filter, 1, 10));
    auto transactions = bulk->transactions;
    ASSERT_TRUE(transactions.size() >= 1);
    bool foundTx = false;
    for (const auto& tx : transactions) {
        if (tx.hash == "BD77DF6A76066AA79DAA7705B9F0DC6B66B7E6FBB3D1FD28A07D6A0EED7AE6B5") {
            foundTx = true;
            EXPECT_EQ(tx.block->height, 660081);
            EXPECT_EQ(tx.logs.size(), 2);
            EXPECT_TRUE(tx.logs[0].success);
            EXPECT_TRUE(tx.messages[0].log.success);
            EXPECT_EQ(tx.messages[0].type, cosmos::constants::kMsgDelegate);
            const cosmos::MsgDelegate& msg = boost::get<cosmos::MsgDelegate>(tx.messages[0].content);
            EXPECT_EQ(msg.delegatorAddress, delegator);
            EXPECT_EQ(msg.validatorAddress, validator);
            EXPECT_EQ(msg.amount.amount, "25257508");
            EXPECT_EQ(msg.amount.denom, "uatom");
            EXPECT_EQ(tx.fee.gas.toInt64(), 300000);
            EXPECT_EQ(tx.fee.amount[0].denom, "uatom");
            EXPECT_EQ(tx.fee.amount[0].amount, "2500");
            EXPECT_EQ(tx.gasUsed, Option<std::string>("104477"));
            break;
        }
    }
    ASSERT_TRUE(foundTx);
}

// FIXME This test fails ; put at the end because it also messes up the other tests
// This test is also highly dependent on external state ( how well the chain is
// doing). Until a better solution is found, this test is deactivated
// TEST_F(CosmosLikeWalletSynchronization, SuccessiveSynchronizations) {
//     std::string hexPubKey(ledger::testing::cosmos::DEFAULT_HEX_PUB_KEY);
//
//     std::shared_ptr<CosmosLikeAccount> account;
//     std::shared_ptr<AbstractWallet> wallet;
//
//     setupTest(account, wallet, hexPubKey);
//
//     // First synchro
//     performSynchro(account);
//     auto blockHeight1 = wait(account->getLastBlock()).height;
//
//     // Wait 30s (new Cosmos block every 7s)
//     fmt::print("Waiting new Cosmos block for 30s...\n");
//     std::flush(std::cerr);
//     std::flush(std::cout);
//     std::this_thread::sleep_for(std::chrono::seconds(30));
//
//     // Second synchro
//     // FIXME Fails due to limitation of test framework??
//     performSynchro(account);
//     auto blockHeight2 = wait(account->getLastBlock()).height;
//
//     EXPECT_NE(blockHeight1, blockHeight2);
// }

