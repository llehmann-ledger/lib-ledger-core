/*
 *
 * transaction_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/08/2019.
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

#include "StellarFixture.hpp"
#include <wallet/stellar/transaction_builders/StellarLikeTransactionBuilder.hpp>
#include <wallet/currencies.hpp>
#include <api_impl/BigIntImpl.hpp>

TEST_F(StellarFixture, ParseRawTransaction) {
    auto strTx = "00000000a1083d11720853a2c476a07e29b64e0f9eb2ff894f1e485628faa7b63de77a4f0"
                 "0000064015dc2cc000000030000000000000000000000010000000000000001000000003a"
                 "83935fabfdc44749ad4d042dbc4df9b59442f325a27960519fba516adb8a5000000000000"
                 "00000000000000000000000000000";

    auto tx = api::StellarLikeTransactionBuilder::parseRawTransaction(ledger::core::currencies::STELLAR, hex::toByteArray(strTx));

    EXPECT_EQ(tx->getSourceAccount()->toString(), "GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3");
    EXPECT_EQ(tx->getSourceAccountSequence()->compare(api::BigInt::fromLong(98448948301135875L)), 0);
    EXPECT_EQ(tx->getFee()->toLong(), 100L);

    EXPECT_EQ(hex::toString(tx->toRawTransaction()), strTx);
}

TEST_F(StellarFixture, ParseSignatureBase) {
    auto strTx = "7ac33997544e3175d266bd022439b22cdb16508c01163f26e5cb2a3e1045a9790000000200"
                 "000000a1083d11720853a2c476a07e29b64e0f9eb2ff894f1e485628faa7b63de77a4f0000"
                 "0064015dc2cc000000030000000000000000000000010000000000000001000000003a8393"
                 "5fabfdc44749ad4d042dbc4df9b59442f325a27960519fba516adb8a500000000000000000"
                 "0000000000000000";

    auto tx = api::StellarLikeTransactionBuilder::parseSignatureBase(ledger::core::currencies::STELLAR, hex::toByteArray(strTx));

    EXPECT_EQ(tx->getSourceAccount()->toString(), "GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3");
    EXPECT_EQ(tx->getSourceAccountSequence()->compare(api::BigInt::fromLong(98448948301135875L)), 0);
    EXPECT_EQ(tx->getFee()->toLong(), 100L);

    EXPECT_EQ(hex::toString(tx->toSignatureBase()), strTx);
}
