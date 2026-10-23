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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "accesstoken_kit.h"
#include "ipc_object_stub.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "i_vibrator_client.h"
#include "miscdevice_delayed_sp_singleton.h"
#include "miscdevice_service.h"
#include "sensors_errors.h"
#include "vibrator_client_stub.h"
#undef VIBRATOR_CLIENT_STUB_H
#include "vibrator_client_proxy.h"
#include "vibrator_hdi_connection.h"
#include "vibrator_infos.h"
#include "vibrator_test_common.h"

#undef LOG_TAG
#define LOG_TAG "VibratorServiceSendMsgTest"

namespace OHOS {
namespace Sensors {
using namespace testing;
using namespace testing::ext;

namespace {
auto g_miscdeviceService = MiscdeviceDelayedSpSingleton<MiscdeviceService>::GetInstance();
static MockHapToken* g_mock = nullptr;
uint64_t g_selfShellTokenId;
} // namespace

PermissionStateFull g_sendMsgTestState = {
    .grantFlags = {1},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .isGeneral = true,
    .permissionName = "ohos.permission.VIBRATE",
    .resDeviceID = {"local"}
};

HapPolicyParams g_sendMsgTestPolicyPrams = {
    .apl = APL_NORMAL,
    .domain = "test.domain",
    .permList = {},
    .permStateList = {g_sendMsgTestState}
};

HapInfoParams g_sendMsgTestInfoParms = {
    .bundleName = "VibratorServiceSendMsgTest",
    .userID = 1,
    .instIndex = 0,
    .appIDDesc = "VibratorServiceSendMsgTest"
};

/*
 * Mock implementation of IVibratorHdiConnection.
 * Used to replace VibratorHdiConnection::iVibratorHdiConnection_ so that
 * SendMsgToClient's external HDI calls (GetVibratorInfo / Stop /
 * GetVibratorCapacity / IsVibratorRunning) are intercepted.
 */
class MockIVibratorHdiConnection : public IVibratorHdiConnection {
public:
    MOCK_METHOD(int32_t, ConnectHdi, (), (override));
    MOCK_METHOD(int32_t, StartOnce, (const VibratorIdentifierIPC &, uint32_t), (override));
    MOCK_METHOD(int32_t, Start, (const VibratorIdentifierIPC &, const std::string &), (override));
#ifdef OHOS_BUILD_ENABLE_VIBRATOR_CUSTOM
    MOCK_METHOD(int32_t, EnableCompositeEffect, (const VibratorIdentifierIPC &, const HdfCompositeEffect &),
                (override));
    MOCK_METHOD(bool, IsVibratorRunning, (const VibratorIdentifierIPC &), (override));
#endif // OHOS_BUILD_ENABLE_VIBRATOR_CUSTOM
    MOCK_METHOD((std::optional<HdfEffectInfo>), GetEffectInfo,
                (const VibratorIdentifierIPC &, const std::string &), (override));
    MOCK_METHOD(int32_t, Stop, (const VibratorIdentifierIPC &, HdfVibratorMode), (override));
    MOCK_METHOD(int32_t, DestroyHdiConnection, (), (override));
    MOCK_METHOD(int32_t, GetDelayTime, (const VibratorIdentifierIPC &, int32_t, int32_t &), (override));
    MOCK_METHOD(int32_t, GetVibratorCapacity, (const VibratorIdentifierIPC &, VibratorCapacity &), (override));
    MOCK_METHOD(int32_t, PlayPattern, (const VibratorIdentifierIPC &, const VibratePattern &), (override));
    MOCK_METHOD(int32_t, PlayPatternBySessionId,
                (const VibratorIdentifierIPC &, uint32_t, const VibratePattern &), (override));
    MOCK_METHOD(int32_t, StartByIntensity,
                (const VibratorIdentifierIPC &, const std::string &, int32_t), (override));
    MOCK_METHOD(int32_t, GetVibratorInfo, (std::vector<HdfVibratorInfo> &), (override));
    MOCK_METHOD(int32_t, GetAllWaveInfo,
                (const VibratorIdentifierIPC &, std::vector<HdfWaveInformation> &), (override));
    MOCK_METHOD(int32_t, GetVibratorList,
                (const VibratorIdentifierIPC &, std::vector<HdfVibratorInfo> &), (override));
    MOCK_METHOD(int32_t, GetEffectInfo,
                (const VibratorIdentifierIPC &, const std::string &, HdfEffectInfo &), (override));
    MOCK_METHOD(int32_t, PlayPackageBySessionId,
                (const VibratorIdentifierIPC &, uint32_t, const VibratePackage &), (override));
    MOCK_METHOD(int32_t, StopVibrateBySessionId, (const VibratorIdentifierIPC &, uint32_t), (override));
    MOCK_METHOD(int32_t, RegisterVibratorPlugCallback, (DevicePlugCallback), (override));
    MOCK_METHOD(DevicePlugCallback, GetVibratorPlugCb, (), (override));
};

/*
 * Mock client stub implementing IVibratorClient so that iface_cast<IVibratorClient>
 * yields a valid proxy and ProcessPlugEvent can be intercepted.
 * OnRemoteRequest is NOT mocked: the real VibratorClientStub::OnRemoteRequest
 * dispatches to ProcessPlugEvent via virtual dispatch, so the MOCK_METHOD on
 * ProcessPlugEvent is triggered when the proxy sends the IPC request in-process.
 */
class MockVibratorClientStub : public VibratorClientStub {
public:
    MOCK_METHOD(int, ProcessPlugEvent, (int32_t eventCode, int32_t deviceId, int32_t vibratorCnt), (override));
};

class VibratorServiceSendMsgTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    // Saved original HDI connection pointer; restored in TearDown.
    std::unique_ptr<IVibratorHdiConnection> savedHdiConn_;
    // Owned mock injected into vibratorHdiConnection_.iVibratorHdiConnection_.
    MockIVibratorHdiConnection* mockHdi_ = nullptr;

