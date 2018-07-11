// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from thread_dispatcher.djinni

#include "NJSThreadDispatcher.hpp"

using namespace v8;
using namespace node;
using namespace std;

std::shared_ptr<ExecutionContext> NJSThreadDispatcher::getSerialExecutionContext(const std::string & name)
{
    Nan::HandleScope scope;
    //Wrap parameters
    auto arg_0 = Nan::New<String>(name).ToLocalChecked();
    Handle<Value> args[1] = {arg_0};
    Local<Object> local_njs_impl = Nan::New<Object>(njs_impl);
    if(!local_njs_impl->IsObject())
    {
        Nan::ThrowError("NJSThreadDispatcher::getSerialExecutionContext fail to retrieve node implementation");
    }
    auto calling_funtion = Nan::Get(local_njs_impl,Nan::New<String>("getSerialExecutionContext").ToLocalChecked()).ToLocalChecked();
    auto handle = this->handle();
    auto result_getSerialExecutionContext = Nan::CallAsFunction(calling_funtion->ToObject(),handle,1,args);
    if(result_getSerialExecutionContext.IsEmpty())
    {
        Nan::ThrowError("NJSThreadDispatcher::getSerialExecutionContext call failed");
    }
    auto checkedResult_getSerialExecutionContext = result_getSerialExecutionContext.ToLocalChecked();
    Local<Object> njs_fResult_getSerialExecutionContext = checkedResult_getSerialExecutionContext->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
    NJSExecutionContext *njs_ptr_fResult_getSerialExecutionContext = static_cast<NJSExecutionContext *>(Nan::GetInternalFieldPointer(njs_fResult_getSerialExecutionContext,0));
    std::shared_ptr<NJSExecutionContext> fResult_getSerialExecutionContext(njs_ptr_fResult_getSerialExecutionContext);

    return fResult_getSerialExecutionContext;
}

std::shared_ptr<ExecutionContext> NJSThreadDispatcher::getThreadPoolExecutionContext(const std::string & name)
{
    Nan::HandleScope scope;
    //Wrap parameters
    auto arg_0 = Nan::New<String>(name).ToLocalChecked();
    Handle<Value> args[1] = {arg_0};
    Local<Object> local_njs_impl = Nan::New<Object>(njs_impl);
    if(!local_njs_impl->IsObject())
    {
        Nan::ThrowError("NJSThreadDispatcher::getThreadPoolExecutionContext fail to retrieve node implementation");
    }
    auto calling_funtion = Nan::Get(local_njs_impl,Nan::New<String>("getThreadPoolExecutionContext").ToLocalChecked()).ToLocalChecked();
    auto handle = this->handle();
    auto result_getThreadPoolExecutionContext = Nan::CallAsFunction(calling_funtion->ToObject(),handle,1,args);
    if(result_getThreadPoolExecutionContext.IsEmpty())
    {
        Nan::ThrowError("NJSThreadDispatcher::getThreadPoolExecutionContext call failed");
    }
    auto checkedResult_getThreadPoolExecutionContext = result_getThreadPoolExecutionContext.ToLocalChecked();
    Local<Object> njs_fResult_getThreadPoolExecutionContext = checkedResult_getThreadPoolExecutionContext->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
    NJSExecutionContext *njs_ptr_fResult_getThreadPoolExecutionContext = static_cast<NJSExecutionContext *>(Nan::GetInternalFieldPointer(njs_fResult_getThreadPoolExecutionContext,0));
    std::shared_ptr<NJSExecutionContext> fResult_getThreadPoolExecutionContext(njs_ptr_fResult_getThreadPoolExecutionContext);

    return fResult_getThreadPoolExecutionContext;
}

