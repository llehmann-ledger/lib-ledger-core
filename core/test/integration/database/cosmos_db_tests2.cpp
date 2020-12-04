#include "../integration/BaseFixture.h"
#include "../../cosmos/Fixtures.hpp"
#include <wallet/cosmos/api_impl/CosmosLikeTransactionApi.hpp>
#include <wallet/cosmos/database/CosmosLikeTransactionDatabaseHelper.hpp>
#include <wallet/cosmos/CosmosLikeOperationQuery.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <wallet/cosmos/CosmosLikeMessage.hpp>
#include <wallet/cosmos/CosmosLikeWallet.hpp>

#include <wallet/pool/WalletPool.hpp>
#include <api/PoolConfiguration.hpp>
#include <utils/DateUtils.hpp>
#include <Uuid.hpp>
#include <chrono>

#include <gtest/gtest.h>

using namespace ledger::testing::cosmos;

class CosmosDBTest2 : public BaseFixture {
public:
 void SetUp() override
 {
     BaseFixture::SetUp();
#ifdef PG_SUPPORT
     const bool usePostgreSQL = true;
     auto poolConfig = DynamicObject::newInstance();
     poolConfig->putString(api::PoolConfiguration::DATABASE_NAME, "postgres://localhost:5432/test_db");
     pool = newDefaultPool(uuid::generate_uuid_v4(), "", poolConfig, usePostgreSQL);
#else
     pool = newDefaultPool(uuid::generate_uuid_v4());
#endif
 }

 void setupTest(
     std::shared_ptr<WalletPool> &pool,
     std::shared_ptr<CosmosLikeAccount> &account,
     std::shared_ptr<CosmosLikeWallet> &wallet,
     const std::string& walletName)
 {
     auto configuration = DynamicObject::newInstance();
     configuration->putString(
         api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
         "44'/<coin_type>'/<account>'/<node>/<address>");
     wallet = std::dynamic_pointer_cast<CosmosLikeWallet>(
         uv::wait(pool->createWallet(walletName, "cosmos", configuration)));

     auto accountInfo = uv::wait(wallet->getNextAccountCreationInfo());
     EXPECT_EQ(accountInfo.index, 0);
     accountInfo.publicKeys.push_back(hex::toByteArray(DEFAULT_HEX_PUB_KEY));

     account = createCosmosLikeAccount(wallet, accountInfo.index, accountInfo);
 }

 void TearDown() override {
     uv::wait(pool->eraseDataSince(std::chrono::time_point<std::chrono::system_clock>{}));
     BaseFixture::TearDown();
 }

 std::shared_ptr<WalletPool> pool;
};


TEST_F(CosmosDBTest2, UnsuportedMsgTypeTest) {
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<CosmosLikeWallet> wallet;
    setupTest(pool, account, wallet, uuid::generate_uuid_v4());

    std::chrono::system_clock::time_point timeRef = DateUtils::now();

    const auto msg = setupSendMessage();
    auto tx = setupTransactionResponse(std::vector<Message>{ msg }, timeRef);

    // Change message type
    tx.messages[0].type = "unknown-message-type";

    {
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        sql.set_log_stream(&std::cerr);
        account->putTransaction(sql, tx);
    }

    {
        auto ops = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
        ASSERT_EQ(ops.size(), 1);

        auto op = ops[0];
        // auto cosmosOp = std::dynamic_pointer_cast<CosmosLikeOperation>(op);
        auto cosmosOp = op->asCosmosLikeOperation();
        const auto txRetrieved = std::dynamic_pointer_cast<CosmosLikeTransactionApi>(cosmosOp->getTransaction())->getRawData();

        assertSameTransaction(tx, txRetrieved);
    }
}

TEST_F(CosmosDBTest2, MultipleMsgTest) {
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<CosmosLikeWallet> wallet;
    setupTest(pool, account, wallet, uuid::generate_uuid_v4());

    std::chrono::system_clock::time_point timeRef = DateUtils::now();

    const auto msgSend = setupSendMessage();
    const auto msgVote = setupVoteMessage();
    const auto tx = setupTransactionResponse(std::vector<Message>{ msgSend, msgVote }, timeRef);

    {
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        sql.set_log_stream(&std::cerr);
        account->putTransaction(sql, tx);
    }

    {
        auto ops = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
        ASSERT_EQ(ops.size(), 2);

        for (auto op : ops)
        {
            // auto op = ops[0];
            // auto cosmosOp = std::dynamic_pointer_cast<CosmosLikeOperation>(op);
            auto cosmosOp = op->asCosmosLikeOperation();
            ASSERT_NE(cosmosOp, nullptr);
            ASSERT_NE(cosmosOp->getMessage(), nullptr);
            ASSERT_NE(cosmosOp->getTransaction(), nullptr);
            auto txPtr = std::dynamic_pointer_cast<CosmosLikeTransactionApi>(cosmosOp->getTransaction());
            ASSERT_NE(txPtr, nullptr);
            auto txRetrieved = txPtr->getRawData();
            assertSameTransaction(tx, txRetrieved);
            auto msgPtr = std::dynamic_pointer_cast<CosmosLikeMessage>(cosmosOp->getMessage());
            ASSERT_NE(msgPtr, nullptr);
            auto msgRetrieved = msgPtr->getRawData();
            if (msgRetrieved.type == "cosmos-sdk/MsgSend") {
                assertSameSendMessage(msgSend, msgRetrieved);
            }
            else if (msgRetrieved.type == "cosmos-sdk/MsgVote") {
                assertSameVoteMessage(msgVote, msgRetrieved);
            }
            else {
                FAIL() << fmt::format(
                    "{} is not an expected message type in the ops", msgRetrieved.type);
            }
        }
    }
}
