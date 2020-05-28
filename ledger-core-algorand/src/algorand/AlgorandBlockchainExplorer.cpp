/*
 * AlgorandBlockchainExplorer
 *
 * Created by Hakim Aammar on 20/04/2020.
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

#include <algorand/AlgorandBlockchainExplorer.hpp>
#include <algorand/AlgorandJsonParser.hpp>
#include <algorand/api/AlgorandConfigurationDefaults.hpp>

#include <core/api/Configuration.hpp>

namespace ledger {
namespace core {
namespace algorand {

    BlockchainExplorer::BlockchainExplorer(
            const std::shared_ptr<api::ExecutionContext>& context,
            const std::shared_ptr<HttpClient>& http,
            const api::AlgorandNetworkParameters& parameters,
            const std::shared_ptr<api::DynamicObject>& configuration)
        : ConfigurationMatchable({api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT})
        , DedicatedContext(context)
        , _http(http)
        , _parameters(parameters)
    {
        setConfiguration(configuration);
        _http->addHeader(constants::purestakeTokenHeader, api::AlgorandConfigurationDefaults::ALGORAND_API_TOKEN);
    }

    Future<api::Block> BlockchainExplorer::getCurrentBlock() const
    {
        // TODO ?
    }

    // TODO In algorand::Account set block.currencyName after calling BlockchainExplorer::getBlock !
    Future<api::Block> BlockchainExplorer::getBlock(uint64_t blockHeight) const
    {
        return _http->GET(fmt::format(constants::purestakeBlockEndpoint, blockHeight))
            .json(false)
            .map<api::Block>(getContext(), [](const HttpRequest::JsonResult& response) {
                    const auto& json = std::get<1>(response)->GetObject();
                    auto block = api::Block();
                    JsonParser::parseBlock(json, block);
                    return block;
            });
    }

    Future<model::Account> BlockchainExplorer::getAccount(const std::string& address) const
    {
        return _http->GET(fmt::format(constants::purestakeAccountEndpoint, address))
            .json(false)
            .map<model::Account>(getContext(), [](const HttpRequest::JsonResult& response) {
                    const auto& json = std::get<1>(response)->GetObject();
                    auto account = model::Account();
                    JsonParser::parseAccount(json, account);
                    return account;
            });
    }

    Future<model::AssetParams> BlockchainExplorer::getAssetById(uint64_t id) const
    {
        return _http->GET(fmt::format(constants::purestakeAssetEndpoint, id))
            .json(false)
            .map<model::AssetParams>(
                    getContext(),
                    [](const HttpRequest::JsonResult& response) {
                        auto assetParams = model::AssetParams();
                        const auto& json = std::get<1>(response)->GetObject();
                        JsonParser::parseAssetParams(json, assetParams);
                        return assetParams;
                    });
    }

    Future<model::Transaction>
    BlockchainExplorer::getTransactionById(const std::string & txId) const {
        return _http->GET(fmt::format(constants::purestakeTransactionEndpoint, txId))
            .json(false)
            .map<model::Transaction>(getContext(), [](const HttpRequest::JsonResult& response) {
                    const auto& json = std::get<1>(response)->GetObject();
                    auto tx = model::Transaction();
                    JsonParser::parseTransaction(json, tx);
                    return tx;
            });
    }

    Future<model::TransactionsBulk>
    BlockchainExplorer::getTransactionsForAddress(const std::string& address, uint64_t fromBlockHeight) const
    {
        return _http->GET(fmt::format(constants::purestakeAccountTransactionsEndpoint, address))
            .json(false)
            .map<model::TransactionsBulk>(getContext(), [](const HttpRequest::JsonResult& response) {
                    const auto& json = std::get<1>(response)->GetObject()[constants::xTransactions.c_str()].GetArray();
                    auto txs = model::TransactionsBulk();
                    JsonParser::parseTransactions(json, txs.transactions);
                    // TODO Manage tx->hasNext ? Pagination ?
                    txs.hasNext = false;
                    return txs;
            });
    }

    Future<model::TransactionParams> BlockchainExplorer::getTransactionParams() const
    {
        return _http->GET(constants::purestakeTransactionsParamsEndpoint)
            .json(false)
            .map<model::TransactionParams>(getContext(), [](const HttpRequest::JsonResult& response) {
                    const auto& json = std::get<1>(response)->GetObject();
                    auto txParams = model::TransactionParams();
                    JsonParser::parseTransactionParams(json, txParams);
                    return txParams;
            });
    }

    Future<std::string> BlockchainExplorer::pushTransaction(const std::vector<uint8_t>& transaction)
    {
        return _http->POST(constants::purestakeTransactionsEndpoint, transaction)
            .json(false)
            .map<std::string>(_executionContext, [](const HttpRequest::JsonResult& response) {
                    return std::get<1>(response)->GetObject()[constants::xTxId.c_str()].GetString();
            });
    }

}  // namespace algorand
}  // namespace core
}  // namespace ledger