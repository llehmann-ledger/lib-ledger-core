/*
 *
 * ethereum_transaction_tests
 *
 * Created by El Khalil Bellakrid on 14/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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
#include "../../fixtures/eth_xpub_fixtures.h"
#include <api/KeychainEngines.hpp>
#include "transaction_test_helper.h"
#include <utils/hex.h>
#include <utils/DateUtils.hpp>
#include <wallet/ethereum/database/EthereumLikeAccountDatabaseHelper.h>
#include <wallet/ethereum/api_impl/EthereumLikeTransactionApi.h>
#include <wallet/currencies.hpp>
#include <iostream>
#include <Uuid.hpp>
using namespace std;

struct EthereumMakeTransaction2 : public EthereumMakeBaseTransaction {
    void SetUpConfig() override {
        auto configuration = DynamicObject::newInstance();
        testData.configuration = configuration;
        testData.walletName = uuid::generate_uuid_v4();
        testData.currencyName = "ethereum";
        testData.inflate_eth = ledger::testing::eth_xpub::inflate;
    }
};


TEST_F(EthereumMakeTransaction2, ParseSignedRawERC20TransactionAndBuildETHBlockchainExplorerTx) {
    //Tx hash 96c71eac2350d498ba89863465c1c362fc3ec323f33b5eb65ee60da5cb75420a (Ropsten)
    auto txHash = "96c71eac2350d498ba89863465c1c362fc3ec323f33b5eb65ee60da5cb75420a";
    auto rawTx = "f8aa8180843b9aca0083030d4094dfb287530fd4c1e59456de82a84e4aae7c250ec180b844a9059cbb000000000000000000000000456b8e57f5e096b9fff45bdbd58b8ce90d830ff900000000000000000000000000000000000000000000001b1ae4d6e2ef50000029a0fde02005f6bb452269dabb128ad2c9a44090f6f160fc7be8ef511224524fc5b9a06e07d22f5e92435dfcfc633f85825291c118d0f05ef46ea71774f306325d7ea2";
    auto ethLikeBCTx = EthereumLikeAccount::getETHLikeBlockchainExplorerTxFromRawTx(account, txHash, hex::toByteArray(rawTx));
    EXPECT_EQ(ethLikeBCTx.status, 1);
    // For test purposes sender is account's address (real sender is 0xd0ec064cff693453ef4595aa555ce65244b212a5)
    auto sender = account->getKeychain()->getAddress()->toEIP55();
    auto receiver = "0x456b8e57F5e096B9Fff45BDbD58B8CE90d830Ff9";
    auto contractAddress = "0xDFb287530FD4c1e59456DE82a84e4aae7C250Ec1";
    EXPECT_EQ(ethLikeBCTx.sender, sender);
    EXPECT_EQ(ethLikeBCTx.receiver, contractAddress);
    EXPECT_EQ(ethLikeBCTx.value, BigInt::ZERO);
    EXPECT_EQ(ethLikeBCTx.gasPrice.toString(), "1000000000");
    EXPECT_EQ(ethLikeBCTx.gasLimit.toString(), "200000");
    EXPECT_EQ(hex::toString(ethLikeBCTx.inputData), "a9059cbb000000000000000000000000456b8e57f5e096b9fff45bdbd58b8ce90d830ff900000000000000000000000000000000000000000000001b1ae4d6e2ef500000");
    EXPECT_EQ(ethLikeBCTx.erc20Transactions.size(), 1);
    EXPECT_EQ(ethLikeBCTx.erc20Transactions[0].value.toString(), "500000000000000000000");
    EXPECT_EQ(ethLikeBCTx.erc20Transactions[0].from, sender);
    EXPECT_EQ(ethLikeBCTx.erc20Transactions[0].to, receiver);
    EXPECT_EQ(ethLikeBCTx.erc20Transactions[0].contractAddress, contractAddress);
    EXPECT_EQ(ethLikeBCTx.erc20Transactions[0].type, api::OperationType::SEND);
}