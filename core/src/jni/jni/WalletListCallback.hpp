// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from callback.djinni

#ifndef DJINNI_GENERATED_WALLETLISTCALLBACK_HPP
#define DJINNI_GENERATED_WALLETLISTCALLBACK_HPP

#include "../../api/WalletListCallback.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class WalletListCallback final : ::djinni::JniInterface<::ledger::core::api::WalletListCallback, WalletListCallback> {
public:
    using CppType = std::shared_ptr<::ledger::core::api::WalletListCallback>;
    using CppOptType = std::shared_ptr<::ledger::core::api::WalletListCallback>;
    using JniType = jobject;

    using Boxed = WalletListCallback;

    ~WalletListCallback();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<WalletListCallback>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<WalletListCallback>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    WalletListCallback();
    friend ::djinni::JniClass<WalletListCallback>;
    friend ::djinni::JniInterface<::ledger::core::api::WalletListCallback, WalletListCallback>;

    class JavaProxy final : ::djinni::JavaProxyHandle<JavaProxy>, public ::ledger::core::api::WalletListCallback
    {
    public:
        JavaProxy(JniType j);
        ~JavaProxy();

        void onCallback(const std::experimental::optional<std::vector<std::shared_ptr<::ledger::core::api::Wallet>>> & result, const std::experimental::optional<::ledger::core::api::Error> & error) override;

    private:
        friend ::djinni::JniInterface<::ledger::core::api::WalletListCallback, ::djinni_generated::WalletListCallback>;
    };

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("co/ledger/core/WalletListCallback") };
    const jmethodID method_onCallback { ::djinni::jniGetMethodID(clazz.get(), "onCallback", "(Ljava/util/ArrayList;Lco/ledger/core/Error;)V") };
};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_WALLETLISTCALLBACK_HPP
