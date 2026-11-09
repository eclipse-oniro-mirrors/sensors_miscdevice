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
#include <optional>
#include <string>
#include <vector>

#include "hdi_connection_mock.h"
#include "sensors_errors.h"
#include "vibrator_hdi_connection.h"

#undef LOG_TAG
#define LOG_TAG "VibratorHdiConnectionTest"

namespace OHOS {
namespace Sensors {
using namespace testing;
using namespace testing::ext;

// VibratorIdentifierIPC has no operator== in production code; define one here so that
// EXPECT_CALL can match exact values.
bool operator==(const VibratorIdentifierIPC &lhs, const VibratorIdentifierIPC &rhs)
{
    return (lhs.deviceId == rhs.deviceId) && (lhs.vibratorId == rhs.vibratorId) &&
        (lhs.position == rhs.position) && (lhs.isLocalVibrator == rhs.isLocalVibrator);
}

namespace {
constexpr uint32_t DURATION = 30;
constexpr uint32_t SESSION_ID = 1;
constexpr int32_t INVALID_RET = -1;
constexpr int32_t DELAY_TIME = 20;
constexpr int32_t INTENSITY = 50;
const std::string EFFECT_TYPE = "haptic.clock.tick";
}  // namespace

class VibratorHdiConnectionTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp()
    {
        vibratorHdiConnection_ = std::make_unique<VibratorHdiConnection>();
        identifier_.deviceId = 0;
        identifier_.vibratorId = 0;
        identifier_.isLocalVibrator = true;
    }

    void TearDown() {}

protected:
    // Inject the mock into the private unique_ptr member (accessible with -Dprivate=public).
    void InjectMock()
    {
        auto mock = std::make_unique<MockVibratorHdiConnection>();
        mock_ = mock.get();
        vibratorHdiConnection_->iVibratorHdiConnection_ = std::move(mock);
    }

