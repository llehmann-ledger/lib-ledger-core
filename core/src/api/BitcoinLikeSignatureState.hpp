// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from bitcoin_like_wallet.djinni

#ifndef DJINNI_GENERATED_BITCOINLIKESIGNATURESTATE_HPP
#define DJINNI_GENERATED_BITCOINLIKESIGNATURESTATE_HPP

#include <functional>
#include <iostream>
#include <string>
#ifndef LIBCORE_EXPORT
    #if defined(_MSC_VER)
       #include <libcore_export.h>
    #else
       #define LIBCORE_EXPORT
    #endif
#endif

namespace ledger { namespace core { namespace api {

enum class BitcoinLikeSignatureState : int {
    ALREADY_SIGNED,
    MISSING_DATA,
    SIGNING_SUCCEED,
};
LIBCORE_EXPORT  std::string to_string(const BitcoinLikeSignatureState& bitcoinLikeSignatureState);
LIBCORE_EXPORT  std::ostream &operator<<(std::ostream &os, const BitcoinLikeSignatureState &o);

} } }  // namespace ledger::core::api

namespace std {

template <>
struct hash<::ledger::core::api::BitcoinLikeSignatureState> {
    size_t operator()(::ledger::core::api::BitcoinLikeSignatureState type) const {
        return std::hash<int>()(static_cast<int>(type));
    }
};

}  // namespace std
#endif //DJINNI_GENERATED_BITCOINLIKESIGNATURESTATE_HPP
