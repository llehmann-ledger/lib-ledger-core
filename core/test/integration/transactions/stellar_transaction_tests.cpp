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
#include <wallet/stellar/transaction_builders/StellarLikeTransactionBuilder.hpp>
#include <api/Account.hpp>
#include <api/AccountCreationInfo.hpp>

#include <Uuid.hpp>

class StellarLikeWalletTransaction : public BaseFixture {

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

TEST_F(StellarLikeWalletTransaction, PaymentTransaction) {
    mockHttp("StellarLikeWalletTransaction.PaymentTransaction");
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "stellar", api::DynamicObject::newInstance()));
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    auto account = std::dynamic_pointer_cast<StellarLikeAccount>(uv::wait(wallet->newAccountWithInfo(accountInfo("a1083d11720853a2c476a07e29b64e0f9eb2ff894f1e485628faa7b63de77a4f"))));
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
    auto address = *uv::wait(account->getFreshPublicAddresses()).begin();
    auto signature = hex::toByteArray("3045022100B2B31575F8536B284410D01217F688BE3A9FAF4BA0BA3A9093F983E40D630EC7022022A7A25B01403CFF0D00B3B853D230F8E96FF832B15D4CCC75203CB65896A2D5");
    auto builder = std::dynamic_pointer_cast<StellarLikeTransactionBuilder>(account->buildTransaction());
    auto sequence = uv::wait(account->getSequence());
    auto fees =  100;//Disabled until nodes can handle this request ==> uv::wait(account->getFeeStats()).modeAcceptedFee;
    builder->setSequence(api::BigInt::fromLong(sequence.toInt64()));
    builder->addNativePayment("GA5IHE27VP64IR2JVVGQILN4JX43LFCC6MS2E6LAKGP3UULK3OFFBJXR", api::Amount::fromLong(wallet->getCurrency(), 20000000));
    builder->setBaseFee( api::Amount::fromLong(wallet->getCurrency(), fees));
    auto tx = uv::wait(builder->build());
    tx->putSignature(signature, address);
    auto wrappedEnvelope = std::dynamic_pointer_cast<StellarLikeTransaction>(tx)->envelope();
    const auto& envelope = boost::get<stellar::xdr::TransactionV1Envelope>(wrappedEnvelope.content);
    EXPECT_EQ(envelope.signatures.size() , 1);
    EXPECT_EQ(envelope.tx.sourceAccount.type, stellar::xdr::CryptoKeyType::KEY_TYPE_ED25519);
    auto accountPubKey = account->getKeychain()->getAddress()->toPublicKey();
    auto sourceAccount = boost::get<stellar::xdr::uint256>(envelope.tx.sourceAccount.content);
    std::vector<uint8_t> envelopeSourcePubKey(sourceAccount.begin(), sourceAccount.end());
    EXPECT_EQ(accountPubKey, envelopeSourcePubKey);
    EXPECT_TRUE(envelope.tx.seqNum >= 98448948301135874L);
    EXPECT_TRUE(envelope.tx.fee == fees);
    EXPECT_TRUE(envelope.tx.operations.size() == 1);
    EXPECT_EQ(envelope.tx.memo.type, stellar::xdr::MemoType::MEMO_NONE);
    EXPECT_EQ(envelope.signatures.front().signature, signature);
    fmt::print("{}\n", hex::toString(tx->toRawTransaction()));
}