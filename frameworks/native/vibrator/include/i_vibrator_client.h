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

#ifndef I_VIBRATOR_CLIENT_H
#define I_VIBRATOR_CLIENT_H

#include "iremote_broker.h"

namespace OHOS {
namespace Sensors {
class IVibratorClient : public IRemoteBroker {
public:
    enum VibratorClientInterfaceId {
        TRANS_ID_PLUG_ABILITY = 0,
    };
    IVibratorClient() = default;
    virtual ~IVibratorClient() = default;
    virtual int ProcessPlugEvent(int32_t eventCode, int32_t deviceId, int32_t vibratorCnt) = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"IVibratorClient");
};
} // namespace Sensors
} // namespace OHOS
#endif // I_VIBRATOR_CLIENT_H
