/*
 * AlgorandAccountDatabaseHelper
 *
 * Created by Hakim Aammar on 18/05/2020.
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

#ifndef LEDGER_CORE_ALGORANDACCOUNTDATABASEHELPER_H
#define LEDGER_CORE_ALGORANDACCOUNTDATABASEHELPER_H

#include <algorand/model/AlgorandAccount.hpp>

#include <algorand/operations/AlgorandOperation.hpp>

#include <core/wallet/AbstractAccount.hpp>
#include <core/wallet/AccountDatabaseHelper.hpp>

#include <soci.h>

namespace ledger {
namespace core {
namespace algorand {

    struct AccountDatabaseEntry {
        int32_t index;
        std::string address;
    };

    class AccountDatabaseHelper : public ledger::core::AccountDatabaseHelper {

    public:

        static std::string createAccount(
            soci::session & sql,
            const std::string & walletUid,
            const AccountDatabaseEntry & account
        );

        static bool queryAccount(
            soci::session & sql,
            const std::string & accountUid,
            AccountDatabaseEntry & account
        );

    };

} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDACCOUNTDATABASEHELPER_H