// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from callback.djinni

#ifndef DJINNI_GENERATED_ALGORANDASSETAMOUNTLISTCALLBACK_HPP_JNI_
#define DJINNI_GENERATED_ALGORANDASSETAMOUNTLISTCALLBACK_HPP_JNI_

#include "../../api/AlgorandAssetAmountListCallback.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class AlgorandAssetAmountListCallback final : ::djinni::JniInterface<::ledger::core::api::AlgorandAssetAmountListCallback, AlgorandAssetAmountListCallback> {
public:
    using CppType = std::shared_ptr<::ledger::core::api::AlgorandAssetAmountListCallback>;
    using CppOptType = std::shared_ptr<::ledger::core::api::AlgorandAssetAmountListCallback>;
    using JniType = jobject;

    using Boxed = AlgorandAssetAmountListCallback;

    ~AlgorandAssetAmountListCallback();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<AlgorandAssetAmountListCallback>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<AlgorandAssetAmountListCallback>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    AlgorandAssetAmountListCallback();
    friend ::djinni::JniClass<AlgorandAssetAmountListCallback>;
    friend ::djinni::JniInterface<::ledger::core::api::AlgorandAssetAmountListCallback, AlgorandAssetAmountListCallback>;

    class JavaProxy final : ::djinni::JavaProxyHandle<JavaProxy>, public ::ledger::core::api::AlgorandAssetAmountListCallback
    {
    public:
        JavaProxy(JniType j);
        ~JavaProxy();

        void onCallback(const std::experimental::optional<std::vector<::ledger::core::api::AlgorandAssetAmount>> & result, const std::experimental::optional<::ledger::core::api::Error> & error) override;

    private:
        friend ::djinni::JniInterface<::ledger::core::api::AlgorandAssetAmountListCallback, ::djinni_generated::AlgorandAssetAmountListCallback>;
    };

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("co/ledger/core/AlgorandAssetAmountListCallback") };
    const jmethodID method_onCallback { ::djinni::jniGetMethodID(clazz.get(), "onCallback", "(Ljava/util/ArrayList;Lco/ledger/core/Error;)V") };
};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_ALGORANDASSETAMOUNTLISTCALLBACK_HPP_JNI_