    void ClearStaticState();
    void InjectMockHdi();
    void RestoreHdi();
};

void VibratorServiceSendMsgTest::ClearStaticState()
{
    // Clear static maps used by SendMsgToClient to ensure test isolation.
    MiscdeviceService::clientPidMap_.clear();
    MiscdeviceService::devicesManageMap_.clear();
}

void VibratorServiceSendMsgTest::InjectMockHdi()
{
    ASSERT_NE(g_miscdeviceService, nullptr);
    // -Dprivate=public exposes the unique_ptr member for swapping.
    savedHdiConn_ = std::move(g_miscdeviceService->vibratorHdiConnection_.iVibratorHdiConnection_);
    mockHdi_ = new MockIVibratorHdiConnection();
    g_miscdeviceService->vibratorHdiConnection_.iVibratorHdiConnection_.reset(mockHdi_);
}

void VibratorServiceSendMsgTest::RestoreHdi()
{
    if (g_miscdeviceService != nullptr) {
        g_miscdeviceService->vibratorHdiConnection_.iVibratorHdiConnection_ = std::move(savedHdiConn_);
    }
    mockHdi_ = nullptr;
}

void VibratorServiceSendMsgTest::SetUpTestCase()
{
    g_selfShellTokenId = GetSelfTokenID();
    VibratorTestCommon::SetTestEvironment(g_selfShellTokenId);
    std::vector<std::string> reqPerm;
    reqPerm.emplace_back("ohos.permission.VIBRATE");
    g_mock = new (std::nothrow) MockHapToken("VibratorServiceSendMsgTest", reqPerm, true);
    VibratorTestCommon::AllocAndGrantHapTokenByTest(g_sendMsgTestInfoParms, g_sendMsgTestPolicyPrams);
}

void VibratorServiceSendMsgTest::TearDownTestCase()
{
    if (g_mock != nullptr) {
        delete g_mock;
        g_mock = nullptr;
    }
    EXPECT_EQ(0, SetSelfTokenID(g_selfShellTokenId));
    VibratorTestCommon::ResetTestEvironment();
}

void VibratorServiceSendMsgTest::SetUp()
{
    ClearStaticState();
    InjectMockHdi();
}

void VibratorServiceSendMsgTest::TearDown()
{
    RestoreHdi();
    ClearStaticState();
}

/*
 * Helper: populate devicesManageMap_ with one device entry containing
 * a single vibrator.  Uses insert (not operator[]) because VibratorAllInfos
 * has no default constructor.
 */
static void PopulateDeviceMap(int32_t deviceId, int32_t vibratorId)
{
    std::vector<int> vibratorIds = {vibratorId};
    VibratorAllInfos allInfos(vibratorIds);
    VibratorInfoIPC infoIpc;
    infoIpc.deviceId = deviceId;
    infoIpc.vibratorId = vibratorId;
    infoIpc.isLocalVibrator = true;
    allInfos.baseInfo.push_back(infoIpc);
    MiscdeviceService::devicesManageMap_.insert({deviceId, allInfos});
}

/*
 * Helper: register a MockVibratorClientStub into clientPidMap_ so that
 * SendMsgToClient's client-notification loop can find and call it.
 */
static sptr<MockVibratorClientStub> RegisterMockClient(int32_t pid)
{
    auto stub = sptr<MockVibratorClientStub>(new MockVibratorClientStub());
    sptr<IRemoteObject> remoteObj = stub;
    MiscdeviceService::clientPidMap_.insert({remoteObj, pid});
    return stub;
}

// =========================================================================
// Branch: info.status == 0, device exists in devicesManageMap_
//         → StopVibratorService called for each baseInfo, then erase.
// =========================================================================
HWTEST_F(VibratorServiceSendMsgTest, SendMsgToClient_OfflineDeviceExists_001, TestSize.Level1)
{
    MISC_HILOGI("SendMsgToClient_OfflineDeviceExists_001 in");
    ASSERT_NE(g_miscdeviceService, nullptr);
    ASSERT_NE(mockHdi_, nullptr);

    const int32_t deviceId = 100;
    const int32_t vibratorId = 0;
    PopulateDeviceMap(deviceId, vibratorId);
    ASSERT_EQ(MiscdeviceService::devicesManageMap_.size(), 1u);

    // StopVibratorService internally calls CheckDeviceIdIsValid (which reads
    // devicesManageMap_ under lock) and then IsVibratorRunning / Stop on the
    // HDI connection.  Provide loose expectations so the calls are accepted.
#ifdef OHOS_BUILD_ENABLE_VIBRATOR_CUSTOM
    EXPECT_CALL(*mockHdi_, IsVibratorRunning(_)).WillRepeatedly(Return(false));
#endif
    EXPECT_CALL(*mockHdi_, Stop(_, _)).WillRepeatedly(Return(NO_ERROR));

    HdfVibratorPlugInfo info;
    info.deviceId = deviceId;
    info.deviceName = "test_device";
    info.vibratorCnt = 1;
    info.status = 0; // offline

    EXPECT_NO_FATAL_FAILURE(g_miscdeviceService->SendMsgToClient(info));

    // Device should have been removed from the map.
    EXPECT_EQ(MiscdeviceService::devicesManageMap_.count(deviceId), 0u);
}

// =========================================================================
// Branch: info.status == 0, device NOT in devicesManageMap_
//         → nothing to stop or erase; map unchanged.
// =========================================================================
HWTEST_F(VibratorServiceSendMsgTest, SendMsgToClient_OfflineDeviceNotExist_001, TestSize.Level1)
{
    MISC_HILOGI("SendMsgToClient_OfflineDeviceNotExist_001 in");
    ASSERT_NE(g_miscdeviceService, nullptr);
    ASSERT_NE(mockHdi_, nullptr);

    const int32_t deviceId = 200;
    // Do NOT populate devicesManageMap_ for this deviceId.
    ASSERT_EQ(MiscdeviceService::devicesManageMap_.size(), 0u);

    HdfVibratorPlugInfo info;
    info.deviceId = deviceId;
    info.deviceName = "absent_device";
    info.vibratorCnt = 0;
    info.status = 0; // offline

    EXPECT_NO_FATAL_FAILURE(g_miscdeviceService->SendMsgToClient(info));

    // Map should still be empty.
    EXPECT_EQ(MiscdeviceService::devicesManageMap_.size(), 0u);
}

// =========================================================================
// Branch: info.status != 0 (online), GetVibratorInfo returns NO_ERROR with
//         non-empty data → InsertVibratorInfo called.
//         InsertVibratorInfo internally calls GetVibratorCapacity and
//         GetAllWaveInfo.
// =========================================================================
HWTEST_F(VibratorServiceSendMsgTest, SendMsgToClient_OnlineGetVibratorInfoSuccess_001, TestSize.Level1)
{
    MISC_HILOGI("SendMsgToClient_OnlineGetVibratorInfoSuccess_001 in");
    ASSERT_NE(g_miscdeviceService, nullptr);
    ASSERT_NE(mockHdi_, nullptr);

    const int32_t deviceId = 300;

    // Prepare fake vibrator info returned by GetVibratorInfo.
    std::vector<HdfVibratorInfo> fakeInfos;
    HdfVibratorInfo hdiInfo;
    hdiInfo.deviceId = deviceId;
    hdiInfo.vibratorId = 0;
    hdiInfo.isSupportIntensity = true;
    hdiInfo.isSupportFrequency = false;
    hdiInfo.isSupportHdHaptic = false;
    hdiInfo.isLocal = 1;
    fakeInfos.push_back(hdiInfo);

    EXPECT_CALL(*mockHdi_, GetVibratorInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(fakeInfos), Return(NO_ERROR)));

