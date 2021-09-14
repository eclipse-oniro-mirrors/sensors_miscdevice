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
#include "jni.h"

#include "light_service_client.h"
#include "sensors_errors.h"
#include "sensors_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, SensorsLogDomain::MISCDEVICE_LIGHT_NATIVE, "LightNativeTest" };
}

class LightNativeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    std::unique_ptr<LightServiceClient> lightServiceClient_;
};

void LightNativeTest::SetUp()
{
    lightServiceClient_ = std::make_unique<LightServiceClient>();
    ASSERT_NE(lightServiceClient_, nullptr);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

void LightNativeTest::TearDown()
{
    lightServiceClient_ = nullptr;
}

/*
 * @tc.name: Light_ON_001
 * @tc.desc: light on
 * @tc.type: FUNC
 */
HWTEST_F(LightNativeTest, Light_ON_001, TestSize.Level1)
{
    std::vector<int32_t> setTest = lightServiceClient_->GetLightIdList();
    if (setTest.size() == 0) {
        return;
    }
    uint32_t corlorBrightness = 0x00ffffff;
    int32_t ret = lightServiceClient_->Light(LightId::LIGHT_ID_LED, corlorBrightness, 2000, 1000);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(ret, NO_ERROR);
}

/*
 * @tc.name: Light_OFF_001
 * @tc.desc: light off
 * @tc.type: FUNC
 */
HWTEST_F(LightNativeTest, Light_OFF_001, TestSize.Level1)
{
    std::vector<int32_t> setTest = lightServiceClient_->GetLightIdList();
    if (setTest.size() == 0) {
        return;
    }
    uint32_t corlorBrightness = 0;
    int32_t ret = lightServiceClient_->Light(LightId::LIGHT_ID_LED, corlorBrightness, 0, 0);
    ASSERT_EQ(ret, NO_ERROR);
}
}  // namespace Sensors
}  // namespace OHOS