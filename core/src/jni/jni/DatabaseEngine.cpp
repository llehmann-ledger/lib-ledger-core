// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from database.djinni

#include "DatabaseEngine.hpp"  // my header
#include "DatabaseConnectionPool.hpp"
#include "Marshal.hpp"

namespace djinni_generated {

DatabaseEngine::DatabaseEngine() : ::djinni::JniInterface<::ledger::core::api::DatabaseEngine, DatabaseEngine>() {}

DatabaseEngine::~DatabaseEngine() = default;

DatabaseEngine::JavaProxy::JavaProxy(JniType j) : Handle(::djinni::jniGetThreadEnv(), j) { }

DatabaseEngine::JavaProxy::~JavaProxy() = default;

std::shared_ptr<::ledger::core::api::DatabaseConnectionPool> DatabaseEngine::JavaProxy::connect(const std::string & c_connectUrl) {
    auto jniEnv = ::djinni::jniGetThreadEnv();
    ::djinni::JniLocalScope jscope(jniEnv, 10);
    const auto& data = ::djinni::JniClass<::djinni_generated::DatabaseEngine>::get();
    auto jret = jniEnv->CallObjectMethod(Handle::get().get(), data.method_connect,
                                         ::djinni::get(::djinni::String::fromCpp(jniEnv, c_connectUrl)));
    ::djinni::jniExceptionCheck(jniEnv);
    return ::djinni_generated::DatabaseConnectionPool::toCpp(jniEnv, jret);
}

}  // namespace djinni_generated
