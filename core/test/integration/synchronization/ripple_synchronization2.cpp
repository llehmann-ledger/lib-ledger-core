/*
 *
 * ripple_synchronization
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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

#include <gtest/gtest.h>
#include "../BaseFixture.h"
#include <set>
#include <api/KeychainEngines.hpp>
#include <utils/DateUtils.hpp>
#include <wallet/ripple/database/RippleLikeAccountDatabaseHelper.h>
#include <wallet/ripple/transaction_builders/RippleLikeTransactionBuilder.h>
#include <iostream>
#include <api/BlockchainExplorerEngines.hpp>
#include <api/RippleLikeOperation.hpp>
#include <api/RippleLikeTransaction.hpp>
#include <Uuid.hpp>

using namespace std;

class RippleLikeWalletSynchronization2 : public BaseFixture {

};

const std::string NOTIF_WITH_BLOCK = "{\"fee_base\":10,\"fee_ref\":10,\"ledger_hash\":\"43BF0F7D1131B5926153E8847CC42B8652B451DF09F94558BE8FF9FF9F846428\",\"ledger_index\":44351888,\"ledger_time\":600609550,\"reserve_base\":20000000,\"reserve_inc\":5000000,\"txn_count\":26,\"type\":\"ledgerClosed\",\"validated_ledgers\":\"32570-44351888\"}";
TEST_F(RippleLikeWalletSynchronization2, EmitNewBlock) {
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    {
        auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "ripple", api::DynamicObject::newInstance()));
        auto account = createRippleLikeAccount(wallet, 0, XRP_KEYS_INFO);
        auto receiver = make_receiver([&] (const std::shared_ptr<api::Event>& event) {
            if (event->getCode() == api::EventCode::NEW_BLOCK) {
                try {
                    auto height = event->getPayload()->getLong(api::Account::EV_NEW_BLOCK_HEIGHT).value_or(0);
                    auto hash = event->getPayload()->getString(api::Account::EV_NEW_BLOCK_HASH).value_or("");
                    auto block = uv::wait(pool->getLastBlock("ripple"));
                    EXPECT_EQ(height, block.height);
                    EXPECT_EQ(hash, block.blockHash);
                } catch (const std::exception& ex) {
                    fmt::print("{}", ex.what());
                    FAIL();
                }
                dispatcher->stop();
            }
        });
        ws->setOnConnectCallback([&] () {
            ws->push(NOTIF_WITH_BLOCK);
        });
        EXPECT_EQ(uv::wait(account->getFreshPublicAddresses())[0]->toString(), "rageXHB6Q4VbvvWdTzKANwjeCT4HXFCKX7");
        auto eventBus = account->getEventBus();
        eventBus->subscribe(getTestExecutionContext(), receiver);
        account->startBlockchainObservation();
        dispatcher->waitUntilStopped();
    }
}

TEST_F(RippleLikeWalletSynchronization2, VaultAccountSynchronization) {
    mockHttp("RippleLikeWalletSynchronization2.VaultAccountSynchronization");
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    auto configuration = DynamicObject::newInstance();
    configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                             "44'/<coin_type>'/<account>'/<node>/<address>");
    auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "ripple", configuration));
    auto nextIndex = uv::wait(wallet->getNextAccountIndex());
    auto account = createRippleLikeAccount(wallet, nextIndex, VAULT_XRP_KEYS_INFO);
    auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
        fmt::print("Received event {}\n", api::to_string(event->getCode()));
        if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
            return;
        EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
        EXPECT_EQ(event->getCode(),
                  api::EventCode::SYNCHRONIZATION_SUCCEED);

        dispatcher->stop();
    });

    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(), receiver);
    dispatcher->waitUntilStopped();

    auto ops = uv::wait(
            std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
    std::cout << "Ops: " << ops.size() << std::endl;

    uint32_t destinationTag = 0;
    for (auto const& op : ops) {
        auto xrpOp = op->asRippleLikeOperation();

        if (xrpOp->getTransaction()->getHash() == "EE38840B83CAB39216611D2F6E4F9828818514C3EA47504AE2521D8957331D3C" ) {
          destinationTag = xrpOp->getTransaction()->getDestinationTag().value_or(0);
          break;
        }
    }

    EXPECT_TRUE(destinationTag == 123456789);
}