    EXPECT_CALL(*mockHdi_, GetVibratorCapacity(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(VibratorCapacity{}), Return(NO_ERROR)));

    EXPECT_CALL(*mockHdi_, GetAllWaveInfo(_, _))
        .WillOnce(Return(NO_ERROR));

    HdfVibratorPlugInfo info;
    info.deviceId = deviceId;
    info.deviceName = "online_device";
    info.vibratorCnt = 1;
    info.status = 1; // online

    EXPECT_NO_FATAL_FAILURE(g_miscdeviceService->SendMsgToClient(info));

    // InsertVibratorInfo should have added the device to the map.
    EXPECT_EQ(MiscdeviceService::devicesManageMap_.count(deviceId), 1u);
}

// =========================================================================
// Branch: info.status != 0 (online), GetVibratorInfo returns error
//         → InsertVibratorInfo still called (with empty vector).
// =========================================================================
HWTEST_F(VibratorServiceSendMsgTest, SendMsgToClient_OnlineGetVibratorInfoError_001, TestSize.Level1)
{
    MISC_HILOGI("SendMsgToClient_OnlineGetVibratorInfoError_001 in");
    ASSERT_NE(g_miscdeviceService, nullptr);
    ASSERT_NE(mockHdi_, nullptr);

    const int32_t deviceId = 400;

    // GetVibratorInfo returns error.
    EXPECT_CALL(*mockHdi_, GetVibratorInfo(_))
        .WillOnce(Return(ERROR));

    // InsertVibratorInfo with empty vector will return NO_ERROR early
    // (infos.empty() path), so GetVibratorCapacity / GetAllWaveInfo are
    // NOT called.  We still set expectations to be safe.
    EXPECT_CALL(*mockHdi_, GetVibratorCapacity(_, _)).Times(AnyNumber());
    EXPECT_CALL(*mockHdi_, GetAllWaveInfo(_, _)).Times(AnyNumber());

    HdfVibratorPlugInfo info;
    info.deviceId = deviceId;
    info.deviceName = "error_device";
    info.vibratorCnt = 0;
    info.status = 1; // online

    EXPECT_NO_FATAL_FAILURE(g_miscdeviceService->SendMsgToClient(info));

    // With empty vibratorInfo, InsertVibratorInfo returns NO_ERROR without
    // inserting anything into the map.
    EXPECT_EQ(MiscdeviceService::devicesManageMap_.count(deviceId), 0u);
}

