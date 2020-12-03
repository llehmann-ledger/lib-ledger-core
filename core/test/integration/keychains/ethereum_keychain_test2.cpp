/*
 *
 * keychain_test_helper
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 08/07/2018.
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


#include <gtest/gtest.h>
#include <src/wallet/ethereum/keychains/EthereumLikeKeychain.hpp>
#include <src/ethereum/EthereumLikeExtendedPublicKey.h>
#include <src/ethereum/EthereumLikeAddress.h>
#include <src/utils/DerivationPath.hpp>
#include <src/utils/optional.hpp>
#include "keychain_test_helper.h"
#include "../BaseFixture.h"
#include <iostream>
#include <Uuid.hpp>
using namespace std;
class EthereumKeychains2 : public BaseFixture {
};

struct DerivationSchemeTestData {
    std::vector<std::string> equivalentDerivationSchemes;
    std::string pubKey;
    std::string chainCode;
    std::string expectedAddress;
};
const std::vector<DerivationSchemeTestData> derivationSchemeTestData = {
        {
                {"44'/<coin_type>'/<account>'/<node>'","44'/60'/<account>'/<node>'", "44'/60'/0'/<node>'","44'/60'/<account>'/0'","44'/60'/0'/0'"},
                "04d2ee4bb49221f9f1662e4791748e68354c26d7d5290ad518c86c4d714c785e6533e0286d3803b0ddde3287eb6f31f77792fdf7323f76152c14069805f23121d2",
                "ddf5a9cf1fdf4746a4495cf36328c7e2af31d18dd0a8f8302f3e13c900f4bfb9",
                "0x390De614378307a6d85cD0e68460378A745295b1"
        }
};

TEST_F(EthereumKeychains2, EthereumDerivationSchemes) {
    auto pool = newDefaultPool(uuid::generate_uuid_v4());
    auto configuration = DynamicObject::newInstance();
    {
        for (auto &elem : derivationSchemeTestData) {
            auto derivationSchemes = elem.equivalentDerivationSchemes;
            for (auto &scheme : derivationSchemes) {
                configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,scheme);
                auto wallet = uv::wait(pool->createWallet(scheme, "ethereum", configuration));
                //Create account as Live does
                api::AccountCreationInfo info = uv::wait(wallet->getNextAccountCreationInfo());
                info.publicKeys.push_back(hex::toByteArray(elem.pubKey));
                info.chainCodes.push_back(hex::toByteArray(elem.chainCode));
                auto account = createEthereumLikeAccount(wallet, info.index, info);
                auto addresses = uv::wait(account->getFreshPublicAddresses());
                EXPECT_GT(addresses.size(), 0);
                EXPECT_EQ(addresses[0]->toString(), elem.expectedAddress);
            }
        }
    }
}
