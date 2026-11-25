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

#include "seektimeonpackage_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fuzzer/FuzzedDataProvider.h>

#include "vibrator_service_client.h"
#include "sensors_errors.h"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t MAX_PATTERN_NUM_TEST = 4;
constexpr int32_t MAX_EVENT_NUM_TEST = 17;
constexpr int32_t MAX_POINT_NUM_TEST = 16;
constexpr int32_t EVENT_TYPE_MIN_TEST = -2;
constexpr int32_t EVENT_TYPE_MAX_TEST = 3;

void FreeSeekPackage(VibratorPackage &package)
{
    if (package.patternNum > 0) {
        VibratorServiceClient::FreeVibratorPackage(package);
    } else if (package.patterns != nullptr) {
        free(package.patterns);
        package.patterns = nullptr;
    }
}
} // namespace

bool SeekTimeOnPackageFuzzTest(FuzzedDataProvider &fdp, int32_t seekTime)
{
    // Fuzz the complete package so ConvertSeekVibratorPackage iterates real
    // patterns/events and reaches SkipEventAndConvertVibratorEvent branches:
    // pattern before/after seekTime, continuous/transient events, events long
    // enough or too short to survive seeking, curve point counts in and out
    // of [CURVE_POINT_NUM_MIN, CURVE_POINT_NUM_MAX].
    VibratorCurvePoint pointPool[MAX_PATTERN_NUM_TEST * MAX_EVENT_NUM_TEST * MAX_POINT_NUM_TEST] = {};
    VibratorEvent eventPool[MAX_PATTERN_NUM_TEST][MAX_EVENT_NUM_TEST] = {};
    VibratorPattern patternPool[MAX_PATTERN_NUM_TEST] = {};
    for (int32_t p = 0; p < MAX_PATTERN_NUM_TEST; ++p) {
        patternPool[p].events = eventPool[p];
        for (int32_t e = 0; e < MAX_EVENT_NUM_TEST; ++e) {
            eventPool[p][e].points =
                &pointPool[(p * MAX_EVENT_NUM_TEST + e) * MAX_POINT_NUM_TEST];
        }
    }
    VibratorPackage completePackage;
    completePackage.patternNum = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_PATTERN_NUM_TEST);
    completePackage.packageDuration = fdp.ConsumeIntegral<int32_t>();
    completePackage.patterns = patternPool;
    for (int32_t p = 0; p < completePackage.patternNum; ++p) {
        patternPool[p].time = fdp.ConsumeIntegral<int32_t>();
        patternPool[p].patternDuration = fdp.ConsumeIntegral<int32_t>();
        patternPool[p].eventNum = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_EVENT_NUM_TEST);
        for (int32_t e = 0; e < patternPool[p].eventNum; ++e) {
            VibratorEvent &fuzzEvent = eventPool[p][e];
            fuzzEvent.type = static_cast<VibratorEventType>(
                fdp.ConsumeIntegralInRange<int8_t>(EVENT_TYPE_MIN_TEST, EVENT_TYPE_MAX_TEST));
            fuzzEvent.time = fdp.ConsumeIntegral<int32_t>();
            fuzzEvent.duration = fdp.ConsumeIntegral<int32_t>();
            fuzzEvent.intensity = fdp.ConsumeIntegral<int32_t>();
            fuzzEvent.frequency = fdp.ConsumeIntegral<int32_t>();
            fuzzEvent.pointNum = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_POINT_NUM_TEST);
            for (int32_t k = 0; k < fuzzEvent.pointNum; ++k) {
                VibratorCurvePoint &fuzzPoint = fuzzEvent.points[k];
                fuzzPoint.time = fdp.ConsumeIntegral<int32_t>();
                fuzzPoint.intensity = fdp.ConsumeIntegral<int32_t>();
                fuzzPoint.frequency = fdp.ConsumeIntegral<int32_t>();
            }
        }
    }

    // Call the internal conversion chain through the singleton so ConvertSeekVibratorPackage,
    // SkipEventAndConvertVibratorEvent, ConvertVibratorPattern and ConvertVibratorPackage
    // in vibrator_service_client.cpp are compiled and counted inside this fuzz binary
    // (same approach as the merged handlevibratordata_fuzzer: internal entries only, no
    // cross-DSO C API calls that would depend on the system image's shared libraries).
    VibratorPackage seekPackage;
    int32_t ret = VibratorServiceClient::GetInstance().SeekTimeOnPackage(seekTime, completePackage,
        seekPackage);
    if (ret == ERR_OK) {
        FreeSeekPackage(seekPackage);
    }
    return true;
}
} // namespace Sensors
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    int32_t seekTime = fdp.ConsumeIntegral<int32_t>();
    OHOS::Sensors::SeekTimeOnPackageFuzzTest(fdp, seekTime);
    return 0;
}
