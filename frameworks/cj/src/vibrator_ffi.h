/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_VIBRATOR_FFI_H
#define OHOS_VIBRATOR_FFI_H

#include <cstdint>

#include "cj_common_ffi.h"
#include "ffi_remote_data.h"

extern "C" {
    struct RetVibrateTime {
        char* timeType;
        int32_t duration;
    };

    struct RetVibratePreset {
        char* presetType;
        char* effectId;
        int32_t count;
        int32_t intensity;
    };

    struct RetHapticFileDescriptor {
        int32_t fd;
        int64_t offSet;
        int64_t length;
    };

    struct RetVibrateFromFile {
        char* fileType;
        RetHapticFileDescriptor hapticFd;
    };

    struct RetVibrateAttribute {
        int32_t id;
        char* usage;
    };

    FFI_EXPORT void FfiVibratorStartVibration(
        RetVibrateTime effect, RetVibrateAttribute attribute, int32_t &code);
    FFI_EXPORT void FfiVibratorStartVibrationPreset(
        RetVibratePreset effect, RetVibrateAttribute attribute, int32_t &code);
    FFI_EXPORT void FfiVibratorStartVibrationFile(
        RetVibrateFromFile effect, RetVibrateAttribute attribute, int32_t &code);
    FFI_EXPORT void FfiVibratorStopVibration(int32_t &code);
    FFI_EXPORT void FfiVibratorStopVibrationMode(
        char* vibMode, int32_t &code);
    FFI_EXPORT bool FfiVibratorSupportEffect(
        char* id, int32_t &code);
    FFI_EXPORT bool FfiVibratorIsHdHapticSupported();
}

#endif // OHOS_VIBRATOR_FFI_H