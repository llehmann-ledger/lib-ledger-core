// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from http_client.djinni

#import "LGHttpUrlConnection+Private.h"
#import "LGHttpUrlConnection.h"
#import "DJIMarshal+Private.h"
#import "DJIObjcWrapperCache+Private.h"
#import "LGHttpReadBodyResult+Private.h"
#include <stdexcept>

static_assert(__has_feature(objc_arc), "Djinni requires ARC to be enabled for this file");

namespace djinni_generated {

class HttpUrlConnection::ObjcProxy final
: public ::ledger::core::api::HttpUrlConnection
, public ::djinni::ObjcProxyCache::Handle<ObjcType>
{
public:
    using Handle::Handle;
    int32_t getStatusCode() override
    {
        @autoreleasepool {
            auto objcpp_result_ = [Handle::get() getStatusCode];
            return ::djinni::I32::toCpp(objcpp_result_);
        }
    }
    std::string getStatusText() override
    {
        @autoreleasepool {
            auto objcpp_result_ = [Handle::get() getStatusText];
            return ::djinni::String::toCpp(objcpp_result_);
        }
    }
    std::unordered_map<std::string, std::string> getHeaders() override
    {
        @autoreleasepool {
            auto objcpp_result_ = [Handle::get() getHeaders];
            return ::djinni::Map<::djinni::String, ::djinni::String>::toCpp(objcpp_result_);
        }
    }
    ::ledger::core::api::HttpReadBodyResult readBody() override
    {
        @autoreleasepool {
            auto objcpp_result_ = [Handle::get() readBody];
            return ::djinni_generated::HttpReadBodyResult::toCpp(objcpp_result_);
        }
    }
};

}  // namespace djinni_generated

namespace djinni_generated {

auto HttpUrlConnection::toCpp(ObjcType objc) -> CppType
{
    if (!objc) {
        return nullptr;
    }
    return ::djinni::get_objc_proxy<ObjcProxy>(objc);
}

auto HttpUrlConnection::fromCppOpt(const CppOptType& cpp) -> ObjcType
{
    if (!cpp) {
        return nil;
    }
    return dynamic_cast<ObjcProxy&>(*cpp).Handle::get();
}

}  // namespace djinni_generated