// =========================================================================
// Branch: info.status != 0 (online), GetVibratorInfo returns NO_ERROR but
//         the vector is empty → InsertVibratorInfo called with empty vector.
// =========================================================================
HWTEST_F(VibratorServiceSendMsgTest, SendMsgToClient_OnlineGetVibratorInfoEmpty_001, TestSize.Level1)
{
    MISC_HILOGI("SendMsgToClient_OnlineGetVibratorInfoEmpty_001 in");
    ASSERT_NE(g_miscdeviceService, nullptr);
    ASSERT_NE(mockHdi_, nullptr);

    const int32_t deviceId = 500;

    std::vector<HdfVibratorInfo> emptyInfos;
    EXPECT_CALL(*mockHdi_, GetVibratorInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(emptyInfos), Return(NO_ERROR)));

    EXPECT_CALL(*mockHdi_, GetVibratorCapacity(_, _)).Times(AnyNumber());
    EXPECT_CALL(*mockHdi_, GetAllWaveInfo(_, _)).Times(AnyNumber());

    HdfVibratorPlugInfo info;
    info.deviceId = deviceId;
    info.deviceName = "empty_device";
    info.vibratorCnt = 0;
    info.status = 1; // online

    EXPECT_NO_FATAL_FAILURE(g_miscdeviceService->SendMsgToClient(info));

    EXPECT_EQ(MiscdeviceService::devicesManageMap_.count(deviceId), 0u);
}

