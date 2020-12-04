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

class StellarLikeWalletSynchronization2 : public BaseFixture {

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

api::AccountCreationInfo accountInfo(const std::string &pubKey);
api::AccountCreationInfo accountInfoFromAddress(const std::string& address);

TEST_F(StellarLikeWalletSynchronization2, SynchronizeStellarAccountWithManageBuyOffer) {
    mockHttp("StellarLikeWalletSynchronization2.SynchronizeStellarAccountWithManageBuyOffer");

    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "stellar", api::DynamicObject::newInstance()));
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    auto account = std::dynamic_pointer_cast<StellarLikeAccount>(uv::wait(wallet->newAccountWithInfo(accountInfoFromAddress("GDDU4HHNCSZ2BI6ELSSFKPSOBL2TEB4A3ZJWOCT2DILQKVJTZBNSOZA2"))));

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
}

TEST_F(StellarLikeWalletSynchronization2, SynchronizeStellarAccountWithMultisig) {
    mockHttp("StellarLikeWalletSynchronization2.SynchronizeStellarAccountWithMultisig");

    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "stellar", api::DynamicObject::newInstance()));
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    auto account = std::dynamic_pointer_cast<StellarLikeAccount>(uv::wait(wallet->newAccountWithInfo(accountInfoFromAddress("GAJTWW4OGH5BWFTH24C7SGIDALKI2HUVC2LXHFD533A5FIMSXE5AB3TJ"))));
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
    auto signers = uv::wait(account->getSigners());
    EXPECT_EQ(signers.size(), 2);
    const auto& signer_1 = std::find_if(signers.begin(), signers.end(), [] (const stellar::AccountSigner& s) {
        return s.key == "GAJTWW4OGH5BWFTH24C7SGIDALKI2HUVC2LXHFD533A5FIMSXE5AB3TJ";
    });
    const auto& signer_2 = std::find_if(signers.begin(), signers.end(), [] (const stellar::AccountSigner& s) {
        return s.key == "GDDU4HHNCSZ2BI6ELSSFKPSOBL2TEB4A3ZJWOCT2DILQKVJTZBNSOZA2";
    });
    EXPECT_NE(signer_1, signers.end());
    EXPECT_NE(signer_2, signers.end());

    EXPECT_EQ(signer_1->type, "ed25519_public_key");
    EXPECT_EQ(signer_1->weight, 10);

    EXPECT_EQ(signer_2->type, "ed25519_public_key");
    EXPECT_EQ(signer_2->weight, 10);
}

// Synchronize an account with protocol 13 upgrade object
TEST_F(StellarLikeWalletSynchronization2, SynchronizeProtocol13) {
    mockHttp("StellarLikeWalletSynchronization2.SynchronizeProtocol13");

    // GBV4NH4G5SWYM6OQJKZKG2PA2O2VQ2W6K5S43WLMLJRWU4XTG5EST5QP
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "stellar", api::DynamicObject::newInstance()));
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    auto account = std::dynamic_pointer_cast<StellarLikeAccount>(uv::wait(wallet->newAccountWithInfo(accountInfoFromAddress("GBSEXVEU2WBBLIUCWIWFDPV5I4HLBSOWRNJVKLNXZFVNITFPKQIVO3YI"))));
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
    auto address = uv::wait(account->getFreshPublicAddresses()).front();
    auto operations = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
    EXPECT_GT(balance->toLong(), 0);
    EXPECT_TRUE(operations.size() >= 11);

    // Fetch the first send operation
    for (const auto& op: operations) {
        fmt::print("{} {} {}\n", api::to_string(op->getOperationType()), op->getAmount()->toString(), op->asStellarLikeOperation()->getRecord().transactionHash);
        if (op->getOperationType() == api::OperationType::SEND) {
            const auto& sop = op->asStellarLikeOperation();
            ASSERT_EQ(sop->getTransaction()->getSourceAccount()->toString(), address->toString());
            ASSERT_TRUE(sop->getTransaction()->getFee()->toLong() > 0);
            auto sequence = std::dynamic_pointer_cast<api::BigIntImpl>(sop->getTransaction()->getSourceAccountSequence())->backend();
            ASSERT_TRUE(sequence > BigInt::ZERO);
        }
    }
}