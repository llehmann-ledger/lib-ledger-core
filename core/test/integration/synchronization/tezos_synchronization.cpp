/*
 *
 * tezos_synchronization
 *
 * Created by El Khalil Bellakrid on 11/05/2019.
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
#include <functional>
#include <api/KeychainEngines.hpp>
#include <utils/DateUtils.hpp>
#include <wallet/tezos/database/TezosLikeAccountDatabaseHelper.h>
#include <wallet/tezos/transaction_builders/TezosLikeTransactionBuilder.h>
#include <iostream>
#include <api/BlockchainExplorerEngines.hpp>
#include <wallet/tezos/api_impl/TezosLikeOperation.h>
#include <wallet/tezos/delegation/TezosLikeOriginatedAccount.h>
#include <api/TezosConfiguration.hpp>
#include <api/TezosConfigurationDefaults.hpp>
#include <Uuid.hpp>

using namespace std;

namespace {
    const std::string kExplorerUrl = "https://xtz-explorer.api.live.ledger.com/explorer";
}

class TezosLikeWalletSynchronization : public BaseFixture {

};

TEST_F(TezosLikeWalletSynchronization, SynchronizeAccountWithMoreThan100OpsAndDeactivateSyncToken) {
    mockHttp("TezosLikeWalletSynchronization.SynchronizeAccountWithMoreThan100OpsAndDeactivateSyncToken");
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    auto configuration = DynamicObject::newInstance();
    configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"44'/<coin_type>'/<account>'/<node>'/<address>");
    configuration->putString(api::TezosConfiguration::TEZOS_XPUB_CURVE, api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519);
    configuration->putBoolean(api::Configuration::DEACTIVATE_SYNC_TOKEN, true);
    configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::BlockchainExplorerEngines::TZSTATS_API);
    configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, kExplorerUrl);
    auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "tezos", configuration));
    auto account = createTezosLikeAccount(wallet, 0, XTZ_WITH_100_OPS_KEYS_INFO);
    auto bus = account->synchronize();
    bus->subscribe(account->getContext(), make_receiver([=](const std::shared_ptr<api::Event> &event) {
        if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
            return;
        EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
        EXPECT_EQ(event->getCode(),
                  api::EventCode::SYNCHRONIZATION_SUCCEED);
        dispatcher->stop();
    }));
    dispatcher->waitUntilStopped();
    auto ops = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations())->execute());
    EXPECT_GT(ops.size(), 100);
}

TEST_F(TezosLikeWalletSynchronization, NonActivated) {
    mockHttp("TezosLikeWalletSynchronization.NonActivated");
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    auto configuration = DynamicObject::newInstance();

    configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'/<node>'/<address>");
    configuration->putString(
        api::TezosConfiguration::TEZOS_XPUB_CURVE,
        api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519
    );
    configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::BlockchainExplorerEngines::TZSTATS_API);
    configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, "https://api.tzstats.com/explorer");

    auto wallet = uv::wait(pool->createWallet(uuid::generate_uuid_v4(), "tezos", configuration));
    std::set<std::string> emittedOperations;

    auto nextIndex = uv::wait(wallet->getNextAccountIndex());
    EXPECT_EQ(nextIndex, 0);

    auto account = createTezosLikeAccount(wallet, nextIndex, XTZ_NON_ACTIVATED_KEYS_INFO);

    auto receiver =
        make_receiver([&](const std::shared_ptr<api::Event> &event) {
          if (event->getCode() == api::EventCode::NEW_OPERATION) {
            auto uid = event->getPayload()
                           ->getString(api::Account::EV_NEW_OP_UID)
                           .value();
            EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
          }
        });

    auto eventBus = pool->getEventBus();
    eventBus->subscribe(getTestExecutionContext(), receiver);

    receiver.reset();
    receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
      fmt::print("Received event {}\n", api::to_string(event->getCode()));
      if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
        return;
      EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
      EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);

      auto balance = uv::wait(account->getBalance());
      EXPECT_GT(balance->toLong(), 0L);

      dispatcher->stop();
    });

    auto restoreKey = account->getRestoreKey();
    EXPECT_EQ(restoreKey, hex::toString(XTZ_NON_ACTIVATED_KEYS_INFO.publicKeys[0]));
    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(), receiver);

    dispatcher->waitUntilStopped();
}
