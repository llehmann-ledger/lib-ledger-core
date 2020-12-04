/*
 *
 * synchronization_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 01/04/2019.
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

#include "../BaseFixture.h"
#include <wallet/common/OperationQuery.h>
#include <math/BigInt.h>
#include <api/StellarLikeMemo.hpp>
#include <api/StellarLikeMemoType.hpp>
#include <wallet/stellar/StellarLikeWallet.hpp>
#include <wallet/stellar/StellarLikeAccount.hpp>
#include <wallet/stellar/StellarLikeOperation.hpp>
#include <api/Account.hpp>
#include <api/AccountCreationInfo.hpp>

#include <Uuid.hpp>

class StellarLikeWalletSynchronization : public BaseFixture {

};

static api::StellarLikeNetworkParameters STELLAR_PARAMS {
    "xlm", {6 << 3}, {12 << 3}, 5000000, 100, {}, "Public Global Stellar Network ; September 2015"
};

static api::Currency STELLAR =
        Currency("stellar")
        .forkOfStellar(STELLAR_PARAMS)
        .bip44(148)
        .paymentUri("web+stellar")
        .unit("stroops", 0, "stroops")
        .unit("lumen", 7, "XLM");

api::AccountCreationInfo accountInfo(const std::string &pubKey)  {
    return api::AccountCreationInfo(0, {"main"}, {"44'/148'/0'"}, { ledger::core::hex::toByteArray(pubKey)}, {});
}

api::AccountCreationInfo accountInfoFromAddress(const std::string& address)  {
    StellarLikeAddress addr(
            address,
            STELLAR,
            Option<std::string>::NONE
    );
    return accountInfo(ledger::core::hex::toString(addr.toPublicKey()));
}

TEST_F(StellarLikeWalletSynchronization, SynchronizeStellarAccount) {
    mockHttp("StellarLikeWalletSynchronization.SynchronizeStellarAccount");

    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "stellar", api::DynamicObject::newInstance()));
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    auto account = std::dynamic_pointer_cast<StellarLikeAccount>(uv::wait(wallet->newAccountWithInfo(accountInfo("a1083d11720853a2c476a07e29b64e0f9eb2ff894f1e485628faa7b63de77a4f"))));
    auto exists = uv::wait(account->exists());
    EXPECT_TRUE(exists);
    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(),
                   make_receiver([=](const std::shared_ptr<api::Event> &event) {
                       fmt::print("Received event {}\n", api::to_string(event->getCode()));
                       if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                           return;
                       EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                       EXPECT_EQ(event->getCode(),
                                 api::EventCode::SYNCHRONIZATION_SUCCEED);
                       getTestExecutionContext()->stop();
                   }));
    EXPECT_EQ(bus, account->synchronize());
    getTestExecutionContext()->waitUntilStopped();
    auto balance = uv::wait(account->getBalance());
    auto operations = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->addOrder(api::OperationOrderKey::DATE, false)->complete())->execute());
    EXPECT_TRUE(balance->toBigInt()->compare(api::BigInt::fromLong(0)) > 0);
    EXPECT_TRUE(operations.size() >= 5);

    for (const auto& op : operations) {
        auto record = op->asStellarLikeOperation()->getRecord();
        fmt::print("{} {} {} {}\n",   api::to_string(op->getOperationType()), op->getAmount()->toString(), op->getFees()->toString(), api::to_string(record.operationType));
        if (op->getOperationType() == api::OperationType::SEND) {
            EXPECT_TRUE(op->getFees()->toLong() >= 100);
        }
    }

    const auto& first = operations.front();

    EXPECT_EQ(first->getAmount()->toString(), "1800038671");
    EXPECT_EQ(first->getFees()->toString(), "100");
    EXPECT_EQ(first->getDate(), DateUtils::fromJSON("2019-03-14T10:08:27Z"));
    EXPECT_EQ(first->getSenders().front(), "GBV4ZDEPNQ2FKSPKGJP2YKDAIZWQ2XKRQD4V4ACH3TCTFY6KPY3OAVS7");
    EXPECT_EQ(first->getRecipients().front(), "GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3");
    EXPECT_EQ(first->isComplete(), true);
    EXPECT_EQ(first->getOperationType(), api::OperationType::RECEIVE);


    const auto& second = operations[1];

    EXPECT_EQ(second->getAmount()->toString(), "50000000");
    EXPECT_EQ(second->getFees()->toString(), "100");
    EXPECT_EQ(second->getDate(), DateUtils::fromJSON("2019-03-14T10:24:15Z"));
    EXPECT_EQ(second->getSenders().front(), "GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3");
    EXPECT_EQ(second->getRecipients().front(), "GB6TMMOCZSFFVXUXPV6FATTGQN6NKV74I2LTBB6LR7GEWLTN2IGZ6L6X");
    EXPECT_EQ(second->isComplete(), true);
    EXPECT_EQ(second->getOperationType(), api::OperationType::SEND);

    auto reserve = uv::wait(account->getBaseReserve());
    EXPECT_TRUE(reserve->toLong() >= (2 * 5000000));

    auto txFound = false;
    for (auto& op : operations) {
        if (op->asStellarLikeOperation()->getRecord().transactionHash == "6c084cdf56ff11b47baeab9a17fe8dc66d8009b7f4f86101758c9e99348af9a3") {
            auto memo = op->asStellarLikeOperation()->getTransaction()->getMemo();
            EXPECT_EQ(memo->getMemoType(), api::StellarLikeMemoType::MEMO_TEXT);
            EXPECT_EQ(memo->getMemoText(), "Salut charlotte");
            txFound = true;
        }
    }
    EXPECT_TRUE(txFound);
}

TEST_F(StellarLikeWalletSynchronization, SynchronizeEmptyStellarAccount) {
    mockHttp("StellarLikeWalletSynchronization.SynchronizeEmptyStellarAccount");

    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "stellar", api::DynamicObject::newInstance()));
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
StellarLikeAddress addr(
            "GCDCR6S7JAYWA3DCD2QOQX7MSHX5BZT2HUFYEMK4R76NXFQ7QQA4TF7W",
            STELLAR,
            Option<std::string>::NONE
            );
     accountInfo(ledger::core::hex::toString(addr.toPublicKey()));
     auto account = std::dynamic_pointer_cast<StellarLikeAccount>(uv::wait(wallet->newAccountWithInfo(accountInfo(ledger::core::hex::toString(addr.toPublicKey())))));

    auto exists = uv::wait(account->exists());
    EXPECT_FALSE(exists);
    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(),
                   make_receiver([=](const std::shared_ptr<api::Event> &event) {
                       fmt::print("Received event {}\n", api::to_string(event->getCode()));
                       if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                           return;
                       EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                       EXPECT_EQ(event->getCode(),
                                 api::EventCode::SYNCHRONIZATION_SUCCEED);
                       getTestExecutionContext()->stop();
                   }));
    getTestExecutionContext()->waitUntilStopped();
    auto address = uv::wait(account->getFreshPublicAddresses())[0];
    auto balance = uv::wait(account->getBalance());
    auto operations = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
    EXPECT_TRUE(balance->toBigInt()->compare(api::BigInt::fromLong(0)) == 0);
    EXPECT_TRUE(operations.size() == 0);

    // Fetch the first send operation
    for (const auto& op: operations) {
        if (op->getOperationType() == api::OperationType::SEND) {
            const auto& sop = op->asStellarLikeOperation();
            ASSERT_EQ(sop->getTransaction()->getSourceAccount()->toString(), address->toString());
            ASSERT_TRUE(sop->getTransaction()->getFee()->toLong() > 0);
            auto sequence = std::dynamic_pointer_cast<api::BigIntImpl>(sop->getTransaction()->getSourceAccountSequence())->backend();
            ASSERT_TRUE(sequence > BigInt::ZERO);
        }
    }
}

TEST_F(StellarLikeWalletSynchronization, SynchronizeStellarAccountWithSubEntry) {
    mockHttp("StellarLikeWalletSynchronization.SynchronizeStellarAccountWithSubEntry");

    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "stellar", api::DynamicObject::newInstance()));
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    StellarLikeAddress addr("GAT4LBXYJGJJJRSNK74NPFLO55CDDXSYVMQODSEAAH3M6EY4S7LPH5GV", STELLAR, Option<std::string>::NONE);
    auto account = std::dynamic_pointer_cast<StellarLikeAccount>(uv::wait(wallet->newAccountWithInfo(accountInfo(hex::toString(addr.toPublicKey())))));

    auto exists = uv::wait(account->exists());
    EXPECT_TRUE(exists);
    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(),
                   make_receiver([=](const std::shared_ptr<api::Event> &event) {
                       fmt::print("Received event {}\n", api::to_string(event->getCode()));
                       if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                           return;
                       EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                       EXPECT_EQ(event->getCode(),
                                 api::EventCode::SYNCHRONIZATION_SUCCEED);
                       getTestExecutionContext()->stop();
                   }));
    EXPECT_EQ(bus, account->synchronize());
    getTestExecutionContext()->waitUntilStopped();
    auto reserve = uv::wait(account->getBaseReserve());
    EXPECT_TRUE(reserve->toLong() > 2 * 5000000);
}