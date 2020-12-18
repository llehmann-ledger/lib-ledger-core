/*
 *
 * WalletFixture
 * ledger-core
 *
 * Created by Alexis Le Provost on 27/01/2020.
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

#pragma once

#include "BaseFixture.h"
#include <wallet/common/AbstractWallet.hpp>
#include <wallet/pool/WalletPool.hpp>
#include <collections/DynamicObject.hpp>
#include <api/ErrorCode.hpp>
#include <api/PoolConfiguration.hpp>
#include <Uuid.hpp>
#include <fmt/format.h>


class WalletFixture : public BaseFixture {
public:

    void SetUp() override {
        BaseFixture::SetUp();

#ifdef PG_SUPPORT
        const bool usePostgreSQL = true;
        auto poolConfig = DynamicObject::newInstance();
        poolConfig->putString(
            api::PoolConfiguration::DATABASE_NAME, "postgres://localhost:5432/test_db");
        pool = newDefaultPool(uuid::generate_uuid_v4(), "", poolConfig, usePostgreSQL);
#else
        pool = newDefaultPool(uuid::generate_uuid_v4());
#endif
    }

    void TearDown() override {
        BaseFixture::TearDown();
        uv::wait(pool->eraseDataSince(std::chrono::time_point<std::chrono::system_clock>{}));
        pool.reset();
    }

    std::shared_ptr<WalletPool> pool;
};
