// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#ifndef DJINNI_GENERATED_CURRENCY_HPP
#define DJINNI_GENERATED_CURRENCY_HPP

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>

namespace ledger { namespace core { namespace api {

struct Currency final {
    std::string name;
    int32_t bip44CoinType;
    int32_t paymentSchemeUri;

    Currency(std::string name_,
             int32_t bip44CoinType_,
             int32_t paymentSchemeUri_)
    : name(std::move(name_))
    , bip44CoinType(std::move(bip44CoinType_))
    , paymentSchemeUri(std::move(paymentSchemeUri_))
    {}

    Currency(const Currency& cpy) {
       this->name = cpy.name;
       this->bip44CoinType = cpy.bip44CoinType;
       this->paymentSchemeUri = cpy.paymentSchemeUri;
    }

    Currency() = default;


    Currency& operator=(const Currency& cpy) {
       this->name = cpy.name;
       this->bip44CoinType = cpy.bip44CoinType;
       this->paymentSchemeUri = cpy.paymentSchemeUri;
       return *this;
    }

    template <class Archive>
    void load(Archive& archive) {
        archive(name, bip44CoinType, paymentSchemeUri);
    }

    template <class Archive>
    void save(Archive& archive) const {
        archive(name, bip44CoinType, paymentSchemeUri);
    }
};

} } }  // namespace ledger::core::api
#endif //DJINNI_GENERATED_CURRENCY_HPP
