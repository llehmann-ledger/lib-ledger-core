/*
 *
 * StellarLikeWalletFactory.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/02/2019.
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

#include <stellar/factories/StellarLikeWalletFactory.hpp>
#include <stellar/StellarLikeWallet.hpp>
#include <stellar/factories/StellarLikeKeychainFactory.hpp>
#include <core/Services.hpp>
#include <stellar/explorers/HorizonBlockchainExplorer.hpp>
#include <stellar/api/StellarConfiguration.hpp>
#include <stellar/synchronizers/StellarLikeBlockchainExplorerAccountSynchronizer.hpp>
#include <core/api/Configuration.hpp>
#include <stellar/stellarNetworks.h>

#define STRING(key, def) entry.configuration->getString(key).value_or(def)

namespace ledger {
    namespace core {

        StellarLikeWalletFactory::StellarLikeWalletFactory(const api::Currency &currency,
                                                           const std::shared_ptr<Services> &services)
                : AbstractWalletFactory(currency, services) {

        }

        std::shared_ptr<AbstractWallet> StellarLikeWalletFactory::build(const WalletDatabaseEntry &entry) {
            StellarLikeWalletParams params;
            // Get currency
            try {
                auto currency = networks::getStellarLikeNetworkParameters(entry.currencyName);
            } catch (Exception) {
                throw make_exception(api::ErrorCode::UNSUPPORTED_CURRENCY, "Unsupported currency '{}'.", entry.currencyName);
            }
            // Get derivation scheme
            DerivationScheme scheme(STRING(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "44'/<coin_type>'/<account>'"));

            // Configure keychain factory
            params.keychainFactory = std::make_shared<StellarLikeKeychainFactory>();

            // Configure explorer
            params.blockchainExplorer = getExplorer(entry);
            // Configure observer

            // Configure synchronizer
            // This line will need to be changed if we have multiple synchronizers for now it's ok.
            params.accountSynchronizer = std::make_shared<StellarLikeBlockchainExplorerAccountSynchronizer>(getServices(), params.blockchainExplorer);

            return std::make_shared<StellarLikeWallet>(entry.name, getCurrency(), getServices(), entry.configuration, scheme, params);
        }

        std::shared_ptr<StellarLikeBlockchainExplorer> StellarLikeWalletFactory::getExplorer(const WalletDatabaseEntry& entry) {
            auto engine = STRING(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::StellarConfiguration::HORIZON_EXPLORER_ENGINE);
            std::shared_ptr<StellarLikeBlockchainExplorer> explorer;
            if (engine == api::StellarConfiguration::HORIZON_EXPLORER_ENGINE) {
                auto baseUrl = STRING(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, api::StellarConfiguration::HORIZON_MAINNET_BLOCKCHAIN_EXPLORER_URL);
                explorer = std::make_shared<HorizonBlockchainExplorer>(getServices()->getDispatcher()->getSerialExecutionContext("stellar_explorer"), getServices()->getHttpClient(baseUrl), entry.configuration);
            }
            return explorer;
        }

        inline std::shared_ptr<api::ExecutionContext> StellarLikeWalletFactory::getContext(const WalletDatabaseEntry& entry) {
            return getServices()->getDispatcher()->getSerialExecutionContext(fmt::format("stellar:{}", entry.name));
        }

    }
}
