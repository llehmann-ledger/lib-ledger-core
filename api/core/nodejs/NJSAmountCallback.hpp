// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from callback.djinni

#ifndef DJINNI_GENERATED_NJSAMOUNTCALLBACK_HPP
#define DJINNI_GENERATED_NJSAMOUNTCALLBACK_HPP


#include "../../../core/src/api/../utils/optional.hpp"
#include "../../../core/src/api/Amount.hpp"
#include "../../../core/src/api/Error.hpp"
#include "NJSAmountCpp.hpp"
#include <memory>

#include <nan.h>
#include <node.h>
#include "../../../core/src/api/AmountCallback.hpp"

using namespace v8;
using namespace node;
using namespace std;
using namespace ledger::core::api;

class NJSAmountCallback: public Nan::ObjectWrap, public ledger::core::api::AmountCallback {
public:

    static void Initialize(Local<Object> target);
    ~NJSAmountCallback()
    {
        persistent().Reset();
        pers_resolver.Reset();
    };
    NJSAmountCallback(Local<Promise::Resolver> resolver){pers_resolver.Reset(resolver);};

    void onCallback(const std::shared_ptr<Amount> & result, const std::experimental::optional<Error> & error);

private:
    static NAN_METHOD(onCallback);

    static NAN_METHOD(New);

    static NAN_METHOD(addRef);
    static NAN_METHOD(removeRef);
    Nan::Persistent<Promise::Resolver> pers_resolver;
};
#endif //DJINNI_GENERATED_NJSAMOUNTCALLBACK_HPP
