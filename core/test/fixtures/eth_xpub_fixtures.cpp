// This file was GENERATED by command:
//     generate_fixtures.py
// DO NOT EDIT BY HAND!!!
#include "eth_xpub_fixtures.h"

namespace ledger {
	namespace testing {
		namespace eth_xpub {
			core::api::ExtendedKeyAccountCreationInfo XPUB_INFO(
			        0, {"eth"}, {"49'/1'/0'"}, {"xpub661MyMwAqRbcGout4B6s29b6gGQsowyoiF6UgXBEr7eFCWYfXuZDvRxP9zEh1Kwq3TLqDQMbkbaRpSnoC28oWvjLeshoQz1StZ9YHM1EpcJ"}
			);
			std::shared_ptr<core::EthereumLikeAccount> inflate(const std::shared_ptr<core::WalletPool>& pool, const std::shared_ptr<core::AbstractWallet>& wallet) {
				auto account = std::dynamic_pointer_cast<core::EthereumLikeAccount>(wait(wallet->newAccountWithExtendedKeyInfo(XPUB_INFO)));
				soci::session sql(pool->getDatabaseSessionPool()->getPool());
				sql.begin();				sql.commit();
				return account;
			}
		}
	}
}
