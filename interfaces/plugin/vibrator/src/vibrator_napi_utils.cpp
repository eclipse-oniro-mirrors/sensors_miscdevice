/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vibrator_napi_utils.h"
#include <string>
#include "hilog/log.h"


using namespace OHOS::HiviewDFX;
static constexpr HiLogLabel LABEL = {LOG_CORE, 0xD002708, "VibratorJsAPI"};

bool IsMatchType(napi_value value, napi_valuetype type, napi_env env)
{
    napi_valuetype paramType;
    napi_typeof(env, value, &paramType);
    if (paramType != type) {
        HiLog::Error(LABEL, "%{public}s  failed!", __func__);
        return false;
    }
    return true;
}

napi_value GetNapiInt32(int32_t number, napi_env env)
{
    napi_value value;
    napi_create_int32(env, number, &value);
    return value;
}

napi_value NapiGetNamedProperty(napi_value jsonObject, string name, napi_env env)
{
    napi_value value;
    napi_get_named_property(env, jsonObject, name.c_str(), &value);
    return value;
}

int32_t GetCppInt32(napi_value value, napi_env env)
{
    int32_t number;
    napi_get_value_int32(env, value, &number);
    return number;
}

string GetCppString(napi_value value, napi_env env)
{
    size_t bufLength = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &bufLength);
    char *buff = (char *)malloc((bufLength + 1) * sizeof(char));
    if (buff == nullptr) {
        HiLog::Error(LABEL, "js obj to str malloc failed");
    }
    status = napi_get_value_string_utf8(env, value, buff, bufLength + 1, &bufLength);
    return buff;
}

int64_t GetCppInt64(napi_value value, napi_env env)
{
    int64_t number;
    napi_get_value_int64(env, value, &number);
    return number;
}

napi_value GreateBusinessError(napi_env env, int32_t errCode, string errMessage, string errName, string errStack)
{
    napi_value result = nullptr;
    napi_value code = nullptr;
    napi_value message = nullptr;
    napi_value name = nullptr;
    napi_value stack = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &code));
    NAPI_CALL(env, napi_create_string_utf8(env, errMessage.data(), NAPI_AUTO_LENGTH, &message));
    NAPI_CALL(env, napi_create_string_utf8(env, errName.data(), NAPI_AUTO_LENGTH, &name));
    NAPI_CALL(env, napi_create_string_utf8(env, errStack.data(), NAPI_AUTO_LENGTH, &stack));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", code));
    NAPI_CALL(env, napi_set_named_property(env, result, "message", message));
    NAPI_CALL(env, napi_set_named_property(env, result, "name", name));
    NAPI_CALL(env, napi_set_named_property(env, result, "stack", stack));
    return result;
}

void EmitAsyncCallbackWork(AsyncCallbackInfo *asyncCallbackInfo)
{
    HiLog::Debug(LABEL, "%s begin", __func__);
    if (asyncCallbackInfo == nullptr) {
        HiLog::Error(LABEL, "%s asyncCallbackInfo is nullptr!", __func__);
        return;
    }
    napi_value resourceName;
    napi_create_string_latin1(asyncCallbackInfo->env, "AsyncCallback", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        asyncCallbackInfo->env, nullptr, resourceName, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {      
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value callback;
            napi_get_reference_value(env, asyncCallbackInfo->callback[0], &callback);
            napi_value result = nullptr;
            napi_value callResult = nullptr;
            if (asyncCallbackInfo->error.code < 0) {
                result = GreateBusinessError(env, asyncCallbackInfo->error.code, asyncCallbackInfo->error.message,
                    asyncCallbackInfo->error.name, asyncCallbackInfo->error.stack);
            } else {
                napi_get_undefined(env, &result);
            }
            napi_call_function(env, nullptr, callback, 1, &result, &callResult);
            napi_delete_reference(env, asyncCallbackInfo->callback[0]);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        },
        asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
    HiLog::Debug(LABEL, "%{public}s  end", __func__);
}

void EmitPromiseWork(AsyncCallbackInfo *asyncCallbackInfo)
{
    if (asyncCallbackInfo == nullptr) {
        HiLog::Error(LABEL, "%s asyncCallbackInfo is nullptr!", __func__);
        return;
    }
    napi_value resourceName;
    napi_create_string_latin1(asyncCallbackInfo->env, "Promise", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        asyncCallbackInfo->env, nullptr, resourceName, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result = nullptr;
            if (asyncCallbackInfo->error.code == 0) {
                napi_get_undefined(env, &result);
                napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            } else {
                result = GreateBusinessError(env, asyncCallbackInfo->error.code, asyncCallbackInfo->error.message,
                    asyncCallbackInfo->error.name, asyncCallbackInfo->error.stack);
                napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        },
        (void*)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
}