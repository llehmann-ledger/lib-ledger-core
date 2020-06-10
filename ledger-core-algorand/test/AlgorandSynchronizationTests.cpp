/*
 * AlgorandSynchronizationTests
 *
 * Created by Hakim Aammar on 01/06/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include "AlgorandTestFixtures.hpp"
#include <algorand/AlgorandAccount.hpp>
#include <algorand/AlgorandWalletFactory.hpp>
#include <algorand/AlgorandWallet.hpp>
#include <algorand/AlgorandLikeCurrencies.hpp>
#include <algorand/AlgorandNetworks.hpp>
#include <algorand/operations/AlgorandOperationQuery.hpp>
#include <algorand/api/AlgorandConfigurationDefaults.hpp>
#include <algorand/api/AlgorandBlockchainExplorerEngines.hpp>
#include <core/api/Configuration.hpp>

#include <integration/WalletFixture.hpp>

#include <functional>

using namespace ledger::testing::algorand;
using namespace ledger::core::algorand;

class AlgorandSynchronizationTest : public WalletFixture<WalletFactory> {
};

TEST_F(AlgorandSynchronizationTest, AccountSynchronizationTest) {
    registerCurrency(currencies::algorand());

    auto configuration = DynamicObject::newInstance();
    configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"44'/<coin_type>'/<account>'/<node>'/<address>");
    configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::AlgorandBlockchainExplorerEngines::ALGORAND_NODE);
    configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, api::AlgorandConfigurationDefaults::ALGORAND_API_ENDPOINT);

    auto wallet = std::dynamic_pointer_cast<Wallet>(wait(walletStore->createWallet("test-wallet", "algorand", configuration)));

    auto nextIndex = wait(wallet->getNextAccountIndex());
    EXPECT_EQ(nextIndex, 0);

    const api::AccountCreationInfo info(
        nextIndex,
        {"main"},
        {"44'/283'/0'/0'"},
        {{ std::begin(OBELIX_ADDRESS), std::end(OBELIX_ADDRESS) }},
        {hex::toByteArray("")}
    );

    auto account = std::dynamic_pointer_cast<Account>(wait(wallet->newAccountWithInfo(info)));
    auto internalPreferences = account->getInternalPreferences()->getSubPreferences("AlgorandAccountSynchronizer");

    auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
        fmt::print("Received event {}\n", api::to_string(event->getCode()));

        if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED) {
            return;
        }
        EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);

        dispatcher->stop();
    });

    account->synchronize()->subscribe(dispatcher->getMainExecutionContext(), receiver);
    dispatcher->waitUntilStopped();

    auto savedState = internalPreferences->template getObject<SavedState>("state");
    std::cout << ">>> Saved block round for next synchronization: " << savedState.getValue().round << std::endl;
    EXPECT_GT(savedState.getValue().round, 6000000);

    auto operations = wait(std::dynamic_pointer_cast<algorand::OperationQuery>(account->queryOperations()->complete())->execute());
    std::cout << ">>> Nb of operations: " << operations.size() << std::endl;
    EXPECT_GT(operations.size(), 20); // 27 on TestNet as of 08 June 2020

}

