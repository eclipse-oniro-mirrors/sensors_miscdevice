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
 * @tc.name: GetLightIdList_001s
 * @tc.desc: get light support id
 * @tc.type: FUNC
 */
HWTEST_F(LightNativeTest, GetLightIdList_001, TestSize.Level1)
{
    std::vector<int32_t> setTarget;
    setTarget.push_back(LightId::LIGHT_ID_LED);
    std::map<std::vector<int32_t>, int32_t> mapTarget;
    mapTarget.insert(std::make_pair(setTarget, 1));
    std::vector<int32_t> setTest = lightServiceClient_->GetLightIdList();
    if (setTest.size() == 0) {
        return;
    }
    bool flag = true;
    auto it = mapTarget.find(setTest);
    if (it == mapTarget.end()) {
        flag = false;
    }
    ASSERT_TRUE(flag);
}

/*
 * @tc.name: PlayLightEffect_001
 * @tc.desc: perform light effect
 * @tc.type: FUNC
 */
HWTEST_F(LightNativeTest, PlayLightEffect_001, TestSize.Level1)
{
    int32_t ret;
    int32_t count = 0;
    std::vector<int32_t> setTest = lightServiceClient_->GetLightIdList();
    if (setTest.size() == 0) {
        return;
    }
    for (auto it = setTest.begin(); it != setTest.end(); it++) {
        if (*it == static_cast<int32_t>(LightId::LIGHT_ID_BELT)) {
            count++;
        }
    }
    if (count == 1) {
        ret = lightServiceClient_->PlayLightEffect(LightId::LIGHT_ID_BELT, "1");
        ASSERT_EQ(ret, ERR_OK);
    } else {
        uint32_t ret = lightServiceClient_->PlayLightEffect(LightId::LIGHT_ID_BELT, "1");
        ASSERT_EQ(ret, LIGHT_ID_NOT_SUPPORT);
    }
}

/*
 * @tc.name: StopLightEffect_001
 * @tc.desc: perform light effect
 * @tc.type: FUNC
 */
HWTEST_F(LightNativeTest, StopLightEffect_001, TestSize.Level1)
{
    int32_t ret;
    int32_t count = 0;
    std::vector<int32_t> setTest = lightServiceClient_->GetLightIdList();
    if (setTest.size() == 0) {
        return;
    }
    for (auto it = setTest.begin(); it != setTest.end(); it++) {
        if (*it == static_cast<int32_t>(LightId::LIGHT_ID_BELT)) {
            count++;
        }
    }
    if (count == 1) {
        ret = lightServiceClient_->StopLightEffect(LightId::LIGHT_ID_BELT);
        ASSERT_EQ(ret, ERR_OK);
    } else {
        uint32_t ret = lightServiceClient_->StopLightEffect(LightId::LIGHT_ID_BELT);
        ASSERT_EQ(ret, LIGHT_ID_NOT_SUPPORT);
    }
}
}  // namespace Sensors
}  // namespace OHOS