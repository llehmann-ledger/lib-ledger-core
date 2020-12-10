/*
 *
 * bitcoin_transaction_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/03/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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
#include "../../fixtures/medium_xpub_fixtures.h"
#include "../../fixtures/bch_xpub_fixtures.h"
#include "../../fixtures/zec_xpub_fixtures.h"
#include <wallet/bitcoin/transaction_builders/BitcoinLikeTransactionBuilder.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeWritableInputApi.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>
#include "transaction_test_helper.h"
#include <crypto/HASH160.hpp>
#include <utils/hex.h>
#include <utils/DateUtils.hpp>
#include <wallet/bitcoin/networks.hpp>
#include <iostream>
#include <Uuid.hpp>
using namespace std;

struct BitcoinMakeP2PKHTransaction2 : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.walletName = uuid::generate_uuid_v4();
        testData.currencyName = "bitcoin";
        testData.inflate_btc = ledger::testing::medium_xpub::inflate;
    }
};

TEST_F(BitcoinMakeP2PKHTransaction2, OptimizeSize) {
    mockHttp("BitcoinMakeP2PKHTransaction2.OptimizeSize");
    auto builder = tx_builder();
    const int64_t feesPerByte = 20;
    builder->sendToAddress(api::Amount::fromLong(currency, 10000), "14GH47aGFWSjvdrEiYTEfwjgsphNtbkWzP");
    builder->pickInputs(api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, feesPerByte));
    auto f = builder->build();
    auto tx = uv::wait(f);
    tx->getInputs()[0]->pushToScriptSig({ 5, 'h', 'e', 'l', 'l', 'o' });
    auto transactionSize = tx->serialize().size();
    auto fees = tx->getFees();
    EXPECT_TRUE(fees->toLong() >= transactionSize * feesPerByte);
}

TEST_F(BitcoinMakeP2PKHTransaction2, Toto) {
    mockHttp("BitcoinMakeP2PKHTransaction2.Toto");
    std::shared_ptr<AbstractWallet> w = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "bitcoin_testnet", DynamicObject::newInstance()));
    api::ExtendedKeyAccountCreationInfo info = uv::wait(w->getNextExtendedKeyAccountCreationInfo());
    info.extendedKeys.push_back("tpubDCJarhe7f951cUufTWeGKh1w6hDgdBcJfvQgyMczbxWvwvLdryxZuchuNK3KmTKXwBNH6Ze6tHGrUqvKGJd1VvSZUhTVx58DrLn9hR16DVr");
    std::shared_ptr<AbstractAccount> account = std::dynamic_pointer_cast<AbstractAccount>(uv::wait(w->newAccountWithExtendedKeyInfo(info)));
    std::shared_ptr<BitcoinLikeAccount> bla = std::dynamic_pointer_cast<BitcoinLikeAccount>(account);
    Promise<Unit> p;
    auto s = bla->synchronize();
    s->subscribe(bla->getContext(), make_receiver([=](const std::shared_ptr<api::Event> &event) mutable {
        fmt::print("Received event {}\n", api::to_string(event->getCode()));
        if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
            return;
        EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
        EXPECT_EQ(event->getCode(),
                  api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT);
        p.success(unit);
    }));
    Unit u = uv::wait(p.getFuture());

    auto builder = std::dynamic_pointer_cast<BitcoinLikeTransactionBuilder>(bla->buildTransaction(false));
    builder->sendToAddress(api::Amount::fromLong(currency, 1000), "ms8C1x7qHa3WJM986NKyx267i2LFGaHRZn");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
    auto f = builder->build();
    auto tx = uv::wait(f);
    std::cout << hex::toString(tx->serialize()) << std::endl;
    std::cout << tx->getOutputs()[0]->getAddress().value_or("NOP") << std::endl;
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize(), 0);
    auto rawPrevious = uv::wait(std::dynamic_pointer_cast<BitcoinLikeWritableInputApi>(tx->getInputs()[0])->getPreviousTransaction());
    std::cout << hex::toString(parsedTx->serialize()) << std::endl;
    std::cout << parsedTx->getInputs().size() << std::endl;
    std::cout << hex::toString(rawPrevious) << std::endl;
    std::cout << tx->getFees()->toLong() << std::endl;
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
}