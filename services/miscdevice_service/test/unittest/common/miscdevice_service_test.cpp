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

#include "miscdevice_service.h"
#include "miscdevice_service_impl.h"
#include "singleton.h"
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
    MiscdeviceServiceImpl &vibratorServiceImpl_ = MiscdeviceServiceImpl::GetInstance();
    int32_t vibratorId_ = 123;
};

void MiscdeviceServiceTest::SetUp()
{
    auto sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(sm, nullptr);
    int32_t ret = vibratorServiceImpl_.InitVibratorServiceImpl();
    ASSERT_EQ(ret, ERR_OK);
}

void MiscdeviceServiceTest::TearDown()
{}

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
 * @tc.name: StopVibrator_001
 * @tc.desc: Test StopVibrator API functionality.
 * @tc.type: FUNC
 * @tc.require: 
 * @tc.author: wangcaiyi
 */
HWTEST_F(MiscdeviceServiceTest, StopVibrator_001, TestSize.Level1)
{
    uint32_t time = 5695;
    int32_t ret = vibratorServiceImpl_.StartOnceVibrator(time);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, NO_ERROR);
    ret = vibratorServiceImpl_.StopVibratorImpl(VIBRATOR_MODE_ONCE);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: StartVibrator_001
 * @tc.desc: Test StartVibrator API functionality.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangcaiyi
 */
HWTEST_F(MiscdeviceServiceTest, StartVibrator_001, TestSize.Level1)
{
    const char *builtIn = "vibrator.haptic.default.effect";
    int32_t ret = vibratorServiceImpl_.StartVibrator(builtIn);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, NO_ERROR);
    ret = vibratorServiceImpl_.StopVibratorImpl(VIBRATOR_MODE_PRESET);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, NO_ERROR);
}
}  // namespace Sensors
}  // namespace OHOS
