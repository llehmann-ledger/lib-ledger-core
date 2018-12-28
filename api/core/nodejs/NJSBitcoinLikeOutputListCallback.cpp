// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from callback.djinni

#include "NJSBitcoinLikeOutputListCallback.hpp"

using namespace v8;
using namespace node;
using namespace std;

void NJSBitcoinLikeOutputListCallback::onCallback(const std::experimental::optional<std::vector<std::shared_ptr<BitcoinLikeOutput>>> & result, const std::experimental::optional<Error> & error)
{
    Nan::HandleScope scope;
    //Wrap parameters
    Local<Value> arg_0;
    if(result)
    {
        auto arg_0_optional = (result).value();
        Local<Array> arg_0_tmp = Nan::New<Array>();
        for(size_t arg_0_tmp_id = 0; arg_0_tmp_id < arg_0_optional.size(); arg_0_tmp_id++)
        {
            auto arg_0_tmp_elem_wrap = NJSBitcoinLikeOutput::wrap(arg_0_optional[arg_0_tmp_id]);
            auto arg_0_tmp_elem = Nan::ObjectWrap::Unwrap<NJSBitcoinLikeOutput>(arg_0_tmp_elem_wrap)->handle();

            arg_0_tmp->Set((int)arg_0_tmp_id,arg_0_tmp_elem);
        }

        arg_0 = arg_0_tmp;
    }

    Local<Value> arg_1;
    if(error)
    {
        auto arg_1_optional = (error).value();
        auto arg_1_tmp = Nan::New<Object>();
        auto arg_1_tmp_1 = Nan::New<Integer>((int)arg_1_optional.code);
        Nan::DefineOwnProperty(arg_1_tmp, Nan::New<String>("code").ToLocalChecked(), arg_1_tmp_1);
        auto arg_1_tmp_2 = Nan::New<String>(arg_1_optional.message).ToLocalChecked();
        Nan::DefineOwnProperty(arg_1_tmp, Nan::New<String>("message").ToLocalChecked(), arg_1_tmp_2);

        arg_1 = arg_1_tmp;
    }

    auto local_resolver = Nan::New<Promise::Resolver>(pers_resolver);
    if(error)
    {
        auto rejected = local_resolver->Reject(Nan::GetCurrentContext(), arg_1);
        rejected.FromJust();
    }
    else
    {
        auto resolve = local_resolver->Resolve(Nan::GetCurrentContext(), arg_0);
        resolve.FromJust();
    }
}

NAN_METHOD(NJSBitcoinLikeOutputListCallback::addRef) {

    NJSBitcoinLikeOutputListCallback *obj = Nan::ObjectWrap::Unwrap<NJSBitcoinLikeOutputListCallback>(info.This());
    obj->Ref();
}

NAN_METHOD(NJSBitcoinLikeOutputListCallback::removeRef) {

    NJSBitcoinLikeOutputListCallback *obj = Nan::ObjectWrap::Unwrap<NJSBitcoinLikeOutputListCallback>(info.This());
    obj->Unref();
}

NAN_METHOD(NJSBitcoinLikeOutputListCallback::New) {
    //Only new allowed
    if(!info.IsConstructCall())
    {
        return Nan::ThrowError("NJSBitcoinLikeOutputListCallback function can only be called as constructor (use New)");
    }

    auto resolver = v8::Promise::Resolver::New(Nan::GetCurrentContext()).ToLocalChecked();
    NJSBitcoinLikeOutputListCallback *node_instance = new NJSBitcoinLikeOutputListCallback(resolver);

    if(node_instance)
    {
        //Wrap and return node instance
        node_instance->Wrap(info.This());
        node_instance->Ref();
        info.GetReturnValue().Set(info.This());
    }
}

void NJSBitcoinLikeOutputListCallback::Initialize(Local<Object> target) {
    Nan::HandleScope scope;

    Local<FunctionTemplate> func_template = Nan::New<FunctionTemplate>(NJSBitcoinLikeOutputListCallback::New);
    Local<ObjectTemplate> objectTemplate = func_template->InstanceTemplate();
    objectTemplate->SetInternalFieldCount(1);

    func_template->SetClassName(Nan::New<String>("NJSBitcoinLikeOutputListCallback").ToLocalChecked());
    Nan::SetPrototypeMethod(func_template,"addRef", addRef);
    Nan::SetPrototypeMethod(func_template,"removeRef", removeRef);

    //Add template to target
    target->Set(Nan::New<String>("NJSBitcoinLikeOutputListCallback").ToLocalChecked(), func_template->GetFunction());
}