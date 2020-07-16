/*
 *
 * StellarMigrations
 *
 * Created by Léo Lehmann on 23/06/2020.
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

#include <stellar/database/Migrations.hpp>

namespace ledger {
    namespace core {
        int constexpr StellarMigration::COIN_ID;
        uint32_t constexpr StellarMigration::CURRENT_VERSION;

    	template <> void migrate<1, StellarMigration>(soci::session& sql, api::DatabaseBackendType type) {
            // Stellar currencies
            sql << "CREATE TABLE stellar_currencies("
                    "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                    "identifier VARCHAR(255) NOT NULL,"
                    "address_version VARCHAR(255) NOT NULL,"
                    "base_reserve BIGINT NOT NULL,"
                    "base_fee BIGINT NOT NULL,"
                    "network_passphrase TEXT NOT NULL,"
                    "additional_SEPs TEXT NOT NULL"
                   ")";

            // Stellar assets
            sql << "CREATE TABLE stellar_assets("
                   "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                   "asset_type VARCHAR(255) NOT NULL,"
                   "asset_code VARCHAR(255),"
                   "asset_issuer VARCHAR(255)"
                   ")";

            // Stellar accounts
            sql << "CREATE TABLE stellar_accounts("
                   "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES accounts(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                   "wallet_uid VARCHAR(255) NOT NULL REFERENCES wallets(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                   "idx INTEGER NOT NULL,"
                   "address TEXT NOT NULL,"
                   "subentries_count INTEGER DEFAULT 0,"
                   "sequence VARCHAR(255) DEFAULT '0'"
                   ")";

            // Stellar account signers
            sql << "CREATE TABLE stellar_account_signers("
                   "account_uid VARCHAR(255) REFERENCES stellar_accounts(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                   "weight INTEGER NOT NULL,"
                   "signer_key VARCHAR(255) NOT NULL,"
                   "key_type VARCHAR(255) NOT NULL"
                   ")";

            // Stellar balances
            sql << "CREATE TABLE stellar_account_balances("
                   "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                   "account_uid VARCHAR(255) NOT NULL REFERENCES stellar_accounts(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                   "asset_uid VARCHAR(255) NOT NULL REFERENCES stellar_assets(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                   "amount VARCHAR(255) NOT NULL,"
                   "buying_liabilities VARCHAR(255),"
                   "selling_liabilities VARCHAR(255)"
                   ")";

            // Stellar transactions
            sql << "CREATE TABLE stellar_transactions("
                   "uid VARCHAR(255) PRIMARY KEY NOT NULL, "
                   "hash VARCHAR(255) NOT NULL,"
                   "source_account VARCHAR(255) NOT NULL,"
                   "sequence VARCHAR(255) NOT NULL,"
                   "fee VARCHAR(255) NOT NULL,"
                   "successful INTEGER NOT NULL,"
                   "ledger VARCHAR(255) NOT NULL,"
                   "memo_type TEXT NOT NULL,"
                   "memo TEXT NOT NULL"
                   ")";

            // Stellar native operations
            sql << "CREATE TABLE stellar_operations("
                   "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                   "transaction_uid VARCHAR(255) NOT NULL REFERENCES stellar_transactions(uid) ON DELETE CASCADE,"
                   "hash VARCHAR(255) NOT NULL,"
                   "created_at VARCHAR(255) NOT NULL,"
                   "asset_uid VARCHAR(255) NOT NULL REFERENCES stellar_assets(uid) ON DELETE CASCADE,"
                   "source_asset_uid VARCHAR(255) REFERENCES stellar_assets(uid) ON DELETE CASCADE,"
                   "amount VARCHAR(255) NOT NULL,"
                   "source_amount VARCHAR(255),"
                   "from_address VARCHAR(255) NOT NULL,"
                   "to_address VARCHAR(255) NOT NULL,"
                   "type INTEGER NOT NULL"
                   ")";


            // Stellar account operations
            sql << "CREATE TABLE stellar_account_operations("
                   "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                   "operation_uid VARCHAR(255) NOT NULL REFERENCES stellar_operations(uid) ON DELETE CASCADE"
                   ")";

            sql << "CREATE TABLE stellar_ledgers("
                   "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES blocks(uid) ON DELETE CASCADE,"
                   "base_fee VARCHAR(255) NOT NULL,"
                   "base_reserve VARCHAR(255) NOT NULL"
                   ")";
    	}


    	template <> void rollback<1, StellarMigration>(soci::session& sql, api::DatabaseBackendType type) {
            // Stellar ledgers
            sql << "DROP TABLE stellar_ledgers";
            // Stellar account <> operation link table
            sql << "DROP TABLE stellar_account_operations";
            // Stellar "native" operations
            sql << "DROP TABLE stellar_operations";
            // Stellar transactions
            sql << "DROP TABLE stellar_transactions";
            // Stellar balances
            sql << "DROP TABLE stellar_account_balances";
            // Stellar account signers
            sql << "DROP TABLE stellar_account_signers";
            // Stellar accounts
            sql << "DROP TABLE stellar_accounts";
            // Stellar assets
            sql << "DROP TABLE stellar_assets";
            // Stellar currencies
            sql << "DROP TABLE stellar_currencies";
	}

       template <> void migrate<2, StellarMigration>(soci::session& sql, api::DatabaseBackendType type) {
            sql << "ALTER TABLE stellar_currencies ADD muxed_address_version STRING";
            sql << "UPDATE stellar_currencies SET muxed_address_version = '60'";
        }

       template <> void rollback<2, StellarMigration>(soci::session& sql, api::DatabaseBackendType type) {
            // SQLite doesn't handle ALTER TABLE DROP
            if (type != api::DatabaseBackendType::SQLITE3) {
                sql << "ALTER TABLE stellar_currencies DROP muxed_address_version";
            } else {
                sql << "CREATE TABLE stellar_currencies_swap("
                       "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                       "identifier VARCHAR(255) NOT NULL,"
                       "address_version VARCHAR(255) NOT NULL,"
                       "base_reserve BIGINT NOT NULL,"
                       "base_fee BIGINT NOT NULL,"
                       "network_passphrase TEXT NOT NULL,"
                       "additional_SEPs TEXT NOT NULL"
                       ")";
                sql << "INSERT INTO stellar_currencies_swap "
                       "SELECT name, identifier, address_version, base_reserve, base_fee, "
                       "network_passphrase, additional_SEPs "
                       "FROM stellar_currencies";
                sql << "DROP TABLE stellar_currencies";
                sql << "ALTER TABLE stellar_currencies_swap RENAME TO stellar_currencies";
            }
       }
    }
}
