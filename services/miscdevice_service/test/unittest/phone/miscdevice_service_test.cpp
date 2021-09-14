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

#include <fcntl.h>
#include <gtest/gtest.h>
#include <thread>
#include <unistd.h>

#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "miscdevice_service_proxy.h"
#include "sensors_errors.h"
#include "sensors_log_domain.h"

#include "vibrator_if.h"
#include "vibrator_type.h"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, SensorsLogDomain::COMMON, "MiscdeviceServiceTest" };
const std::string CMD_LINE = "ps -ef | grep 'hardware.light' | grep -v grep | awk '{print $2}'";
constexpr int32_t BUFFER_SIZE = 8;
constexpr pid_t INVALID_PID = -1;
}  // namespace

class MiscdeviceServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    pid_t GetSensorServicePid();
    sptr<IMiscdeviceService> miscDeviceProxy_;
};

void MiscdeviceServiceTest::SetUp()
{
    auto sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(sm, nullptr);
    miscDeviceProxy_ = iface_cast<IMiscdeviceService>(sm->GetSystemAbility(MISCDEVICE_SERVICE_ABILITY_ID));
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

void MiscdeviceServiceTest::TearDown()
{
    miscDeviceProxy_ = nullptr;
}

pid_t MiscdeviceServiceTest::GetSensorServicePid()
{
    pid_t pid = INVALID_PID;
    char buf[BUFFER_SIZE] = { 0 };
    FILE *fp = popen(CMD_LINE.c_str(), "r");
    if (fp == nullptr) {
        HiLog::Error(LABEL, "get error when getting sensor service process id");
        return pid;
    }

    fgets(buf, sizeof(buf) - 1, fp);
    pclose(fp);
    fp = nullptr;
    HiLog::Info(LABEL, "process is : %{public}s", buf);

    std::string pidStr(buf);
    pidStr = TrimStr(pidStr, '\n');
    HiLog::Info(LABEL, "pidStr is : %{public}s", pidStr.c_str());
    if (pidStr.empty()) {
        return pid;
    }

    if (IsNumericStr(pidStr)) {
        pid = std::stoi(pidStr);
    }
    return pid;
}

/*
 * @tc.name: Light_ON_001
 * @tc.desc: light on
 * @tc.type: FUNC
 */
HWTEST_F(MiscdeviceServiceTest, Light_ON_001, TestSize.Level1)
{
    ASSERT_NE(miscDeviceProxy_, nullptr);
    std::vector<int32_t> setTest = miscDeviceProxy_->GetLightSupportId();
    if (setTest.size() == 0) {
        return;
    }
    uint32_t corlorBrightness = 0x00ffffff;
    int32_t ret = miscDeviceProxy_->Light(LightId::LIGHT_ID_LED, corlorBrightness, 0, 0);
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: Light_OFF_001
 * @tc.desc: light off
 * @tc.type: FUNC
 */
HWTEST_F(MiscdeviceServiceTest, Light_OFF_001, TestSize.Level1)
{
    ASSERT_NE(miscDeviceProxy_, nullptr);
    std::vector<int32_t> setTest = miscDeviceProxy_->GetLightSupportId();
    if (setTest.size() == 0) {
        return;
    }
    uint32_t corlorBrightness = 0;
    int32_t ret = miscDeviceProxy_->Light(LightId::LIGHT_ID_LED, corlorBrightness, 0, 0);
    ASSERT_EQ(ret, NO_ERROR);
}
}  // namespace Sensors
}  // namespace OHOS