// =========================================================================
// Branch: clientPidMap_ is empty → the notification loop body never executes.
// =========================================================================
HWTEST_F(VibratorServiceSendMsgTest, SendMsgToClient_NoClients_001, TestSize.Level1)
{
    MISC_HILOGI("SendMsgToClient_NoClients_001 in");
    ASSERT_NE(g_miscdeviceService, nullptr);
    ASSERT_NE(mockHdi_, nullptr);

    const int32_t deviceId = 600;
    ASSERT_EQ(MiscdeviceService::clientPidMap_.size(), 0u);

    // Set up for online path with empty info.
    EXPECT_CALL(*mockHdi_, GetVibratorInfo(_))
        .WillOnce(Return(ERROR));
    EXPECT_CALL(*mockHdi_, GetVibratorCapacity(_, _)).Times(AnyNumber());
    EXPECT_CALL(*mockHdi_, GetAllWaveInfo(_, _)).Times(AnyNumber());

    HdfVibratorPlugInfo info;
    info.deviceId = deviceId;
    info.deviceName = "no_clients_device";
    info.vibratorCnt = 0;
    info.status = 1;

    EXPECT_NO_FATAL_FAILURE(g_miscdeviceService->SendMsgToClient(info));

    // No crash, no clients notified.
    EXPECT_EQ(MiscdeviceService::clientPidMap_.size(), 0u);
}

// =========================================================================
// Branch: clientPidMap_ has a registered client with valid proxy
//         → clientProxy->ProcessPlugEvent is called.
// =========================================================================
HWTEST_F(VibratorServiceSendMsgTest, SendMsgToClient_WithClient_001, TestSize.Level1)
{
    MISC_HILOGI("SendMsgToClient_WithClient_001 in");
    ASSERT_NE(g_miscdeviceService, nullptr);
    ASSERT_NE(mockHdi_, nullptr);

    const int32_t deviceId = 700;

    // Register a mock client stub.
    auto mockClient = RegisterMockClient(12345);
    ASSERT_EQ(MiscdeviceService::clientPidMap_.size(), 1u);

    // Set up for offline path (status == 0) with no device in map, so
    // we focus on the client notification path.
    EXPECT_CALL(*mockHdi_, GetVibratorInfo(_)).Times(AnyNumber());
    EXPECT_CALL(*mockHdi_, GetVibratorCapacity(_, _)).Times(AnyNumber());
    EXPECT_CALL(*mockHdi_, GetAllWaveInfo(_, _)).Times(AnyNumber());

    const int32_t expectedEventCode = 1;
    const int32_t expectedVibratorCnt = 2;

    EXPECT_CALL(*mockClient, ProcessPlugEvent(expectedEventCode, deviceId, expectedVibratorCnt))
        .WillOnce(Return(NO_ERROR));

    HdfVibratorPlugInfo info;
    info.deviceId = deviceId;
    info.deviceName = "client_device";
    info.vibratorCnt = expectedVibratorCnt;
    info.status = expectedEventCode; // online

    EXPECT_NO_FATAL_FAILURE(g_miscdeviceService->SendMsgToClient(info));

    // Verify the mock was actually called.
    Mock::VerifyAndClearExpectations(mockClient.GetRefPtr());
}