    std::unique_ptr<VibratorHdiConnection> vibratorHdiConnection_;
    MockVibratorHdiConnection *mock_ = nullptr;
    VibratorIdentifierIPC identifier_;
};

/**
 * @tc.name: ConnectHdi_001
 * @tc.desc: Test ConnectHdi, the inner hdi connection shall be created after connecting
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, ConnectHdi_001, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_NO_FATAL_FAILURE(vibratorHdiConnection_->ConnectHdi());
    EXPECT_NE(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
}

/**
 * @tc.name: StartOnce_001
 * @tc.desc: Test StartOnce when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, StartOnce_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    EXPECT_EQ(vibratorHdiConnection_->StartOnce(identifier_, DURATION), VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: StartOnce_002
 * @tc.desc: Test StartOnce when the underlying hdi connection vibrates once successfully
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, StartOnce_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, StartOnce(identifier_, DURATION)).Times(1).WillOnce(Return(0));
    EXPECT_EQ(vibratorHdiConnection_->StartOnce(identifier_, DURATION), ERR_OK);
}

/**
 * @tc.name: StartOnce_003
 * @tc.desc: Test StartOnce when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, StartOnce_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, StartOnce(_, _)).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(vibratorHdiConnection_->StartOnce(identifier_, DURATION), VIBRATOR_ON_ERR);
}

/**
 * @tc.name: Start_001
 * @tc.desc: Test Start when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, Start_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    EXPECT_EQ(vibratorHdiConnection_->Start(identifier_, EFFECT_TYPE), VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: Start_002
 * @tc.desc: Test Start when the underlying hdi connection starts the effect successfully
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, Start_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, Start(identifier_, EFFECT_TYPE)).Times(1).WillOnce(Return(0));
    EXPECT_EQ(vibratorHdiConnection_->Start(identifier_, EFFECT_TYPE), ERR_OK);
}

/**
 * @tc.name: Start_003
 * @tc.desc: Test Start when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, Start_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, Start(_, _)).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(vibratorHdiConnection_->Start(identifier_, EFFECT_TYPE), VIBRATOR_ON_ERR);
}

#ifdef OHOS_BUILD_ENABLE_VIBRATOR_CUSTOM
/**
 * @tc.name: EnableCompositeEffect_001
 * @tc.desc: Test EnableCompositeEffect when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, EnableCompositeEffect_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    HdfCompositeEffect hdfCompositeEffect;
    EXPECT_EQ(vibratorHdiConnection_->EnableCompositeEffect(identifier_, hdfCompositeEffect),
        VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: EnableCompositeEffect_002
 * @tc.desc: Test EnableCompositeEffect when the underlying hdi connection succeeds
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, EnableCompositeEffect_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    HdfCompositeEffect hdfCompositeEffect;
    EXPECT_CALL(*mock_, EnableCompositeEffect(identifier_, _)).Times(1).WillOnce(Return(0));
    EXPECT_EQ(vibratorHdiConnection_->EnableCompositeEffect(identifier_, hdfCompositeEffect), ERR_OK);
}

/**
 * @tc.name: EnableCompositeEffect_003
 * @tc.desc: Test EnableCompositeEffect when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, EnableCompositeEffect_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    HdfCompositeEffect hdfCompositeEffect;
    EXPECT_CALL(*mock_, EnableCompositeEffect(_, _)).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(vibratorHdiConnection_->EnableCompositeEffect(identifier_, hdfCompositeEffect), VIBRATOR_ON_ERR);
}

/**
 * @tc.name: IsVibratorRunning_001
 * @tc.desc: Test IsVibratorRunning when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, IsVibratorRunning_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    EXPECT_FALSE(vibratorHdiConnection_->IsVibratorRunning(identifier_));
}

/**
 * @tc.name: IsVibratorRunning_002
 * @tc.desc: Test IsVibratorRunning when the vibrator is running
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, IsVibratorRunning_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, IsVibratorRunning(identifier_)).Times(1).WillOnce(Return(true));
    EXPECT_TRUE(vibratorHdiConnection_->IsVibratorRunning(identifier_));
}

/**
 * @tc.name: IsVibratorRunning_003
 * @tc.desc: Test IsVibratorRunning when the vibrator is not running
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, IsVibratorRunning_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, IsVibratorRunning(_)).Times(1).WillOnce(Return(false));
    EXPECT_FALSE(vibratorHdiConnection_->IsVibratorRunning(identifier_));
}
#endif // OHOS_BUILD_ENABLE_VIBRATOR_CUSTOM

#ifdef HDF_DRIVERS_INTERFACE_VIBRATOR
/**
 * @tc.name: GetEffectInfoOptional_001
 * @tc.desc: Test GetEffectInfo(optional) when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetEffectInfoOptional_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    EXPECT_FALSE(vibratorHdiConnection_->GetEffectInfo(identifier_, EFFECT_TYPE).has_value());
}

/**
 * @tc.name: GetEffectInfoOptional_002
 * @tc.desc: Test GetEffectInfo(optional) when the effect is supported
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetEffectInfoOptional_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    HdfEffectInfo effectInfo;
    EXPECT_CALL(*mock_, GetEffectInfo(identifier_, EFFECT_TYPE)).Times(1).WillOnce(Return(effectInfo));
    std::optional<HdfEffectInfo> ret = vibratorHdiConnection_->GetEffectInfo(identifier_, EFFECT_TYPE);
    EXPECT_TRUE(ret.has_value());
}

/**
 * @tc.name: GetEffectInfoOptional_003
 * @tc.desc: Test GetEffectInfo(optional) when the effect is not supported
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetEffectInfoOptional_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, GetEffectInfo(_, _)).Times(1).WillOnce(Return(std::nullopt));
    EXPECT_FALSE(vibratorHdiConnection_->GetEffectInfo(identifier_, EFFECT_TYPE).has_value());
}

/**
 * @tc.name: Stop_001
 * @tc.desc: Test Stop when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, Stop_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    EXPECT_EQ(vibratorHdiConnection_->Stop(identifier_, HDF_VIBRATOR_MODE_ONCE), VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: Stop_002
 * @tc.desc: Test Stop when the underlying hdi connection stops the vibrator successfully
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, Stop_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, Stop(identifier_, HDF_VIBRATOR_MODE_ONCE)).Times(1).WillOnce(Return(0));
    EXPECT_EQ(vibratorHdiConnection_->Stop(identifier_, HDF_VIBRATOR_MODE_ONCE), ERR_OK);
}

/**
 * @tc.name: Stop_003
 * @tc.desc: Test Stop when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, Stop_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, Stop(_, _)).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(vibratorHdiConnection_->Stop(identifier_, HDF_VIBRATOR_MODE_ONCE), VIBRATOR_OFF_ERR);
}
#endif // HDF_DRIVERS_INTERFACE_VIBRATOR

/**
 * @tc.name: DestroyHdiConnection_001
 * @tc.desc: Test DestroyHdiConnection when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, DestroyHdiConnection_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    EXPECT_EQ(vibratorHdiConnection_->DestroyHdiConnection(), VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: DestroyHdiConnection_002
 * @tc.desc: Test DestroyHdiConnection when the underlying hdi connection is destroyed successfully
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, DestroyHdiConnection_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, DestroyHdiConnection()).Times(1).WillOnce(Return(0));
    EXPECT_EQ(vibratorHdiConnection_->DestroyHdiConnection(), ERR_OK);
}

/**
 * @tc.name: DestroyHdiConnection_003
 * @tc.desc: Test DestroyHdiConnection when the underlying hdi connection fails to destroy
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, DestroyHdiConnection_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, DestroyHdiConnection()).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(vibratorHdiConnection_->DestroyHdiConnection(), VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: GetDelayTime_001
 * @tc.desc: Test GetDelayTime when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetDelayTime_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    int32_t delayTime = 0;
    EXPECT_EQ(vibratorHdiConnection_->GetDelayTime(identifier_, 0, delayTime), VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: GetDelayTime_002
 * @tc.desc: Test GetDelayTime when the underlying hdi connection returns the delay time
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetDelayTime_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, GetDelayTime(identifier_, 0, _)).Times(1).WillOnce(
        DoAll(SetArgReferee<2>(DELAY_TIME), Return(ERR_OK)));
    int32_t delayTime = 0;
    EXPECT_EQ(vibratorHdiConnection_->GetDelayTime(identifier_, 0, delayTime), ERR_OK);
    EXPECT_EQ(delayTime, DELAY_TIME);
}

/**
 * @tc.name: GetDelayTime_003
 * @tc.desc: Test GetDelayTime when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetDelayTime_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, GetDelayTime(_, _, _)).Times(1).WillOnce(Return(INVALID_RET));
    int32_t delayTime = 0;
    EXPECT_EQ(vibratorHdiConnection_->GetDelayTime(identifier_, 0, delayTime), INVALID_RET);
}

/**
 * @tc.name: GetVibratorCapacity_001
 * @tc.desc: Test GetVibratorCapacity when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetVibratorCapacity_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    VibratorCapacity capacity;
    EXPECT_EQ(vibratorHdiConnection_->GetVibratorCapacity(identifier_, capacity), VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: GetVibratorCapacity_002
 * @tc.desc: Test GetVibratorCapacity when the underlying hdi connection returns the capacity
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetVibratorCapacity_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    VibratorCapacity capacity;
    capacity.isSupportHdHaptic = true;
    EXPECT_CALL(*mock_, GetVibratorCapacity(identifier_, _)).Times(1).WillOnce(
        DoAll(SetArgReferee<1>(capacity), Return(ERR_OK)));
    VibratorCapacity result;
    EXPECT_EQ(vibratorHdiConnection_->GetVibratorCapacity(identifier_, result), ERR_OK);
    EXPECT_TRUE(result.isSupportHdHaptic);
}

/**
 * @tc.name: GetVibratorCapacity_003
 * @tc.desc: Test GetVibratorCapacity when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetVibratorCapacity_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, GetVibratorCapacity(_, _)).Times(1).WillOnce(Return(INVALID_RET));
    VibratorCapacity capacity;
    EXPECT_EQ(vibratorHdiConnection_->GetVibratorCapacity(identifier_, capacity), INVALID_RET);
}

/**
 * @tc.name: PlayPattern_001
 * @tc.desc: Test PlayPattern when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, PlayPattern_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    VibratePattern pattern;
    EXPECT_EQ(vibratorHdiConnection_->PlayPattern(identifier_, pattern), VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: PlayPattern_002
 * @tc.desc: Test PlayPattern when the underlying hdi connection plays the pattern successfully
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, PlayPattern_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    VibratePattern pattern;
    EXPECT_CALL(*mock_, PlayPattern(identifier_, _)).Times(1).WillOnce(Return(ERR_OK));
    EXPECT_EQ(vibratorHdiConnection_->PlayPattern(identifier_, pattern), ERR_OK);
}

/**
 * @tc.name: PlayPattern_003
 * @tc.desc: Test PlayPattern when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, PlayPattern_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    VibratePattern pattern;
    EXPECT_CALL(*mock_, PlayPattern(_, _)).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(vibratorHdiConnection_->PlayPattern(identifier_, pattern), INVALID_RET);
}

/**
 * @tc.name: PlayPatternBySessionId_001
 * @tc.desc: Test PlayPatternBySessionId when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, PlayPatternBySessionId_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    VibratePattern pattern;
    EXPECT_EQ(vibratorHdiConnection_->PlayPatternBySessionId(identifier_, SESSION_ID, pattern),
        VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: PlayPatternBySessionId_002
 * @tc.desc: Test PlayPatternBySessionId when the underlying hdi connection succeeds
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, PlayPatternBySessionId_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    VibratePattern pattern;
    EXPECT_CALL(*mock_, PlayPatternBySessionId(identifier_, SESSION_ID, _)).Times(1).WillOnce(Return(ERR_OK));
    EXPECT_EQ(vibratorHdiConnection_->PlayPatternBySessionId(identifier_, SESSION_ID, pattern), ERR_OK);
}

/**
 * @tc.name: PlayPatternBySessionId_003
 * @tc.desc: Test PlayPatternBySessionId when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, PlayPatternBySessionId_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    VibratePattern pattern;
    EXPECT_CALL(*mock_, PlayPatternBySessionId(_, _, _)).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(vibratorHdiConnection_->PlayPatternBySessionId(identifier_, SESSION_ID, pattern), INVALID_RET);
}

#ifdef HDF_DRIVERS_INTERFACE_VIBRATOR
/**
 * @tc.name: PlayPackageBySessionId_001
 * @tc.desc: Test PlayPackageBySessionId when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, PlayPackageBySessionId_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    VibratePackage package;
    EXPECT_EQ(vibratorHdiConnection_->PlayPackageBySessionId(identifier_, SESSION_ID, package),
        VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: PlayPackageBySessionId_002
 * @tc.desc: Test PlayPackageBySessionId when the underlying hdi connection succeeds
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, PlayPackageBySessionId_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    VibratePackage package;
    EXPECT_CALL(*mock_, PlayPackageBySessionId(identifier_, SESSION_ID, _)).Times(1).WillOnce(Return(ERR_OK));
    EXPECT_EQ(vibratorHdiConnection_->PlayPackageBySessionId(identifier_, SESSION_ID, package), ERR_OK);
}

/**
 * @tc.name: PlayPackageBySessionId_003
 * @tc.desc: Test PlayPackageBySessionId when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, PlayPackageBySessionId_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    VibratePackage package;
    EXPECT_CALL(*mock_, PlayPackageBySessionId(_, _, _)).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(vibratorHdiConnection_->PlayPackageBySessionId(identifier_, SESSION_ID, package), INVALID_RET);
}

/**
 * @tc.name: StopVibrateBySessionId_001
 * @tc.desc: Test StopVibrateBySessionId when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, StopVibrateBySessionId_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    EXPECT_EQ(vibratorHdiConnection_->StopVibrateBySessionId(identifier_, SESSION_ID), VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: StopVibrateBySessionId_002
 * @tc.desc: Test StopVibrateBySessionId when the underlying hdi connection succeeds
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, StopVibrateBySessionId_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, StopVibrateBySessionId(identifier_, SESSION_ID)).Times(1).WillOnce(Return(ERR_OK));
    EXPECT_EQ(vibratorHdiConnection_->StopVibrateBySessionId(identifier_, SESSION_ID), ERR_OK);
}

/**
 * @tc.name: StopVibrateBySessionId_003
 * @tc.desc: Test StopVibrateBySessionId when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, StopVibrateBySessionId_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, StopVibrateBySessionId(_, _)).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(vibratorHdiConnection_->StopVibrateBySessionId(identifier_, SESSION_ID), INVALID_RET);
}
#endif // HDF_DRIVERS_INTERFACE_VIBRATOR

/**
 * @tc.name: StartByIntensity_001
 * @tc.desc: Test StartByIntensity when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, StartByIntensity_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    EXPECT_EQ(vibratorHdiConnection_->StartByIntensity(identifier_, EFFECT_TYPE, INTENSITY),
        VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: StartByIntensity_002
 * @tc.desc: Test StartByIntensity when the underlying hdi connection succeeds
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, StartByIntensity_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, StartByIntensity(identifier_, EFFECT_TYPE, INTENSITY)).Times(1).WillOnce(Return(0));
    EXPECT_EQ(vibratorHdiConnection_->StartByIntensity(identifier_, EFFECT_TYPE, INTENSITY), ERR_OK);
}

/**
 * @tc.name: StartByIntensity_003
 * @tc.desc: Test StartByIntensity when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, StartByIntensity_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, StartByIntensity(_, _, _)).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(vibratorHdiConnection_->StartByIntensity(identifier_, EFFECT_TYPE, INTENSITY), VIBRATOR_ON_ERR);
}

#ifdef HDF_DRIVERS_INTERFACE_VIBRATOR
/**
 * @tc.name: GetVibratorInfo_001
 * @tc.desc: Test GetVibratorInfo when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetVibratorInfo_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    std::vector<HdfVibratorInfo> vibratorInfo;
    EXPECT_EQ(vibratorHdiConnection_->GetVibratorInfo(vibratorInfo), VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: GetVibratorInfo_002
 * @tc.desc: Test GetVibratorInfo when the underlying hdi connection returns the info list
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetVibratorInfo_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    std::vector<HdfVibratorInfo> mockInfo(2);
    EXPECT_CALL(*mock_, GetVibratorInfo(_)).Times(1).WillOnce(
        DoAll(SetArgReferee<0>(mockInfo), Return(ERR_OK)));
    std::vector<HdfVibratorInfo> vibratorInfo;
    EXPECT_EQ(vibratorHdiConnection_->GetVibratorInfo(vibratorInfo), ERR_OK);
    EXPECT_EQ(vibratorInfo.size(), mockInfo.size());
}

/**
 * @tc.name: GetVibratorInfo_003
 * @tc.desc: Test GetVibratorInfo when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetVibratorInfo_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, GetVibratorInfo(_)).Times(1).WillOnce(Return(INVALID_RET));
    std::vector<HdfVibratorInfo> vibratorInfo;
    EXPECT_EQ(vibratorHdiConnection_->GetVibratorInfo(vibratorInfo), INVALID_RET);
}

/**
 * @tc.name: GetAllWaveInfo_001
 * @tc.desc: Test GetAllWaveInfo when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetAllWaveInfo_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    std::vector<HdfWaveInformation> waveInfos;
    EXPECT_EQ(vibratorHdiConnection_->GetAllWaveInfo(identifier_, waveInfos), VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: GetAllWaveInfo_002
 * @tc.desc: Test GetAllWaveInfo when the underlying hdi connection returns the wave info list
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetAllWaveInfo_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    std::vector<HdfWaveInformation> mockWaveInfos(2);
    EXPECT_CALL(*mock_, GetAllWaveInfo(identifier_, _)).Times(1).WillOnce(
        DoAll(SetArgReferee<1>(mockWaveInfos), Return(ERR_OK)));
    std::vector<HdfWaveInformation> waveInfos;
    EXPECT_EQ(vibratorHdiConnection_->GetAllWaveInfo(identifier_, waveInfos), ERR_OK);
    EXPECT_EQ(waveInfos.size(), mockWaveInfos.size());
}

/**
 * @tc.name: GetAllWaveInfo_003
 * @tc.desc: Test GetAllWaveInfo when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetAllWaveInfo_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, GetAllWaveInfo(_, _)).Times(1).WillOnce(Return(INVALID_RET));
    std::vector<HdfWaveInformation> waveInfos;
    EXPECT_EQ(vibratorHdiConnection_->GetAllWaveInfo(identifier_, waveInfos), INVALID_RET);
}

/**
 * @tc.name: GetVibratorList_001
 * @tc.desc: Test GetVibratorList when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetVibratorList_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    std::vector<HdfVibratorInfo> vibratorInfo;
    EXPECT_EQ(vibratorHdiConnection_->GetVibratorList(identifier_, vibratorInfo), VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: GetVibratorList_002
 * @tc.desc: Test GetVibratorList when the underlying hdi connection returns the vibrator list
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetVibratorList_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    std::vector<HdfVibratorInfo> mockVibratorInfo(2);
    EXPECT_CALL(*mock_, GetVibratorList(identifier_, _)).Times(1).WillOnce(
        DoAll(SetArgReferee<1>(mockVibratorInfo), Return(ERR_OK)));
    std::vector<HdfVibratorInfo> vibratorInfo;
    EXPECT_EQ(vibratorHdiConnection_->GetVibratorList(identifier_, vibratorInfo), ERR_OK);
    EXPECT_EQ(vibratorInfo.size(), mockVibratorInfo.size());
}

/**
 * @tc.name: GetVibratorList_003
 * @tc.desc: Test GetVibratorList when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetVibratorList_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, GetVibratorList(_, _)).Times(1).WillOnce(Return(INVALID_RET));
    std::vector<HdfVibratorInfo> vibratorInfo;
    EXPECT_EQ(vibratorHdiConnection_->GetVibratorList(identifier_, vibratorInfo), INVALID_RET);
}

/**
 * @tc.name: GetEffectInfoRef_001
 * @tc.desc: Test GetEffectInfo(reference) when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetEffectInfoRef_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    HdfEffectInfo effectInfo;
    EXPECT_EQ(vibratorHdiConnection_->GetEffectInfo(identifier_, EFFECT_TYPE, effectInfo),
        VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: GetEffectInfoRef_002
 * @tc.desc: Test GetEffectInfo(reference) when the underlying hdi connection returns the effect info
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetEffectInfoRef_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    HdfEffectInfo mockEffectInfo;
    EXPECT_CALL(*mock_, GetEffectInfo(identifier_, EFFECT_TYPE, _)).Times(1).WillOnce(
        DoAll(SetArgReferee<2>(mockEffectInfo), Return(ERR_OK)));
    HdfEffectInfo effectInfo;
    EXPECT_EQ(vibratorHdiConnection_->GetEffectInfo(identifier_, EFFECT_TYPE, effectInfo), ERR_OK);
}

/**
 * @tc.name: GetEffectInfoRef_003
 * @tc.desc: Test GetEffectInfo(reference) when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetEffectInfoRef_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_CALL(*mock_, GetEffectInfo(_, _, _)).Times(1).WillOnce(Return(INVALID_RET));
    HdfEffectInfo effectInfo;
    EXPECT_EQ(vibratorHdiConnection_->GetEffectInfo(identifier_, EFFECT_TYPE, effectInfo), INVALID_RET);
}
#endif // HDF_DRIVERS_INTERFACE_VIBRATOR

/**
 * @tc.name: RegisterVibratorPlugCallback_001
 * @tc.desc: Test RegisterVibratorPlugCallback when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, RegisterVibratorPlugCallback_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    DevicePlugCallback callback = [](const HdfVibratorPlugInfo &plugInfo) {};
    EXPECT_EQ(vibratorHdiConnection_->RegisterVibratorPlugCallback(callback), VIBRATOR_HDF_CONNECT_ERR);
}

/**
 * @tc.name: RegisterVibratorPlugCallback_002
 * @tc.desc: Test RegisterVibratorPlugCallback when the underlying hdi connection succeeds
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, RegisterVibratorPlugCallback_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    DevicePlugCallback callback = [](const HdfVibratorPlugInfo &plugInfo) {};
    EXPECT_CALL(*mock_, RegisterVibratorPlugCallback(_)).Times(1).WillOnce(Return(0));
    EXPECT_EQ(vibratorHdiConnection_->RegisterVibratorPlugCallback(callback), ERR_OK);
}

/**
 * @tc.name: RegisterVibratorPlugCallback_003
 * @tc.desc: Test RegisterVibratorPlugCallback when the underlying hdi connection fails
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, RegisterVibratorPlugCallback_003, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    DevicePlugCallback callback = [](const HdfVibratorPlugInfo &plugInfo) {};
    EXPECT_CALL(*mock_, RegisterVibratorPlugCallback(_)).Times(1).WillOnce(Return(INVALID_RET));
    EXPECT_EQ(vibratorHdiConnection_->RegisterVibratorPlugCallback(callback), VIBRATOR_ON_ERR);
}

/**
 * @tc.name: GetVibratorPlugCb_001
 * @tc.desc: Test GetVibratorPlugCb when the hdi connection is not initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetVibratorPlugCb_001, TestSize.Level2)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    ASSERT_EQ(vibratorHdiConnection_->iVibratorHdiConnection_, nullptr);
    EXPECT_EQ(vibratorHdiConnection_->GetVibratorPlugCb(), nullptr);
}

/**
 * @tc.name: GetVibratorPlugCb_002
 * @tc.desc: Test GetVibratorPlugCb when the hdi connection is initialized
 * @tc.type: FUNC
 */
HWTEST_F(VibratorHdiConnectionTest, GetVibratorPlugCb_002, TestSize.Level1)
{
    ASSERT_NE(vibratorHdiConnection_, nullptr);
    InjectMock();
    EXPECT_EQ(vibratorHdiConnection_->GetVibratorPlugCb(), nullptr);
}
}  // namespace Sensors
}  // namespace OHOS
