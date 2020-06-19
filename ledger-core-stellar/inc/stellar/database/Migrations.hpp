#pragma once

#include <core/api/DatabaseBackendType.hpp>
#include <core/database/Migrations.hpp>

#include <stellar/StellarLikeCoinID.hpp>

namespace ledger {
    namespace core {
        /// Tag type.
        struct StellarMigration {
          static int constexpr COIN_ID = STELLAR_COIN_ID;
          static uint32_t constexpr CURRENT_VERSION = 1;
        };

        //TODO
  }
}
