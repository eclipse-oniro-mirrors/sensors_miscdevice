/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "playpackagebysessionid_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

#include "securec.h"
#include "vibrator_agent.h"

namespace OHOS {
template<class T>
size_t GetObject(const uint8_t *data, size_t size, T &object)
{
    size_t objectSize = sizeof(object);
    if (objectSize > size) {
        return 0;
    }
    errno_t ret = memcpy_s(&object, objectSize, data, objectSize);
    if (ret != EOK) {
        return 0;
    }
    return objectSize;
}

bool PlayPackageBySessionIdTest(const uint8_t *data, size_t size)
{
    uint32_t sessionId = 0;
    VibratorPackage package{ 0 };
    size_t startPos = 0;
    startPos += GetObject<uint32_t>(data, size, sessionId);
    GetObject<int32_t>(data + startPos, size - startPos, package.patternNum);
    int32_t ret = OHOS::Sensors::PlayPackageBySessionId(sessionId, package);
    if (ret == 0) {
        return false;
    }
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::PlayPackageBySessionIdTest(data, size);
    return 0;
}