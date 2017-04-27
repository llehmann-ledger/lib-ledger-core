// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#import "LGCurrency+Private.h"
#import "DJIMarshal+Private.h"
#include <cassert>

namespace djinni_generated {

auto Currency::toCpp(ObjcType obj) -> CppType
{
    assert(obj);
    return {::djinni::String::toCpp(obj.name),
            ::djinni::I32::toCpp(obj.bip44CoinType),
            ::djinni::I32::toCpp(obj.paymentSchemeUri)};
}

auto Currency::fromCpp(const CppType& cpp) -> ObjcType
{
    return [[LGCurrency alloc] initWithName:(::djinni::String::fromCpp(cpp.name))
                              bip44CoinType:(::djinni::I32::fromCpp(cpp.bip44CoinType))
                           paymentSchemeUri:(::djinni::I32::fromCpp(cpp.paymentSchemeUri))];
}

}  // namespace djinni_generated