std::shared_ptr<ExecutionContext> NJSThreadDispatcher::getMainExecutionContext()
{
    Nan::HandleScope scope;
    //Wrap parameters
    Handle<Value> args[1];
    Local<Object> local_njs_impl = Nan::New<Object>(njs_impl);
    if(!local_njs_impl->IsObject())
    {
        Nan::ThrowError("NJSThreadDispatcher::getMainExecutionContext fail to retrieve node implementation");
    }
    auto calling_funtion = Nan::Get(local_njs_impl,Nan::New<String>("getMainExecutionContext").ToLocalChecked()).ToLocalChecked();
    auto handle = this->handle();
    auto result_getMainExecutionContext = Nan::CallAsFunction(calling_funtion->ToObject(),handle,0,args);
    if(result_getMainExecutionContext.IsEmpty())
    {
        Nan::ThrowError("NJSThreadDispatcher::getMainExecutionContext call failed");
    }
    auto checkedResult_getMainExecutionContext = result_getMainExecutionContext.ToLocalChecked();
    Local<Object> njs_fResult_getMainExecutionContext = checkedResult_getMainExecutionContext->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
    NJSExecutionContext *njs_ptr_fResult_getMainExecutionContext = static_cast<NJSExecutionContext *>(Nan::GetInternalFieldPointer(njs_fResult_getMainExecutionContext,0));
    std::shared_ptr<NJSExecutionContext> fResult_getMainExecutionContext(njs_ptr_fResult_getMainExecutionContext);

    return fResult_getMainExecutionContext;
}

std::shared_ptr<Lock> NJSThreadDispatcher::newLock()
{
    Nan::HandleScope scope;
    //Wrap parameters
    Handle<Value> args[1];
    Local<Object> local_njs_impl = Nan::New<Object>(njs_impl);
    if(!local_njs_impl->IsObject())
    {
        Nan::ThrowError("NJSThreadDispatcher::newLock fail to retrieve node implementation");
    }
    auto calling_funtion = Nan::Get(local_njs_impl,Nan::New<String>("newLock").ToLocalChecked()).ToLocalChecked();
    auto handle = this->handle();
    auto result_newLock = Nan::CallAsFunction(calling_funtion->ToObject(),handle,0,args);
    if(result_newLock.IsEmpty())
    {
        Nan::ThrowError("NJSThreadDispatcher::newLock call failed");
    }
    auto checkedResult_newLock = result_newLock.ToLocalChecked();
    Local<Object> njs_fResult_newLock = checkedResult_newLock->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
    NJSLock *njs_ptr_fResult_newLock = static_cast<NJSLock *>(Nan::GetInternalFieldPointer(njs_fResult_newLock,0));
    std::shared_ptr<NJSLock> fResult_newLock(njs_ptr_fResult_newLock);

    return fResult_newLock;
}

NAN_METHOD(NJSThreadDispatcher::addRef) {

    NJSThreadDispatcher *obj = Nan::ObjectWrap::Unwrap<NJSThreadDispatcher>(info.This());
    obj->Ref();
}

NAN_METHOD(NJSThreadDispatcher::removeRef) {

    NJSThreadDispatcher *obj = Nan::ObjectWrap::Unwrap<NJSThreadDispatcher>(info.This());
    obj->Unref();
}

NAN_METHOD(NJSThreadDispatcher::New) {
    //Only new allowed
    if(!info.IsConstructCall())
    {
        return Nan::ThrowError("NJSThreadDispatcher function can only be called as constructor (use New)");
    }

    NJSThreadDispatcher *node_instance = nullptr;
    if(info[0]->IsObject())
    {
        node_instance = new NJSThreadDispatcher(info[0]->ToObject());
    }
    else
    {
        return Nan::ThrowError("NJSThreadDispatcher::New requires an implementation from node");
    }

    if(node_instance)
    {
        //Wrap and return node instance
        node_instance->Wrap(info.This());
        node_instance->Ref();
        info.GetReturnValue().Set(info.This());
    }
}

void NJSThreadDispatcher::Initialize(Local<Object> target) {
    Nan::HandleScope scope;

    Local<FunctionTemplate> func_template = Nan::New<FunctionTemplate>(NJSThreadDispatcher::New);
    Local<ObjectTemplate> objectTemplate = func_template->InstanceTemplate();
    objectTemplate->SetInternalFieldCount(1);

    func_template->SetClassName(Nan::New<String>("NJSThreadDispatcher").ToLocalChecked());
    Nan::SetPrototypeMethod(func_template,"addRef", addRef);
    Nan::SetPrototypeMethod(func_template,"removeRef", removeRef);

    //Add template to target
    target->Set(Nan::New<String>("NJSThreadDispatcher").ToLocalChecked(), func_template->GetFunction());
}