// =========================================================================
// Branch: clientPidMap_ has a registered client, status == 0 (offline)
//         → ProcessPlugEvent called with eventCode = 0.
// =========================================================================
HWTEST_F(VibratorServiceSendMsgTest, SendMsgToClient_WithClientOffline_001, TestSize.Level1)
{
    MISC_HILOGI("SendMsgToClient_WithClientOffline_001 in");
    ASSERT_NE(g_miscdeviceService, nullptr);
    ASSERT_NE(mockHdi_, nullptr);

    const int32_t deviceId = 800;

    auto mockClient = RegisterMockClient(67890);
    ASSERT_EQ(MiscdeviceService::clientPidMap_.size(), 1u);

    EXPECT_CALL(*mockHdi_, GetVibratorInfo(_)).Times(AnyNumber());
    EXPECT_CALL(*mockHdi_, GetVibratorCapacity(_, _)).Times(AnyNumber());
    EXPECT_CALL(*mockHdi_, GetAllWaveInfo(_, _)).Times(AnyNumber());
    EXPECT_CALL(*mockHdi_, Stop(_, _)).Times(AnyNumber());
#ifdef OHOS_BUILD_ENABLE_VIBRATOR_CUSTOM
    EXPECT_CALL(*mockHdi_, IsVibratorRunning(_)).Times(AnyNumber());
#endif

    EXPECT_CALL(*mockClient, ProcessPlugEvent(0, deviceId, _))
        .WillOnce(Return(NO_ERROR));

    HdfVibratorPlugInfo info;
    info.deviceId = deviceId;
    info.deviceName = "offline_client_device";
    info.vibratorCnt = 1;
    info.status = 0; // offline

    EXPECT_NO_FATAL_FAILURE(g_miscdeviceService->SendMsgToClient(info));

    Mock::VerifyAndClearExpectations(mockClient.GetRefPtr());
}

