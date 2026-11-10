/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef HDI_CONNECTION_MOCK_H
#define HDI_CONNECTION_MOCK_H

#include <gmock/gmock.h>

#include "i_light_hdi_connection.h"
#include "i_vibrator_hdi_connection.h"

namespace OHOS {
namespace Sensors {
class MockLightHdiConnection : public ILightHdiConnection {
public:
    MOCK_METHOD(int32_t, ConnectHdi, (), (override));
    MOCK_METHOD(int32_t, GetLightList, (std::vector<LightInfoIPC> &lightList), (override));
    MOCK_METHOD(int32_t, TurnOn,
        (int32_t lightId, const LightColor &color, const LightAnimationIPC &animation), (override));
    MOCK_METHOD(int32_t, TurnOff, (int32_t lightId), (override));
    MOCK_METHOD(int32_t, DestroyHdiConnection, (), (override));
};

class MockVibratorHdiConnection : public IVibratorHdiConnection {
public:
    MOCK_METHOD(int32_t, ConnectHdi, (), (override));
    MOCK_METHOD(int32_t, StartOnce, (const VibratorIdentifierIPC &identifier, uint32_t duration), (override));
    MOCK_METHOD(int32_t, Start,
        (const VibratorIdentifierIPC &identifier, const std::string &effectType), (override));
#ifdef HDF_DRIVERS_INTERFACE_VIBRATOR
#ifdef OHOS_BUILD_ENABLE_VIBRATOR_CUSTOM
    MOCK_METHOD(int32_t, EnableCompositeEffect,
        (const VibratorIdentifierIPC &identifier, const HdfCompositeEffect &hdfCompositeEffect), (override));
    MOCK_METHOD(bool, IsVibratorRunning, (const VibratorIdentifierIPC &identifier), (override));
#endif // OHOS_BUILD_ENABLE_VIBRATOR_CUSTOM
    MOCK_METHOD(std::optional<HdfEffectInfo>, GetEffectInfo,
        (const VibratorIdentifierIPC &identifier, const std::string &effect), (override));
    MOCK_METHOD(int32_t, Stop, (const VibratorIdentifierIPC &identifier, HdfVibratorMode mode), (override));
#endif // HDF_DRIVERS_INTERFACE_VIBRATOR
    MOCK_METHOD(int32_t, DestroyHdiConnection, (), (override));
    MOCK_METHOD(int32_t, GetDelayTime,
        (const VibratorIdentifierIPC &identifier, int32_t mode, int32_t &delayTime), (override));
    MOCK_METHOD(int32_t, GetVibratorCapacity,
        (const VibratorIdentifierIPC &identifier, VibratorCapacity &capacity), (override));
    MOCK_METHOD(int32_t, PlayPattern,
        (const VibratorIdentifierIPC &identifier, const VibratePattern &pattern), (override));
    MOCK_METHOD(int32_t, PlayPatternBySessionId,
        (const VibratorIdentifierIPC &identifier, uint32_t sessionId, const VibratePattern &pattern), (override));
    MOCK_METHOD(int32_t, StartByIntensity,
        (const VibratorIdentifierIPC &identifier, const std::string &effect, int32_t intensity), (override));
#ifdef HDF_DRIVERS_INTERFACE_VIBRATOR
    MOCK_METHOD(int32_t, GetVibratorInfo, (std::vector<HdfVibratorInfo> &hdfVibratorInfo), (override));
    MOCK_METHOD(int32_t, GetAllWaveInfo,
        (const VibratorIdentifierIPC &identifier, std::vector<HdfWaveInformation> &waveInfos), (override));
    MOCK_METHOD(int32_t, GetVibratorList,
        (const VibratorIdentifierIPC &identifier, std::vector<HdfVibratorInfo> &hdfVibratorInfo), (override));
    MOCK_METHOD(int32_t, GetEffectInfo,
        (const VibratorIdentifierIPC &identifier, const std::string &effectType, HdfEffectInfo &effectInfo),
        (override));
    MOCK_METHOD(int32_t, PlayPackageBySessionId,
        (const VibratorIdentifierIPC &identifier, uint32_t sessionId, const VibratePackage &package),
        (override));
    MOCK_METHOD(int32_t, StopVibrateBySessionId,
        (const VibratorIdentifierIPC &identifier, uint32_t sessionId), (override));
#endif // HDF_DRIVERS_INTERFACE_VIBRATOR
    MOCK_METHOD(int32_t, RegisterVibratorPlugCallback, (DevicePlugCallback cb), (override));
    MOCK_METHOD(DevicePlugCallback, GetVibratorPlugCb, (), (override));
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // HDI_CONNECTION_MOCK_H
