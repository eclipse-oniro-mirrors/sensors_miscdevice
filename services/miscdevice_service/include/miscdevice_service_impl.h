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

#ifndef MISCDEVICE_INTERFACE_H
#define MISCDEVICE_INTERFACE_H

#include "errors.h"
#include "refbase.h"
#include "singleton.h"
#include "vibrator_if.h"
#include "vibrator_type.h"
#include "nocopyable.h"

namespace OHOS {
namespace Sensors {
class MiscdeviceServiceImpl : public Singleton<MiscdeviceServiceImpl> {
public:
    MiscdeviceServiceImpl() = default;

    virtual ~MiscdeviceServiceImpl() = default;
    bool IsVibratorEffectAvailable(int32_t vibratorId, const std::string &effectType);
    ErrCode InitVibratorServiceImpl();
    ErrCode StartVibrator(const char *effectType) const;
    ErrCode StartOnceVibrator(uint32_t duration) const;
    ErrCode StopVibratorImpl(enum VibratorMode mode) const;

private:
    DISALLOW_COPY_AND_MOVE(MiscdeviceServiceImpl);

    const struct VibratorInterface *vibratorInterface = nullptr;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // SENSOR_INTERFACE_H
