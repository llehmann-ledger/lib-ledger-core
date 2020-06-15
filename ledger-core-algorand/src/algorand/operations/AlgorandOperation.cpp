/*
 * AlgorandOperation
 *
 * Created by Rémi Barjon on 12/05/2020.
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

#include <algorand/operations/AlgorandOperation.hpp>
#include <algorand/AlgorandAccount.hpp>

#include "../transactions/api_impl/AlgorandTransactionImpl.hpp"

#include <core/operation/OperationDatabaseHelper.hpp>

#include <fmt/format.h>
#include <sys/stat.h>

namespace ledger {
namespace core {
namespace algorand {

    Operation::Operation(const std::shared_ptr<AbstractAccount>& account)
        : ::ledger::core::Operation(account)
        , transaction(nullptr)
        , algorandType{}
    {}

    Operation::Operation(const std::shared_ptr<AbstractAccount>& account,
                         const model::Transaction& txn)
        : Operation(account)
    {
        setTransaction(txn);
        inflate();
    }

    api::AlgorandOperationType Operation::getAlgorandOperationType() const
    {
        return algorandType;
    }

    std::shared_ptr<api::AlgorandTransaction> Operation::getTransaction() const
    {
        return transaction;
    }

    void Operation::refreshUid(const std::string&)
    {
        const auto& txn = getTransactionData();
        const auto id =
            fmt::format("{}+{}", *txn.header.id, api::to_string(algorandType));
        uid = OperationDatabaseHelper::createUid(accountUid, id, getOperationType());
    }

    const model::Transaction& Operation::getTransactionData() const
    {
        return transaction->getTransactionData();
    }

    void Operation::setTransaction(const model::Transaction& txn)
    {
        transaction = std::make_shared<AlgorandTransactionImpl>(txn);
    }

    void Operation::inflate()
    {
        if (!_account || !transaction) { return; }
        inflateFromAccount();
        inflateFromTransaction();
    }

    void Operation::inflateFromAccount()
    {
        const auto& account = getAlgorandAccount();
        accountUid = account.getAccountUid();
        walletUid = account.getWallet()->getWalletUid();
        currencyName = account.getWallet()->getCurrency().name;
        trust = std::make_shared<TrustIndicator>();
    }

    void Operation::inflateFromTransaction()
    {
        inflateDate();
        inflateBlock();
        inflateAmountAndFees();
        inflateSenders();
        inflateRecipients();
        inflateType();
        setAlgorandOperationType();
        refreshUid();
    }

    void Operation::inflateDate()
    {
        const auto& txn = getTransactionData();
        date = std::chrono::system_clock::time_point(
            std::chrono::seconds(txn.header.timestamp.getValueOr(0)));
    }

    void Operation::inflateBlock()
    {
        const auto& txn = getTransactionData();
        block = [&]() {
            api::Block block;
            block.currencyName = currencyName;
            if (txn.header.round) {
                if (*txn.header.round > std::numeric_limits<int64_t>::max()) {
                    throw make_exception(api::ErrorCode::OUT_OF_RANGE, "Block height exceeds maximum value");
                }
                block.height = static_cast<int64_t>(*txn.header.round);
                block.blockHash = std::to_string(*txn.header.round);
            }
            if (txn.header.timestamp) {
                block.time = std::chrono::system_clock::time_point(std::chrono::seconds(*txn.header.timestamp));
            }
            block.uid = BlockDatabaseHelper::createBlockUid(block);
            return block;
        }();
    }


    void Operation::inflateAmountAndFees()
    {
        const auto& accountAddr = getAlgorandAccount().getAddress();
        const auto& txn = getTransactionData();
        uint64_t fullAmount = 0;
        if (accountAddr == txn.header.sender.toString()) {
            fullAmount += txn.header.fee;
            fees = BigInt(static_cast<unsigned long long>(fullAmount));
        }
        if (txn.header.type == model::constants::pay) {
            const auto& details = boost::get<model::PaymentTxnFields>(txn.details);
            if (txn.header.sender == details.receiverAddr) {
                amount = BigInt(static_cast<unsigned long long>(fullAmount));
                return;
            }
            if (accountAddr == txn.header.sender.toString()) {
                fullAmount += details.amount;
                fullAmount -= details.fromRewards.getValueOr(0);
            }
            if (accountAddr == details.receiverAddr.toString()) {
                fullAmount += details.amount;
                fullAmount += details.receiverRewards.getValueOr(0);
            }
            if (details.closeAddr && accountAddr == details.closeAddr->toString()) {
                fullAmount += details.closeAmount.getValueOr(0);
                fullAmount += details.closeRewards.getValueOr(0);
            }
            amount = BigInt(static_cast<unsigned long long>(fullAmount));
        }
    }

    void Operation::inflateSenders()
    {
        const auto& txn = getTransactionData();
        senders = { txn.header.sender.toString() };
    }

    void Operation::inflateRecipients()
    {
        const auto& txn = getTransactionData();
        recipients = {};
        if (txn.header.type == model::constants::pay) {
            const auto& details = boost::get<model::PaymentTxnFields>(txn.details);
            recipients.push_back(details.receiverAddr.toString());
            if (details.closeAddr) {
                recipients.push_back(details.closeAddr->toString());
            }
        } else if (txn.header.type == model::constants::axfer) {
            const auto& details = boost::get<model::AssetTransferTxnFields>(txn.details);
            recipients.push_back(details.assetReceiver.toString());
            if (details.assetCloseTo) {
                recipients.push_back(details.assetCloseTo->toString());
            }
        }
    }

    void Operation::inflateType()
    {
        const auto& accountAddr = getAlgorandAccount().getAddress();
        const auto& txn = getTransactionData();
        type = api::OperationType::NONE;

        if (txn.header.sender.toString() == accountAddr) {
            type = api::OperationType::SEND;
        } else if (txn.header.type == model::constants::pay) {
            const auto& details = boost::get<model::PaymentTxnFields>(txn.details);
            if (details.receiverAddr.toString() == accountAddr ||
                details.closeAddr && details.closeAddr->toString() == accountAddr) {
                type = api::OperationType::RECEIVE;
            }
        } else if (txn.header.type == model::constants::axfer) {
            const auto& details = boost::get<model::AssetTransferTxnFields>(txn.details);
            if (details.assetReceiver.toString() == accountAddr ||
                details.assetCloseTo && details.assetCloseTo->toString() == accountAddr) {
                type = api::OperationType::RECEIVE;
            }
        }
    }


    const Account& Operation::getAlgorandAccount() const
    {
        return static_cast<Account&>(*getAccount());
    }

    void Operation::setAlgorandOperationType()
    {
        const auto& txn = transaction->getTransactionData();
        if (txn.header.type == model::constants::pay) {
            const auto& details = boost::get<model::PaymentTxnFields>(txn.details);
            if (details.closeAddr) {
                algorandType = api::AlgorandOperationType::ACCOUNT_CLOSE;
            } else {
                algorandType = api::AlgorandOperationType::PAYMENT;
            }
        } else if (txn.header.type == model::constants::keyreg) {
            const auto& details = boost::get<model::KeyRegTxnFields>(txn.details);
            if (details.nonParticipation && !(*details.nonParticipation)) {
                algorandType = api::AlgorandOperationType::ACCOUNT_REGISTER_ONLINE;
            } else {
                algorandType = api::AlgorandOperationType::ACCOUNT_REGISTER_OFFLINE;
            }
        } else if (txn.header.type == model::constants::acfg) {
            const auto& details = boost::get<model::AssetConfigTxnFields>(txn.details);
            if (!details.assetId) {
                algorandType = api::AlgorandOperationType::ASSET_CREATE;
            } else if (!details.assetParams) {
                algorandType = api::AlgorandOperationType::ASSET_DESTROY;
            } else {
                algorandType = api::AlgorandOperationType::ASSET_RECONFIGURE;
            }
        } else if (txn.header.type == model::constants::axfer) {
            const auto& details = boost::get<model::AssetTransferTxnFields>(txn.details);
            if (details.assetSender) {
                algorandType = api::AlgorandOperationType::ASSET_REVOKE;
            } else if (details.assetCloseTo) {
                algorandType = api::AlgorandOperationType::ASSET_OPT_OUT;
            } else if (!details.assetAmount) {
                algorandType = api::AlgorandOperationType::ASSET_OPT_IN;
            } else {
                algorandType = api::AlgorandOperationType::ASSET_TRANSFER;
            }
        } else if (txn.header.type == model::constants::afreeze) {
            algorandType = api::AlgorandOperationType::ASSET_FREEZE;
        } else {
            algorandType = api::AlgorandOperationType::UNSUPPORTED;
        }
    }

} // namespace algorand
} // namespace core
} // namespace ledger

