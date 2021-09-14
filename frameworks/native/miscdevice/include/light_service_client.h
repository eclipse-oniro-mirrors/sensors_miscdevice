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

#ifndef LIGHT_SERVICE_CLIENT_H
#define LIGHT_SERVICE_CLIENT_H

#include <mutex>
#include "iremote_object.h"
#include "miscdevice_service_proxy.h"
#include "singleton.h"

namespace OHOS {
namespace Sensors {
class LightServiceClient : public Singleton<LightServiceClient> {
public:
    std::vector<int32_t> GetLightIdList();
    bool IsLightEffectSupport(int32_t lightId, const std::string &effectId);
    int32_t Light(int32_t lightId, uint64_t brightness, uint32_t timeOn, uint32_t timeOff);
    int32_t PlayLightEffect(int32_t lightId, const std::string &type);
    int32_t StopLightEffect(int32_t lightId);
    void ProcessDeathObserver(const wptr<IRemoteObject> &object);

private:
    int32_t InitServiceClient();
    sptr<IRemoteObject::DeathRecipient> serviceDeathObserver_;
    sptr<IMiscdeviceService> miscdeviceProxy_;
    std::mutex clientMutex_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // LIGHT_SERVICE_CLIENT_H
