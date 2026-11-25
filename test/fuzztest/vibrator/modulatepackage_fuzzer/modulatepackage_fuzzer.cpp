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

#include "modulatepackage_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "vibrator_service_client.h"
#include "sensors_errors.h"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t MAX_PATTERN_NUM_TEST = 4;
constexpr int32_t MAX_EVENT_NUM_TEST = 4;
constexpr int32_t MAX_POINT_NUM_TEST = 16;
constexpr size_t POINT_POOL_SIZE = MAX_PATTERN_NUM_TEST * MAX_EVENT_NUM_TEST * MAX_POINT_NUM_TEST;
constexpr int32_t EVENT_TYPE_MIN_TEST = -2;
constexpr int32_t EVENT_TYPE_MAX_TEST = 3;

void FillCurvePoints(FuzzedDataProvider &fdp, VibratorCurvePoint *points, int32_t pointNum)
{
    for (int32_t i = 0; i < pointNum; ++i) {
        points[i].time = fdp.ConsumeIntegral<int32_t>();
        points[i].intensity = fdp.ConsumeIntegral<int32_t>();
        points[i].frequency = fdp.ConsumeIntegral<int32_t>();
    }
}

void FillModulationCurve(FuzzedDataProvider &fdp, VibratorEvent &modulationCurve, int32_t modulationTime,
    VibratorCurvePoint (&modPoints)[MAX_POINT_NUM_TEST])
{
    modulationCurve.type = EVENT_TYPE_CONTINUOUS;
    modulationCurve.time = modulationTime;
    modulationCurve.duration = fdp.ConsumeIntegral<int32_t>();
    modulationCurve.intensity = fdp.ConsumeIntegral<int32_t>();
    modulationCurve.frequency = fdp.ConsumeIntegral<int32_t>();
    modulationCurve.pointNum = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_POINT_NUM_TEST);
    modulationCurve.points = modPoints;
    FillCurvePoints(fdp, modPoints, modulationCurve.pointNum);
}

void FillEventFields(FuzzedDataProvider &fdp, VibratorEvent &fuzzEvent)
{
    fuzzEvent.type = static_cast<VibratorEventType>(
        fdp.ConsumeIntegralInRange<int8_t>(EVENT_TYPE_MIN_TEST, EVENT_TYPE_MAX_TEST));
    fuzzEvent.time = fdp.ConsumeIntegral<int32_t>();
    fuzzEvent.duration = fdp.ConsumeIntegral<int32_t>();
    fuzzEvent.intensity = fdp.ConsumeIntegral<int32_t>();
    fuzzEvent.frequency = fdp.ConsumeIntegral<int32_t>();
    fuzzEvent.pointNum = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_POINT_NUM_TEST);
    FillCurvePoints(fdp, fuzzEvent.points, fuzzEvent.pointNum);
}

void FillPackageBeforeModulation(FuzzedDataProvider &fdp, VibratorPackage &package,
    VibratorCurvePoint (&pointPool)[POINT_POOL_SIZE],
    VibratorEvent (&eventPool)[MAX_PATTERN_NUM_TEST][MAX_EVENT_NUM_TEST],
    VibratorPattern (&patternPool)[MAX_PATTERN_NUM_TEST])
{
    for (int32_t p = 0; p < MAX_PATTERN_NUM_TEST; ++p) {
        patternPool[p].events = eventPool[p];
        for (int32_t e = 0; e < MAX_EVENT_NUM_TEST; ++e) {
            eventPool[p][e].points = &pointPool[(p * MAX_EVENT_NUM_TEST + e) * MAX_POINT_NUM_TEST];
        }
    }
    package.patterns = patternPool;
    package.patternNum = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_PATTERN_NUM_TEST);
    package.packageDuration = fdp.ConsumeIntegral<int32_t>();
    for (int32_t p = 0; p < package.patternNum; ++p) {
        patternPool[p].time = fdp.ConsumeIntegral<int32_t>();
        patternPool[p].patternDuration = fdp.ConsumeIntegral<int32_t>();
        patternPool[p].eventNum = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_EVENT_NUM_TEST);
        for (int32_t e = 0; e < patternPool[p].eventNum; ++e) {
            FillEventFields(fdp, eventPool[p][e]);
        }
    }
}
} // namespace

bool ModulatePackageFuzzTest(FuzzedDataProvider &fdp, int32_t modulationTime)
{
    // Fuzz the modulation curve event, its pointNum and points decide the curve intervals.
    VibratorCurvePoint modPoints[MAX_POINT_NUM_TEST] = {};
    VibratorEvent modulationCurve;
    FillModulationCurve(fdp, modulationCurve, modulationTime, modPoints);

    // Fuzz the package before modulation: patterns -> events -> curve points.
    VibratorCurvePoint pointPool[POINT_POOL_SIZE] = {};
    VibratorEvent eventPool[MAX_PATTERN_NUM_TEST][MAX_EVENT_NUM_TEST] = {};
    VibratorPattern patternPool[MAX_PATTERN_NUM_TEST] = {};
    VibratorPackage beforeModulationPackage;
    FillPackageBeforeModulation(fdp, beforeModulationPackage, pointPool, eventPool, patternPool);

    // Call the internal pure logic directly so the modulation chain in vibrator_service_client.cpp
    // is compiled and counted inside this fuzz binary: ModulatePackage -> ModulateVibratorPattern
    // -> ModulateVibratorEvent -> ModulateEvent(With|Without)CurvePoints -> GetCurveListAfterModulation
    // -> BinarySearchInterval / ConvertVibratorEventsToCurveIntervals / Restrict*Range, and the
    // FreePartiallyAllocated* chains on error paths.
    VibratorPackage afterModulationPackage;
    int32_t ret = VibratorServiceClient::ModulatePackage(modulationCurve, beforeModulationPackage,
        afterModulationPackage);
    if (ret == ERR_OK) {
        VibratorServiceClient::FreeVibratorPackage(afterModulationPackage);
    }
    return true;
}
} // namespace Sensors
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    int32_t modulationTime = fdp.ConsumeIntegral<int32_t>();
    OHOS::Sensors::ModulatePackageFuzzTest(fdp, modulationTime);
    return 0;
}
