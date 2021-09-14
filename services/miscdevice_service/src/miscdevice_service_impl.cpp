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

#include "miscdevice_service_impl.h"

#include <cstring>

#include "sensors_errors.h"
#include "sensors_log_domain.h"
#include "system_ability_definition.h"


namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, SensorsLogDomain::MISCDEVICE_SERVICE, "MiscdeviceServiceImpl" };
}

bool MiscdeviceServiceImpl::IsVibratorEffectAvailable(int32_t vibratorId, const std::string &effectType)
{
    return true;
}

ErrCode MiscdeviceServiceImpl::InitVibratorServiceImpl()
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    vibratorInterface = NewVibratorInterfaceInstance();
    if (vibratorInterface == nullptr) {
        HiLog::Error(LABEL, "%{public}s failed, vibratorInterface cannot be null", __func__);
        return ERR_INVALID_VALUE;
    }
    HiLog::Info(LABEL, "%{public}s end", __func__);
    return ERR_OK;
}

ErrCode MiscdeviceServiceImpl::StartVibrator(const char *effectType) const
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    if (effectType == nullptr) {
        HiLog::Error(LABEL, "%{public}s failed, effectType cannot be null", __func__);
        return ERR_INVALID_VALUE;
    }
    int32_t ret = vibratorInterface->Start(effectType);
    HiLog::Info(LABEL, "%{public}s end", __func__);
    return ret;
}

ErrCode MiscdeviceServiceImpl::StartOnceVibrator(uint32_t duration) const
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    if (duration < 0) {
        HiLog::Error(LABEL, "%{public}s duration is invalid", __func__);
        return ERR_INVALID_VALUE;
    }
    int32_t ret = vibratorInterface->StartOnce(duration);
    HiLog::Info(LABEL, "%{public}s end", __func__);
    return ret;
}

ErrCode MiscdeviceServiceImpl::StopVibratorImpl(enum VibratorMode mode) const
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    if (mode < VIBRATOR_MODE_ONCE) {
        HiLog::Error(LABEL, "%{public}s mode is invalid", __func__);
        return ERR_INVALID_VALUE;
    }
    int32_t ret = vibratorInterface->Stop(mode);
    HiLog::Info(LABEL, "%{public}s end", __func__);
    return ret;
}
}  // namespace Sensors
}  // namespace OHOS
