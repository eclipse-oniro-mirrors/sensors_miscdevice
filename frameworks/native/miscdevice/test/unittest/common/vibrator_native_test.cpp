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
#include "vibrator_service_client.h"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, SensorsLogDomain::MISCDEVICE_VIBRATOR_NATIVE, "VibratorNativeTest" };
}

class VibratorNativeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    int32_t vibratorId_ = 123;
    std::unique_ptr<VibratorServiceClient> vibratorServiceClient_;
};

void VibratorNativeTest::SetUp()
{
    vibratorServiceClient_ = std::make_unique<VibratorServiceClient>();
    ASSERT_NE(vibratorServiceClient_, nullptr);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

void VibratorNativeTest::TearDown()
{
    vibratorServiceClient_ = nullptr;
}

/*
 * @tc.name: IsVibratorEffectSupport_001
 * @tc.desc: IS support vibrator effect.
 * @tc.type: FUNC
 */
HWTEST_F(VibratorNativeTest, IsVibratorEffectSupport_001, TestSize.Level1)
{
    std::string supportString("haptic.clock.timer");
    bool flag = vibratorServiceClient_->IsVibratorEffectSupport(vibratorId_, supportString);
    if (!flag) {
        return;
    }
    ASSERT_TRUE(flag);
    std::string notsupportString("haptic");
    flag = vibratorServiceClient_->IsVibratorEffectSupport(vibratorId_, notsupportString);
    ASSERT_TRUE(flag);
}

/*
 * @tc.name: VibratorOn_001
 * @tc.desc: vibrator on
 * @tc.type: FUNC
 */
HWTEST_F(VibratorNativeTest, VibratorOn_001, TestSize.Level1)
{
    uint32_t time = 2000;
    int32_t ret = vibratorServiceClient_->Vibrate(vibratorId_, time);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: VibratorOff_001
 * @tc.desc: vibrator off
 * @tc.type: FUNC
 */
HWTEST_F(VibratorNativeTest, VibratorOff_001, TestSize.Level1)
{
    int32_t ret = vibratorServiceClient_->Stop(vibratorId_, "time");
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: GetVibratorIdList_001
 * @tc.desc: get vibrator list
 * @tc.type: FUNC
 */
HWTEST_F(VibratorNativeTest, GetVibratorIdList_001, TestSize.Level1)
{
    std::vector<int32_t> vibratorIdList = vibratorServiceClient_->GetVibratorIdList();
    ASSERT_GT(vibratorIdList.size(), 0UL);
}

/*
 * @tc.name: GetVibratorParameter_001
 * @tc.desc: get vibrator parameter
 * @tc.type: FUNC
 */
HWTEST_F(VibratorNativeTest, GetVibratorParameter_001, TestSize.Level1)
{
    std::string ret = vibratorServiceClient_->GetVibratorParameter(vibratorId_, "mmi_vibrator_calib_get_result");
    HiLog::Info(LABEL, "%{public}s ret : %{public}s", __func__, ret.c_str());
}

/*
 * @tc.name: PlayVibratorEffect_001
 * @tc.desc: perform vibrator effect
 * @tc.type: FUNC
 */
HWTEST_F(VibratorNativeTest, PlayVibratorEffect_001, TestSize.Level1)
{
    std::string supportString("haptic.clock.timer");
    int32_t ret;
    if (vibratorServiceClient_->IsVibratorEffectSupport(vibratorId_, supportString)) {
        ret = vibratorServiceClient_->Vibrate(vibratorId_, supportString, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        if (ret == ERROR) {
            ASSERT_EQ(ret, ERROR);
        }
    }
}

/*
 * @tc.name: PlayVibratorEffect_002
 * @tc.desc: perform vibrator effect repeat
 * @tc.type: FUNC
 * @tc.require: AR000F46AB
 * @tc.author: guofuqiang
 */
HWTEST_F(VibratorNativeTest, PlayVibratorEffect_002, TestSize.Level1)
{
    std::string supportString("haptic.ringtone.Bounce");
    if (vibratorServiceClient_->IsVibratorEffectSupport(vibratorId_, supportString)) {
        int32_t ret = vibratorServiceClient_->Vibrate(vibratorId_, supportString, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        if (ret == NO_ERROR) {
            ASSERT_EQ(ret, NO_ERROR);
        }
        ret = vibratorServiceClient_->Stop(vibratorId_, "preset");
        if (ret == NO_ERROR) {
            ASSERT_EQ(ret, NO_ERROR);
        }
    }
}

/*
 * @tc.name: StopVibratorEffect_001
 * @tc.desc: stop vibrator effect
 * @tc.type: FUNC
 */
HWTEST_F(VibratorNativeTest, StopVibratorEffect_001, TestSize.Level1)
{
    std::string supportString("preset");
    int32_t ret = vibratorServiceClient_->Stop(vibratorId_, supportString);
    if (ret == NO_ERROR) {
        ASSERT_EQ(ret, NO_ERROR);
    }
}

/*
 * @tc.name: PlayCustomVibratorEffect_001
 * @tc.desc: perform custom vibrator effect
 * @tc.type: FUNC
 */
HWTEST_F(VibratorNativeTest, PlayCustomVibratorEffect_001, TestSize.Level1)
{
    std::vector<int32_t> timing;
    std::vector<int32_t> intensity = { 100, 120, 210, 50 };
    int32_t periodCount = 5;
    int32_t ret;

    timing.push_back(500);  // off time
    timing.push_back(500);  // on time
    timing.push_back(500);  // off time
    timing.push_back(500);  // on time

    ret = vibratorServiceClient_->Vibrate(vibratorId_, timing, intensity, periodCount);
    ASSERT_EQ(ret, NO_ERROR);

    std::this_thread::sleep_for(std::chrono::milliseconds(4000));

    timing.clear();
    timing.push_back(3000);  // off time
    timing.push_back(3000);  // on time
    intensity.clear();
    intensity.push_back(200);
    intensity.push_back(100);
    ret = vibratorServiceClient_->Vibrate(vibratorId_, timing, intensity, periodCount);
    ASSERT_EQ(ret, NO_ERROR);
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    ret = vibratorServiceClient_->Stop(vibratorId_, "customized");
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: SetVibratorParameter_001
 * @tc.desc: set vibrator parameter
 * @tc.type: FUNC
 */
HWTEST_F(VibratorNativeTest, SetVibratorParameter_001, TestSize.Level1)
{
    std::string s("mmi_vibrator_calib_on");
    int32_t ret = vibratorServiceClient_->SetVibratorParameter(vibratorId_, s);
    ASSERT_EQ(ret, NO_ERROR);
}
}  // namespace Sensors
}  // namespace OHOS