// =========================================================================
// Branch: Multiple clients in clientPidMap_ → all notified.
// =========================================================================
HWTEST_F(VibratorServiceSendMsgTest, SendMsgToClient_MultipleClients_001, TestSize.Level1)
{
    MISC_HILOGI("SendMsgToClient_MultipleClients_001 in");
    ASSERT_NE(g_miscdeviceService, nullptr);
    ASSERT_NE(mockHdi_, nullptr);

    const int32_t deviceId = 900;

    auto mockClient1 = RegisterMockClient(111);
    auto mockClient2 = RegisterMockClient(222);
    auto mockClient3 = RegisterMockClient(333);
    ASSERT_EQ(MiscdeviceService::clientPidMap_.size(), 3u);

    EXPECT_CALL(*mockHdi_, GetVibratorInfo(_)).Times(AnyNumber());
    EXPECT_CALL(*mockHdi_, GetVibratorCapacity(_, _)).Times(AnyNumber());
    EXPECT_CALL(*mockHdi_, GetAllWaveInfo(_, _)).Times(AnyNumber());

    EXPECT_CALL(*mockClient1, ProcessPlugEvent(1, deviceId, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*mockClient2, ProcessPlugEvent(1, deviceId, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*mockClient3, ProcessPlugEvent(1, deviceId, _)).WillOnce(Return(NO_ERROR));

    HdfVibratorPlugInfo info;
    info.deviceId = deviceId;
    info.deviceName = "multi_client_device";
    info.vibratorCnt = 3;
    info.status = 1; // online

    EXPECT_NO_FATAL_FAILURE(g_miscdeviceService->SendMsgToClient(info));

    Mock::VerifyAndClearExpectations(mockClient1.GetRefPtr());
    Mock::VerifyAndClearExpectations(mockClient2.GetRefPtr());
    Mock::VerifyAndClearExpectations(mockClient3.GetRefPtr());
}

// =========================================================================
// Branch: Offline device with multiple vibrators in baseInfo
//         → StopVibratorService called for each vibrator.
// =========================================================================
HWTEST_F(VibratorServiceSendMsgTest, SendMsgToClient_OfflineMultipleVibrators_001, TestSize.Level1)
{
    MISC_HILOGI("SendMsgToClient_OfflineMultipleVibrators_001 in");
    ASSERT_NE(g_miscdeviceService, nullptr);
    ASSERT_NE(mockHdi_, nullptr);

    const int32_t deviceId = 1000;
    // Populate with two vibrators.
    std::vector<int> vibratorIds = {0, 1};
    VibratorAllInfos allInfos(vibratorIds);
    for (int vid : vibratorIds) {
        VibratorInfoIPC infoIpc;
        infoIpc.deviceId = deviceId;
        infoIpc.vibratorId = vid;
        infoIpc.isLocalVibrator = true;
        allInfos.baseInfo.push_back(infoIpc);
    }
    MiscdeviceService::devicesManageMap_.insert({deviceId, allInfos});
    ASSERT_EQ(MiscdeviceService::devicesManageMap_.size(), 1u);
    auto it = MiscdeviceService::devicesManageMap_.find(deviceId);
    ASSERT_NE(it, MiscdeviceService::devicesManageMap_.end());
    ASSERT_EQ(it->second.baseInfo.size(), 2u);

#ifdef OHOS_BUILD_ENABLE_VIBRATOR_CUSTOM
    EXPECT_CALL(*mockHdi_, IsVibratorRunning(_)).WillRepeatedly(Return(false));
#endif
    EXPECT_CALL(*mockHdi_, Stop(_, _)).WillRepeatedly(Return(NO_ERROR));

    HdfVibratorPlugInfo info;
    info.deviceId = deviceId;
    info.deviceName = "multi_vibrator_device";
    info.vibratorCnt = 2;
    info.status = 0; // offline

    EXPECT_NO_FATAL_FAILURE(g_miscdeviceService->SendMsgToClient(info));

    // Device should have been removed.
    EXPECT_EQ(MiscdeviceService::devicesManageMap_.count(deviceId), 0u);
}

// =========================================================================
// Branch: Online device, InsertVibratorInfo fails (device already in map)
//         → error logged but no crash.
// =========================================================================
HWTEST_F(VibratorServiceSendMsgTest, SendMsgToClient_OnlineInsertDuplicate_001, TestSize.Level1)
{
    MISC_HILOGI("SendMsgToClient_OnlineInsertDuplicate_001 in");
    ASSERT_NE(g_miscdeviceService, nullptr);
    ASSERT_NE(mockHdi_, nullptr);

    const int32_t deviceId = 1100;

    // Pre-populate the map so InsertVibratorInfo finds a duplicate.
    PopulateDeviceMap(deviceId, 0);
    ASSERT_EQ(MiscdeviceService::devicesManageMap_.size(), 1u);

    // GetVibratorInfo returns info for the same deviceId.
    std::vector<HdfVibratorInfo> fakeInfos;
    HdfVibratorInfo hdiInfo;
    hdiInfo.deviceId = deviceId;
    hdiInfo.vibratorId = 0;
    hdiInfo.isSupportIntensity = true;
    hdiInfo.isSupportFrequency = false;
    hdiInfo.isSupportHdHaptic = false;
    hdiInfo.isLocal = 1;
    fakeInfos.push_back(hdiInfo);

    EXPECT_CALL(*mockHdi_, GetVibratorInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(fakeInfos), Return(NO_ERROR)));

    // InsertVibratorInfo will skip because deviceId already in map,
    // so GetVibratorCapacity / GetAllWaveInfo may or may not be called.
    EXPECT_CALL(*mockHdi_, GetVibratorCapacity(_, _)).Times(AnyNumber());
    EXPECT_CALL(*mockHdi_, GetAllWaveInfo(_, _)).Times(AnyNumber());

    HdfVibratorPlugInfo info;
    info.deviceId = deviceId;
    info.deviceName = "duplicate_device";
    info.vibratorCnt = 1;
    info.status = 1; // online

    EXPECT_NO_FATAL_FAILURE(g_miscdeviceService->SendMsgToClient(info));

    // Map should still contain exactly one entry (no duplicate inserted).
    EXPECT_EQ(MiscdeviceService::devicesManageMap_.size(), 1u);
}
} // namespace Sensors
} // namespace OHOS
