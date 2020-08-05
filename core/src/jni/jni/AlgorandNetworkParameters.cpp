// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from idl.djinni

#include "AlgorandNetworkParameters.hpp"  // my header
#include "Marshal.hpp"

namespace djinni_generated {

AlgorandNetworkParameters::AlgorandNetworkParameters() = default;

AlgorandNetworkParameters::~AlgorandNetworkParameters() = default;

auto AlgorandNetworkParameters::fromCpp(JNIEnv* jniEnv, const CppType& c) -> ::djinni::LocalRef<JniType> {
    const auto& data = ::djinni::JniClass<AlgorandNetworkParameters>::get();
    auto r = ::djinni::LocalRef<JniType>{jniEnv->NewObject(data.clazz.get(), data.jconstructor,
                                                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c.genesisID)),
                                                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c.genesisHash)))};
    ::djinni::jniExceptionCheck(jniEnv);
    return r;
}

auto AlgorandNetworkParameters::toCpp(JNIEnv* jniEnv, JniType j) -> CppType {
    ::djinni::JniLocalScope jscope(jniEnv, 3);
    assert(j != nullptr);
    const auto& data = ::djinni::JniClass<AlgorandNetworkParameters>::get();
    return {::djinni::String::toCpp(jniEnv, (jstring)jniEnv->GetObjectField(j, data.field_genesisID)),
            ::djinni::String::toCpp(jniEnv, (jstring)jniEnv->GetObjectField(j, data.field_genesisHash))};
}

}  // namespace djinni_generated
