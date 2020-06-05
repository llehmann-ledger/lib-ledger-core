// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#ifndef DJINNI_GENERATED_COSMOSLIKEREWARD_HPP_JNI_
#define DJINNI_GENERATED_COSMOSLIKEREWARD_HPP_JNI_

#include "../../api/CosmosLikeReward.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class CosmosLikeReward final : ::djinni::JniInterface<::ledger::core::api::CosmosLikeReward, CosmosLikeReward> {
public:
    using CppType = std::shared_ptr<::ledger::core::api::CosmosLikeReward>;
    using CppOptType = std::shared_ptr<::ledger::core::api::CosmosLikeReward>;
    using JniType = jobject;

    using Boxed = CosmosLikeReward;

    ~CosmosLikeReward();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<CosmosLikeReward>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<CosmosLikeReward>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    CosmosLikeReward();
    friend ::djinni::JniClass<CosmosLikeReward>;
    friend ::djinni::JniInterface<::ledger::core::api::CosmosLikeReward, CosmosLikeReward>;

};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_COSMOSLIKEREWARD_HPP_JNI_