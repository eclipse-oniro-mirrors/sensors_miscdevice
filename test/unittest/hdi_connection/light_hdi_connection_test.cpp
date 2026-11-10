/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "hdi_connection_mock.h"
#include "light_hdi_connection.h"
#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "LightHdiConnectionTest"

namespace OHOS {
namespace Sensors {
using namespace testing;
using namespace testing::ext;

namespace {
constexpr int32_t LIGHT_ID_FIRST = 1;
constexpr int32_t LIGHT_ID_SECOND = 2;
constexpr int32_t INVALID_RET = -1;
}  // namespace

class LightHdiConnectionTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp()
    {
        lightHdiConnection_ = std::make_unique<LightHdiConnection>();
    }

    void TearDown() {}

protected:
    // Inject the mock into the private unique_ptr member (accessible with -Dprivate=public).
    void InjectMock()
    {
        auto mock = std::make_unique<MockLightHdiConnection>();
        mock_ = mock.get();
        lightHdiConnection_->iLightHdiConnection_ = std::move(mock);
    }

    std::unique_ptr<LightHdiConnection> lightHdiConnection_;
    MockLightHdiConnection *mock_ = nullptr;
};

/**
 * @tc.name: ConnectHdiService_001
 * @tc.desc: Test ConnectHdiService when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, ConnectHdiService_001, TestSize.Level2)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    ASSERT_EQ(lightHdiConnection_->iLightHdiConnection_, nullptr);
    EXPECT_EQ(lightHdiConnection_->ConnectHdiService(), ERROR);
}

/**
 * @tc.name: ConnectHdiService_002
 * @tc.desc: Test ConnectHdiService when the inner ConnectHdi of the hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, ConnectHdiService_002, TestSize.Level2)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, ConnectHdi()).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_CALL(*mock_, GetLightList(_)).Times(0);
    EXPECT_EQ(lightHdiConnection_->ConnectHdiService(), LIGHT_HDF_CONNECT_ERR);
}

/**
 * @tc.name: ConnectHdiService_003
 * @tc.desc: Test ConnectHdiService when the hdi connection is established successfully
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, ConnectHdiService_003, TestSize.Level1)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    InjectMock();
    std::vector<LightInfoIPC> mockList(2);  // default size, values are not important here
    EXPECT_CALL(*mock_, ConnectHdi()).Times(1).WillOnce(Return(ERR_OK));
    EXPECT_CALL(*mock_, GetLightList(_)).Times(1).WillOnce(
        DoAll(SetArgReferee<0>(mockList), Return(ERR_OK)));
    EXPECT_EQ(lightHdiConnection_->ConnectHdiService(), ERR_OK);
    EXPECT_EQ(lightHdiConnection_->lightInfoList_.size(), mockList.size());
}

/**
 * @tc.name: ConnectHdiService_004
 * @tc.desc: Test ConnectHdiService when GetLightList fails after the connection is established
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, ConnectHdiService_004, TestSize.Level2)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, ConnectHdi()).Times(1).WillOnce(Return(ERR_OK));
    EXPECT_CALL(*mock_, GetLightList(_)).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(lightHdiConnection_->ConnectHdiService(), INVALID_RET);
}

/**
 * @tc.name: ConnectHdi_001
 * @tc.desc: Test ConnectHdi, compatible connection is used as fallback when hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, ConnectHdi_001, TestSize.Level1)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    EXPECT_EQ(lightHdiConnection_->ConnectHdi(), ERR_OK);
    EXPECT_NE(lightHdiConnection_->iLightHdiConnection_, nullptr);
    std::vector<LightInfoIPC> lightList;
    EXPECT_EQ(lightHdiConnection_->GetLightList(lightList), ERR_OK);
}

/**
 * @tc.name: GetLightList_001
 * @tc.desc: Test GetLightList when the cached light list is not empty
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, GetLightList_001, TestSize.Level1)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    LightInfoIPC first;
    first.SetLightId(LIGHT_ID_FIRST);
    LightInfoIPC second;
    second.SetLightId(LIGHT_ID_SECOND);
    lightHdiConnection_->lightInfoList_ = {first, second};
    std::vector<LightInfoIPC> lightList;
    EXPECT_EQ(lightHdiConnection_->GetLightList(lightList), ERR_OK);
    ASSERT_EQ(lightList.size(), 2UL);
    EXPECT_EQ(lightList[0].GetLightId(), LIGHT_ID_FIRST);
    EXPECT_EQ(lightList[1].GetLightId(), LIGHT_ID_SECOND);
}

/**
 * @tc.name: GetLightList_002
 * @tc.desc: Test GetLightList when the cached light list is empty
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, GetLightList_002, TestSize.Level2)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    std::vector<LightInfoIPC> lightList;
    lightList.push_back(LightInfoIPC());
    EXPECT_EQ(lightHdiConnection_->GetLightList(lightList), ERR_OK);
    EXPECT_TRUE(lightList.empty());
}

/**
 * @tc.name: TurnOn_001
 * @tc.desc: Test TurnOn when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, TurnOn_001, TestSize.Level2)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    ASSERT_EQ(lightHdiConnection_->iLightHdiConnection_, nullptr);
    LightColor color;
    color.singleColor = 0xFF;
    LightAnimationIPC animation;
    EXPECT_EQ(lightHdiConnection_->TurnOn(LIGHT_ID_FIRST, color, animation), ERROR);
}

/**
 * @tc.name: TurnOn_002
 * @tc.desc: Test TurnOn when the underlying hdi connection turns on the light successfully
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, TurnOn_002, TestSize.Level1)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    InjectMock();
    LightColor color;
    color.singleColor = 0xFF;
    LightAnimationIPC animation;
    EXPECT_CALL(*mock_, TurnOn(LIGHT_ID_FIRST, _, _)).Times(1).WillOnce(Return(ERR_OK));
    EXPECT_EQ(lightHdiConnection_->TurnOn(LIGHT_ID_FIRST, color, animation), ERR_OK);
}

/**
 * @tc.name: TurnOn_003
 * @tc.desc: Test TurnOn when the underlying hdi connection fails to turn on the light
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, TurnOn_003, TestSize.Level2)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    InjectMock();
    LightColor color;
    color.singleColor = 0xFF;
    LightAnimationIPC animation;
    EXPECT_CALL(*mock_, TurnOn(_, _, _)).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(lightHdiConnection_->TurnOn(LIGHT_ID_FIRST, color, animation), LIGHT_ID_NOT_SUPPORT);
}

/**
 * @tc.name: TurnOff_001
 * @tc.desc: Test TurnOff when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, TurnOff_001, TestSize.Level2)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    ASSERT_EQ(lightHdiConnection_->iLightHdiConnection_, nullptr);
    EXPECT_EQ(lightHdiConnection_->TurnOff(LIGHT_ID_FIRST), ERROR);
}

/**
 * @tc.name: TurnOff_002
 * @tc.desc: Test TurnOff when the underlying hdi connection turns off the light successfully
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, TurnOff_002, TestSize.Level1)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, TurnOff(LIGHT_ID_FIRST)).Times(1).WillOnce(Return(ERR_OK));
    EXPECT_EQ(lightHdiConnection_->TurnOff(LIGHT_ID_FIRST), ERR_OK);
}

/**
 * @tc.name: TurnOff_003
 * @tc.desc: Test TurnOff when the underlying hdi connection fails to turn off the light
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, TurnOff_003, TestSize.Level2)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, TurnOff(_)).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(lightHdiConnection_->TurnOff(LIGHT_ID_FIRST), LIGHT_ERR);
}

/**
 * @tc.name: DestroyHdiConnection_001
 * @tc.desc: Test DestroyHdiConnection when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, DestroyHdiConnection_001, TestSize.Level2)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    ASSERT_EQ(lightHdiConnection_->iLightHdiConnection_, nullptr);
    EXPECT_EQ(lightHdiConnection_->DestroyHdiConnection(), ERROR);
}

/**
 * @tc.name: DestroyHdiConnection_002
 * @tc.desc: Test DestroyHdiConnection when the underlying hdi connection is destroyed successfully
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, DestroyHdiConnection_002, TestSize.Level1)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, DestroyHdiConnection()).Times(1).WillOnce(Return(ERR_OK));
    EXPECT_EQ(lightHdiConnection_->DestroyHdiConnection(), ERR_OK);
}

/**
 * @tc.name: DestroyHdiConnection_003
 * @tc.desc: Test DestroyHdiConnection when the underlying hdi connection fails to destroy
 * @tc.type: FUNC
 */
HWTEST_F(LightHdiConnectionTest, DestroyHdiConnection_003, TestSize.Level2)
{
    ASSERT_NE(lightHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, DestroyHdiConnection()).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(lightHdiConnection_->DestroyHdiConnection(), LIGHT_HDF_CONNECT_ERR);
}
}  // namespace Sensors
}  // namespace OHOS
