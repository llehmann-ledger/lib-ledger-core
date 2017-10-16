// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#ifndef DJINNI_GENERATED_BLOCK_HPP_JNI_
#define DJINNI_GENERATED_BLOCK_HPP_JNI_

#include "../../api/Block.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class Block final {
public:
    using CppType = ::ledger::core::api::Block;
    using JniType = jobject;

    using Boxed = Block;

    ~Block();

    static CppType toCpp(JNIEnv* jniEnv, JniType j);
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c);

private:
    Block();
    friend ::djinni::JniClass<Block>;

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("co/ledger/core/Block") };
    const jmethodID jconstructor { ::djinni::jniGetMethodID(clazz.get(), "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/util/Date;Ljava/lang/String;J)V") };
    const jfieldID field_hash { ::djinni::jniGetFieldID(clazz.get(), "hash", "Ljava/lang/String;") };
    const jfieldID field_uid { ::djinni::jniGetFieldID(clazz.get(), "uid", "Ljava/lang/String;") };
    const jfieldID field_time { ::djinni::jniGetFieldID(clazz.get(), "time", "Ljava/util/Date;") };
    const jfieldID field_currencyName { ::djinni::jniGetFieldID(clazz.get(), "currencyName", "Ljava/lang/String;") };
    const jfieldID field_height { ::djinni::jniGetFieldID(clazz.get(), "height", "J") };
};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_BLOCK_HPP_JNI_
