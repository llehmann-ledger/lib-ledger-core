/*
 *
 * StellarFixture.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 18/02/2019.
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

#ifndef LEDGER_CORE_STELLARFIXTURE_HPP
#define LEDGER_CORE_STELLARFIXTURE_HPP

#include <gtest/gtest.h>
#include <stellar/StellarLikeWallet.hpp>
#include <stellar/StellarLikeAccount.hpp>
#include <stellar/StellarLikeOperation.hpp>
#include <core/wallet/CurrencyBuilder.hpp>
#include <core/api/Account.hpp>
#include <core/api/AccountCreationInfo.hpp>
#include <integration/IntegrationEnvironment.hpp>

static ledger::core::api::Currency STELLAR =
        ledger::core::CurrencyBuilder("stellar")
        .bip44(148)
        .paymentUri("web+stellar")
        .unit("stroops", 0, "stroops")
        .unit("lumen", 7, "XLM");

static ledger::core::api::StellarLikeNetworkParameters STELLAR_PARAMS {
    "xlm", {6 << 3},  {12 << 3}, 5000000, 100, {}, "Public Global Stellar Network ; September 2015"
};

class StellarFixture {
public:

    ledger::core::api::AccountCreationInfo accountInfo(const std::string& pubKey) const;
    ledger::core::api::AccountCreationInfo defaultAccount() const;
    ledger::core::api::AccountCreationInfo emptyAccount() const;
    ledger::core::api::AccountCreationInfo accountInfoFromAddress(const std::string& address) const;

    ledger::core::api::Currency getCurrency() const;
};


#endif //LEDGER_CORE_STELLARFIXTURE_HPP
