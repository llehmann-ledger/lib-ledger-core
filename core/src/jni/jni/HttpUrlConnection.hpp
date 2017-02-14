// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from http_client.djinni

#pragma once

#include "../../api/HttpUrlConnection.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class HttpUrlConnection final : ::djinni::JniInterface<::ledger::core::api::HttpUrlConnection, HttpUrlConnection> {
public:
    using CppType = std::shared_ptr<::ledger::core::api::HttpUrlConnection>;
    using CppOptType = std::shared_ptr<::ledger::core::api::HttpUrlConnection>;
    using JniType = jobject;

    using Boxed = HttpUrlConnection;

    ~HttpUrlConnection();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<HttpUrlConnection>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<HttpUrlConnection>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    HttpUrlConnection();
    friend ::djinni::JniClass<HttpUrlConnection>;
    friend ::djinni::JniInterface<::ledger::core::api::HttpUrlConnection, HttpUrlConnection>;

    class JavaProxy final : ::djinni::JavaProxyHandle<JavaProxy>, public ::ledger::core::api::HttpUrlConnection
    {
    public:
        JavaProxy(JniType j);
        ~JavaProxy();

        int32_t getStatusCode() override;
        std::string getStatusText() override;
        std::unordered_map<std::string, std::string> getHeaders() override;
        ::ledger::core::api::HttpReadBodyResult readBody() override;

    private:
        friend ::djinni::JniInterface<::ledger::core::api::HttpUrlConnection, ::djinni_generated::HttpUrlConnection>;
    };

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("co/ledger/core/HttpUrlConnection") };
    const jmethodID method_getStatusCode { ::djinni::jniGetMethodID(clazz.get(), "getStatusCode", "()I") };
    const jmethodID method_getStatusText { ::djinni::jniGetMethodID(clazz.get(), "getStatusText", "()Ljava/lang/String;") };
    const jmethodID method_getHeaders { ::djinni::jniGetMethodID(clazz.get(), "getHeaders", "()Ljava/util/HashMap;") };
    const jmethodID method_readBody { ::djinni::jniGetMethodID(clazz.get(), "readBody", "()Lco/ledger/core/HttpReadBodyResult;") };
};

}  // namespace djinni_generated
