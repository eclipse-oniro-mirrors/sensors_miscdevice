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
#include <gtest/gtest.h>
#include <thread>

#include "ipc_skeleton.h"
#include "iservice_registry.h"

#include "sensors_errors.h"
#include "sensors_log_domain.h"
#include "vibrator_agent.h"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, SensorsLogDomain::MISCDEVICE_VIBRATOR_INTERFACE, "VibratorAgentNativeTest" };
}

class VibratorAgentNativeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void VibratorAgentNativeTest::SetUp()
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
}

void VibratorAgentNativeTest::TearDown()
{
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: PlayVibratorEffect_001
 * @tc.desc: perform vibrator short effect
 * @tc.type: FUNC
 * @tc.require: SR000F5A2Q AR000F8QO2
 * @tc.author: houpengfei
 */
HWTEST_F(VibratorAgentNativeTest, PlayVibratorEffect_001, TestSize.Level1)
{
    const char *supportString = "haptic.ringtone.Bounce";
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ASSERT_EQ(ret, ERROR);
}

/*
 * @tc.name: PlayVibratorEffect_002
 * @tc.desc: perform vibrator short effect
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangcaiyi
 */
HWTEST_F(VibratorAgentNativeTest, PlayVibratorEffect_002, TestSize.Level1)
{
    const char *supportString = "vibrator.haptic.default.time";
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: PlayVibratorEffect_003
 * @tc.desc: perform vibrator short effect
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangcaiyi
 */
HWTEST_F(VibratorAgentNativeTest, PlayVibratorEffect_003, TestSize.Level1)
{
    const char *supportString = "vibrator.haptic.default.effect";
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: PlayVibratorDuration_001
 * @tc.desc: perform vibrator duration
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: houpengfei
 */
HWTEST_F(VibratorAgentNativeTest, PlayVibratorDuration_001, TestSize.Level1)
{
    int32_t ret = StartVibratorOnce(1000);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: PlayVibratorDuration_002
 * @tc.desc: perform vibrator duration
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangcaiyi
 */
HWTEST_F(VibratorAgentNativeTest, PlayVibratorDuration_002, TestSize.Level1)
{
    int32_t ret = StartVibratorOnce(5000);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: PlayVibratorDuration_003
 * @tc.desc: perform vibrator duration
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangcaiyi
 */
HWTEST_F(VibratorAgentNativeTest, PlayVibratorDuration_003, TestSize.Level1)
{
    int32_t ret = StartVibratorOnce(50000);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: StopVibrator_001
 * @tc.desc: stop vibrator duration
 * @tc.type: FUNC
 * @tc.require: AR000F8QO2
 * @tc.author: guofuqiang
 */
HWTEST_F(VibratorAgentNativeTest, StopVibrator_001, TestSize.Level1)
{
    int32_t ret = StartVibratorOnce(5000);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ASSERT_EQ(ret, NO_ERROR);
    ret = StopVibrator(VIBRATOR_STOP_MODE_TIME);
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: StopVibrator_002
 * @tc.desc: stop vibrator preset effect
 * @tc.type: FUNC
 * @tc.require: AR000F8QO2
 * @tc.author: guofuqiang
 */
HWTEST_F(VibratorAgentNativeTest, StopVibrator_002, TestSize.Level1)
{
    const char *supportString = "haptic.ringtone.Bounce";
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ASSERT_EQ(ret, ERROR);
    ret = StopVibrator(VIBRATOR_STOP_MODE_PRESET);
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: StopVibrator_003
 * @tc.desc: perform vibrator duration
 * @tc.type: FUNC
 * @tc.require: AR000F8QO2
 * @tc.author: guofuqiang
 */
HWTEST_F(VibratorAgentNativeTest, StopVibrator_003, TestSize.Level1)
{
    int32_t ret = StartVibratorOnce(5000);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ASSERT_EQ(ret, NO_ERROR);
    ret = StopVibrator(VIBRATOR_STOP_MODE_PRESET);
    ASSERT_EQ(ret, ERROR);
}

/*
 * @tc.name: StopVibrator_004
 * @tc.desc: perform vibrator duration
 * @tc.type: FUNC
 * @tc.require: AR000F8QO2
 * @tc.author: guofuqiang
 */
HWTEST_F(VibratorAgentNativeTest, StopVibrator_004, TestSize.Level1)
{
    const char *supportString = "haptic.ringtone.Bounce";
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ASSERT_EQ(ret, ERROR);
    ret = StopVibrator(VIBRATOR_STOP_MODE_TIME);
    ASSERT_EQ(ret, ERROR);
}

/*
 * @tc.name: StopVibrator_005
 * @tc.desc: stop vibrator preset effect
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangcaiyi
 */
HWTEST_F(VibratorAgentNativeTest, StopVibrator_005, TestSize.Level1)
{
    const char *supportString = "vibrator.haptic.default.time";
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ASSERT_EQ(ret, NO_ERROR);
    ret = StopVibrator(VIBRATOR_STOP_MODE_PRESET);
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: EnableLooping_001
 * @tc.desc: enable vibrator with effect
 * @tc.type: FUNC
 * @tc.require: AR000FLCJ3
 * @tc.author: guofuqiang
 */
HWTEST_F(VibratorAgentNativeTest, EnableLooping_001, TestSize.Level1)
{
    const char *supportString = "haptic.ringtone.Dream";
    EnableLooping();
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    ASSERT_EQ(ret, NO_ERROR);
    ret = DisableLooping();
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: EnableLooping_002
 * @tc.desc: enable vibrator with effect
 * @tc.type: FUNC
 * @tc.require: AR000FLCJ3
 * @tc.author: guofuqiang
 */
HWTEST_F(VibratorAgentNativeTest, EnableLooping_002, TestSize.Level1)
{
    const char *supportString = "haptic.common.click";
    EnableLooping();
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, ERROR);
}

/*
 * @tc.name: EnableLooping_003
 * @tc.desc: enable vibrator with effect
 * @tc.type: FUNC
 * @tc.require: AR000FLCJ3
 * @tc.author: guofuqiang
 */
HWTEST_F(VibratorAgentNativeTest, EnableLooping_003, TestSize.Level1)
{
    const char *supportString = "haptic.ringtone.Cartoon";
    EnableLooping();
    int32_t ret = DisableLooping();
    ASSERT_EQ(ret, ERROR);
    ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, ERROR);
}

/*
 * @tc.name: EnableLooping_004
 * @tc.desc: enable vibrator with effect
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangcaiyi
 */
HWTEST_F(VibratorAgentNativeTest, EnableLooping_004, TestSize.Level1)
{
    const char *supportString = "vibrator.haptic.default.time";
    EnableLooping();
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    ASSERT_EQ(ret, ERROR);
    ret = DisableLooping();
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: EnableLooping_005
 * @tc.desc: enable vibrator with effect
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangcaiyi
 */
HWTEST_F(VibratorAgentNativeTest, EnableLooping_005, TestSize.Level1)
{
    const char *supportString = "vibrator.haptic.default.time";
    EnableLooping();
    int32_t ret = DisableLooping();
    ASSERT_EQ(ret, ERROR);
    ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: StartVibrator_001
 * @tc.desc: enable vibrator with effect
 * @tc.type: FUNC
 * @tc.require: AR000FLCJ3
 * @tc.author: guofuqiang
 */
HWTEST_F(VibratorAgentNativeTest, StartVibrator_001, TestSize.Level1)
{
    const char *supportString = "haptic.ringtone.Cartoon";
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, ERROR);
    EnableLooping();
    ret = DisableLooping();
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: StartVibrator_002
 * @tc.desc: enable vibrator with effect
 * @tc.type: FUNC
 * @tc.require: AR000FLCJ3
 * @tc.author: guofuqiang
 */
HWTEST_F(VibratorAgentNativeTest, StartVibrator_002, TestSize.Level1)
{
    const char *supportString = "haptic.common.click";
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    if (ret == ERROR) {
        ASSERT_EQ(ret, ERROR);
        EnableLooping();
        ret = DisableLooping();
        ASSERT_EQ(ret, NO_ERROR);
    }
}

/*
 * @tc.name: StartVibrator_003
 * @tc.desc: enable vibrator with effect
 * @tc.type: FUNC
 * @tc.require: AR000FLCJ3
 * @tc.author: guofuqiang
 */
HWTEST_F(VibratorAgentNativeTest, StartVibrator_003, TestSize.Level1)
{
    const char *supportString = "haptic.ringtone.Cartoon";
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, ERROR);
    ret = DisableLooping();
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: StartVibrator_004
 * @tc.desc: enable vibrator with effect
 * @tc.type: FUNC
 * @tc.require: 
 * @tc.author: wangcaiyi
 */
HWTEST_F(VibratorAgentNativeTest, StartVibrator_004, TestSize.Level1)
{
    const char *supportString = "vibrator.haptic.default.time";
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, NO_ERROR);
    EnableLooping();
    ret = DisableLooping();
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: StartVibrator_005
 * @tc.desc: enable vibrator with effect
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangcaiyi
 */
HWTEST_F(VibratorAgentNativeTest, StartVibrator_005, TestSize.Level1)
{
    const char *supportString = "vibrator.haptic.default.time";
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    if (ret == 0) {
        ASSERT_EQ(ret, NO_ERROR);
        EnableLooping();
        ret = DisableLooping();
        ASSERT_EQ(ret, NO_ERROR);
    }
}

/*
 * @tc.name: StartVibrator_006
 * @tc.desc: enable vibrator with effect
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangcaiyi
 */
HWTEST_F(VibratorAgentNativeTest, StartVibrator_006, TestSize.Level1)
{
    const char *supportString = "vibrator.haptic.default.time";
    int32_t ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, NO_ERROR);
    ret = DisableLooping();
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: DisableLooping_001
 * @tc.desc: disable vibrator
 * @tc.type: FUNC
 * @tc.require: AR000FLCJ3
 * @tc.author: guofuqiang
 */
HWTEST_F(VibratorAgentNativeTest, DisableLooping_001, TestSize.Level1)
{
    int32_t ret = DisableLooping();
    ASSERT_EQ(ret, ERROR);
}

/*
 * @tc.name: DisableLooping_002
 * @tc.desc: disable vibrator
 * @tc.type: FUNC
 * @tc.require: AR000FLCJ3
 * @tc.author: guofuqiang
 */
HWTEST_F(VibratorAgentNativeTest, DisableLooping_002, TestSize.Level1)
{
    int32_t ret = DisableLooping();
    ASSERT_EQ(ret, ERROR);
    const char *supportString = "haptic.ringtone.Cartoon";
    ret = StartVibrator(supportString);
    EnableLooping();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, ERROR);
    ret = DisableLooping();
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: DisableLooping_003
 * @tc.desc: disable vibrator
 * @tc.type: FUNC
 * @tc.require: AR000FLCJ3
 * @tc.author: guofuqiang
 */
HWTEST_F(VibratorAgentNativeTest, DisableLooping_003, TestSize.Level1)
{
    int32_t ret = DisableLooping();
    ASSERT_EQ(ret, ERROR);
    EnableLooping();
    const char *supportString = "haptic.ringtone.Cartoon";
    ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, NO_ERROR);
    ret = DisableLooping();
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: DisableLooping_004
 * @tc.desc: disable vibrator
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangcaiyi
 */
HWTEST_F(VibratorAgentNativeTest, DisableLooping_004, TestSize.Level1)
{
    int32_t ret = DisableLooping();
    ASSERT_EQ(ret, ERROR);
    const char *supportString = "vibrator.haptic.default.time";
    ret = StartVibrator(supportString);
    EnableLooping();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, NO_ERROR);
    ret = DisableLooping();
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: DisableLooping_005
 * @tc.desc: disable vibrator
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wangcaiyi
 */
HWTEST_F(VibratorAgentNativeTest, DisableLooping_005, TestSize.Level1)
{
    int32_t ret = DisableLooping();
    ASSERT_EQ(ret, ERROR);
    EnableLooping();
    const char *supportString = "vibrator.haptic.default.time";
    ret = StartVibrator(supportString);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, ERROR);
    ret = DisableLooping();
    ASSERT_EQ(ret, ERROR);
}
}  // namespace Sensors
}  // namespace OHOS