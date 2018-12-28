// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from callback.djinni

#include "OperationListCallback.hpp"  // my header
#include "Error.hpp"
#include "Marshal.hpp"
#include "Operation.hpp"

namespace djinni_generated {

OperationListCallback::OperationListCallback() : ::djinni::JniInterface<::ledger::core::api::OperationListCallback, OperationListCallback>() {}

OperationListCallback::~OperationListCallback() = default;

OperationListCallback::JavaProxy::JavaProxy(JniType j) : Handle(::djinni::jniGetThreadEnv(), j) { }

OperationListCallback::JavaProxy::~JavaProxy() = default;

void OperationListCallback::JavaProxy::onCallback(const std::experimental::optional<std::vector<std::shared_ptr<::ledger::core::api::Operation>>> & c_result, const std::experimental::optional<::ledger::core::api::Error> & c_error) {
    auto jniEnv = ::djinni::jniGetThreadEnv();
    ::djinni::JniLocalScope jscope(jniEnv, 10);
    const auto& data = ::djinni::JniClass<::djinni_generated::OperationListCallback>::get();
    jniEnv->CallVoidMethod(Handle::get().get(), data.method_onCallback,
                           ::djinni::get(::djinni::Optional<std::experimental::optional, ::djinni::List<::djinni_generated::Operation>>::fromCpp(jniEnv, c_result)),
                           ::djinni::get(::djinni::Optional<std::experimental::optional, ::djinni_generated::Error>::fromCpp(jniEnv, c_error)));
    ::djinni::jniExceptionCheck(jniEnv);
}

}  // namespace djinni_generated