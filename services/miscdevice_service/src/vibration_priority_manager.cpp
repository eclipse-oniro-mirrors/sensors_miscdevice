/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "vibration_priority_manager.h"

#include <tokenid_kit.h>
#ifdef OHOS_BUILD_ENABLE_DO_NOT_DISTURB
#include <regex>
#endif // OHOS_BUILD_ENABLE_DO_NOT_DISTURB

#include "accesstoken_kit.h"
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
#include "hisysevent.h"
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#ifdef OHOS_BUILD_ENABLE_VIBRATOR_INPUT_METHOD
#include "bundle_mgr_client.h"
#include "os_account_manager.h"
#endif // OHOS_BUILD_ENABLE_VIBRATOR_INPUT_METHOD

#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "VibrationPriorityManager"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;
#ifdef OHOS_BUILD_ENABLE_DO_NOT_DISTURB
const int32_t INVALID_USERID = 100;
const int32_t WHITE_LIST_MAX_COUNT = 100;
static int32_t g_currentUserId = INVALID_USERID;
static std::mutex g_settingMutex;
#endif // OHOS_BUILD_ENABLE_DO_NOT_DISTURB
namespace {
const std::string SETTING_COLUMN_KEYWORD = "KEYWORD";
const std::string SETTING_COLUMN_VALUE = "VALUE";
const std::string SETTING_FEEDBACK_KEY = "physic_navi_haptic_feedback_enabled";
const std::string SETTING_RINGER_MODE_KEY = "ringer_mode";
const std::string SETTING_URI_PROXY = "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
const std::string SCENEBOARD_BUNDLENAME = "com.ohos.sceneboard";
constexpr const char *SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
#ifdef OHOS_BUILD_ENABLE_DO_NOT_DISTURB
const std::string USER_SETTING_SECURE_URI_PROXY = "datashare:///com.ohos.settingsdata/entry/settingsdata/"
                                                  "USER_SETTINGSDATA_SECURE_##USERID##?Proxy=true";
const std::string DO_NOT_DISTURB_SWITCH = "focus_mode_enable";
const std::string DO_NOT_DISTURB_WHITE_LIST = "intelligent_scene_notification_white_list";
const std::string WHITE_LIST_KEY_BUNDLE = "bundle";
const std::string WHITE_LIST_KEY_UID = "uid";
constexpr const char *USERID_REPLACE = "##USERID##";
#endif // OHOS_BUILD_ENABLE_DO_NOT_DISTURB
#ifdef OHOS_BUILD_ENABLE_VIBRATOR_CROWN
const std::string SETTING_CROWN_FEEDBACK_KEY = "watch_crown_feedback_enabled";
const std::string SETTING_VIBRATE_INTENSITY_KEY = "vibration_intensity_index";
#endif
constexpr int32_t DECEM_BASE = 10;
constexpr int32_t DATA_SHARE_READY = 0;
constexpr int32_t DATA_SHARE_NOT_READY = 1055;
}  // namespace

VibrationPriorityManager::VibrationPriorityManager() {}

VibrationPriorityManager::~VibrationPriorityManager()
{
    remoteObj_ = nullptr;
    if (UnregisterObserver(observer_) != ERR_OK) {
        MISC_HILOGE("UnregisterObserver failed");
    }
#ifdef OHOS_BUILD_ENABLE_DO_NOT_DISTURB
    if (UnregisterUserObserver() != ERR_OK) {
        MISC_HILOGE("UnregisterUserObserver failed");
    }
#endif // OHOS_BUILD_ENABLE_DO_NOT_DISTURB
}

bool VibrationPriorityManager::Init()
{
    auto sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        MISC_HILOGE("sm cannot be nullptr");
        return false;
    }
    remoteObj_ = sm->GetSystemAbility(MISCDEVICE_SERVICE_ABILITY_ID);
    if (remoteObj_ == nullptr) {
        MISC_HILOGE("GetSystemAbility return nullptr");
        return false;
    }
    MiscDeviceObserver::UpdateFunc updateFunc = [&]() {
        int32_t feedback = miscFeedback_;
        if (GetIntValue(SETTING_FEEDBACK_KEY, feedback) != ERR_OK) {
            MISC_HILOGE("Get feedback failed");
        }
        miscFeedback_ = feedback;
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
        HiSysEventWrite(HiSysEvent::Domain::MISCDEVICE, "SWITCHES_TOGGLE",
            HiSysEvent::EventType::BEHAVIOR, "SWITCH_TYPE", "feedback", "STATUS", feedback);
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
        MISC_HILOGI("feedback:%{public}d", feedback);
        int32_t ringerMode = miscAudioRingerMode_;
        if (GetIntValue(SETTING_RINGER_MODE_KEY, ringerMode) != ERR_OK) {
            MISC_HILOGE("Get ringerMode failed");
        }
        miscAudioRingerMode_ = ringerMode;
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
        HiSysEventWrite(HiSysEvent::Domain::MISCDEVICE, "SWITCHES_TOGGLE",
            HiSysEvent::EventType::BEHAVIOR, "SWITCH_TYPE", "ringerMode", "STATUS", ringerMode);
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
        MISC_HILOGI("ringerMode:%{public}d", ringerMode);
#ifdef OHOS_BUILD_ENABLE_VIBRATOR_CROWN
        MiscCrownIntensityFeedbackInit();
#endif
    };
    auto observer_ = CreateObserver(updateFunc);
    if (observer_ == nullptr) {
        MISC_HILOGE("observer is null");
        return false;
    }
    if (RegisterObserver(observer_) != ERR_OK) {
        MISC_HILOGE("RegisterObserver failed");
        return false;
    }
#ifdef OHOS_BUILD_ENABLE_DO_NOT_DISTURB
    ReregisterCurrentUserObserver();
#endif // OHOS_BUILD_ENABLE_DO_NOT_DISTURB
    return true;
}

#ifdef OHOS_BUILD_ENABLE_DO_NOT_DISTURB
void VibrationPriorityManager::InitDoNotDisturbData()
{
    int32_t switchTemp = doNotDisturbSwitch_;
    if (GetDoNotDisturbIntValue(DO_NOT_DISTURB_SWITCH, switchTemp) != ERR_OK) {
        doNotDisturbSwitch_ = DONOTDISTURB_SWITCH_OFF;
        MISC_HILOGE("Get doNotDisturbSwitch failed");
    } else {
        doNotDisturbSwitch_ = switchTemp;
        MISC_HILOGI("doNotDisturbSwitch:%{public}d", switchTemp);
    }
    std::lock_guard<std::mutex> whiteListLock(whiteListMutex_);
    if (doNotDisturbSwitch_ == DONOTDISTURB_SWITCH_ON) {
        std::vector<WhiteListAppInfo> whiteListTemp;
        int32_t whiteListRet = GetWhiteListValue(DO_NOT_DISTURB_WHITE_LIST, whiteListTemp);
        if (whiteListRet != ERR_OK) {
            doNotDisturbSwitch_ = DONOTDISTURB_SWITCH_OFF;
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
            HiSysEventWrite(HiSysEvent::Domain::MISCDEVICE, "DATASHARE_EXCEPTION", HiSysEvent::EventType::FAULT,
                "PKG_NAME", "GetWhiteListValue", "ERROR_CODE", whiteListRet);
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
            MISC_HILOGE("Get doNotDisturbWhiteList failed");
        } else {
            int32_t whiteListSize = static_cast<int32_t>(whiteListTemp.size());
            if (whiteListSize == 0) {
                doNotDisturbWhiteList_.clear();
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
                HiSysEventWrite(HiSysEvent::Domain::MISCDEVICE, "WHITELIST_COUNT_EXCEPTION",
                    HiSysEvent::EventType::FAULT, "PKG_NAME", "InitDoNotDisturbData", "ACTUAL_COUNT", whiteListSize);
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
            } else if (whiteListSize > WHITE_LIST_MAX_COUNT) {
                doNotDisturbWhiteList_.clear();
                doNotDisturbWhiteList_.assign(whiteListTemp.begin(), whiteListTemp.begin() + WHITE_LIST_MAX_COUNT);
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
                HiSysEventWrite(HiSysEvent::Domain::MISCDEVICE, "WHITELIST_COUNT_EXCEPTION",
                    HiSysEvent::EventType::FAULT, "PKG_NAME", "InitDoNotDisturbData", "ACTUAL_COUNT", whiteListSize);
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
                MISC_HILOGW("whiteListTemp size:%{public}d", whiteListSize);
            } else {
                doNotDisturbWhiteList_ = whiteListTemp;
            }
            MISC_HILOGI("doNotDisturbWhiteList size:%{public}d", static_cast<int32_t>(doNotDisturbWhiteList_.size()));
        }
    } else if (doNotDisturbSwitch_ == DONOTDISTURB_SWITCH_OFF) {
        doNotDisturbWhiteList_.clear();
        MISC_HILOGD("clear doNotDisturbWhiteList_, DoNotDisturbSwitch:%{public}d",
            static_cast<int32_t>(doNotDisturbSwitch_));
    } else {
        MISC_HILOGW("DoNotDisturbSwitch invalid, DoNotDisturbSwitch:%{public}d",
            static_cast<int32_t>(doNotDisturbSwitch_));
    }
}

void VibrationPriorityManager::ReregisterCurrentUserObserver()
{
    UnregisterUserObserver();
    UpdateCurrentUserId();
    RegisterUserObserver();
}

void VibrationPriorityManager::UpdateCurrentUserId()
{
    std::lock_guard<std::mutex> lock(g_settingMutex);
    std::vector<int32_t> activeUserIds;
    int retId = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeUserIds);
    if (retId != 0) {
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
        HiSysEventWrite(HiSysEvent::Domain::MISCDEVICE, "USER_SWITCHED_EXCEPTION", HiSysEvent::EventType::FAULT,
            "PKG_NAME", "QueryActiveOsAccountIds", "ERROR_CODE", retId);
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
        MISC_HILOGE("QueryActiveOsAccountIds failed %{public}d", retId);
        return;
    }
    if (activeUserIds.empty()) {
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
        HiSysEventWrite(HiSysEvent::Domain::MISCDEVICE, "USER_SWITCHED_EXCEPTION", HiSysEvent::EventType::FAULT,
            "PKG_NAME", "activeUserIdsEmpty", "ERROR_CODE", ERROR);
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
        MISC_HILOGE("activeUserIds empty");
        return;
    }
    g_currentUserId = activeUserIds[0];
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
    HiSysEventWrite(HiSysEvent::Domain::MISCDEVICE, "USER_SWITCHED_EXCEPTION", HiSysEvent::EventType::FAULT,
        "PKG_NAME", "UpdateCurrentUserId", "ERROR_CODE", ERR_OK);
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
    MISC_HILOGI("g_currentUserId is %{public}d", g_currentUserId);
}

int32_t VibrationPriorityManager::RegisterUserObserver()
{
    MISC_HILOGI("RegisterUserObserver start");
    std::lock_guard<std::mutex> currentUserObserverLock(currentUserObserverMutex_);
    MiscDeviceObserver::UpdateFunc updateFunc = [&]() { InitDoNotDisturbData(); };
    currentUserObserver_ = CreateObserver(updateFunc);
    if (currentUserObserver_ == nullptr) {
        MISC_HILOGE("currentUserObserver_ is null");
        return MISC_NO_INIT_ERR;
    }
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto doNotDisturbHelper =
        CreateDataShareHelper(ReplaceUserIdForUri(USER_SETTING_SECURE_URI_PROXY, g_currentUserId));
    if (doNotDisturbHelper == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
        HiSysEventWrite(HiSysEvent::Domain::MISCDEVICE, "DATASHARE_EXCEPTION", HiSysEvent::EventType::FAULT,
            "PKG_NAME", "RegisterUserObserver", "ERROR_CODE", MISC_NO_INIT_ERR);
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
        MISC_HILOGE("doNotDisturbHelper is nullptr");
        return MISC_NO_INIT_ERR;
    }
    auto doNotDisturbSwitch = DoNotDisturbAssembleUri(DO_NOT_DISTURB_SWITCH);
    doNotDisturbHelper->RegisterObserver(doNotDisturbSwitch, currentUserObserver_);
    doNotDisturbHelper->NotifyChange(doNotDisturbSwitch);
    auto doNotDisturbWhiteList = DoNotDisturbAssembleUri(DO_NOT_DISTURB_WHITE_LIST);
    doNotDisturbHelper->RegisterObserver(doNotDisturbWhiteList, currentUserObserver_);
    doNotDisturbHelper->NotifyChange(doNotDisturbWhiteList);
    std::thread execCb(VibrationPriorityManager::ExecRegisterCb, currentUserObserver_);
    execCb.detach();
    ReleaseDataShareHelper(doNotDisturbHelper);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    MISC_HILOGI("Succeed to RegisterUserObserver of uri");
    return ERR_OK;
}

int32_t VibrationPriorityManager::UnregisterUserObserver()
{
    MISC_HILOGI("UnregisterUserObserver start");
    if (currentUserObserver_ == nullptr) {
        MISC_HILOGE("currentUserObserver_ is nullptr");
        return MISC_NO_INIT_ERR;
    }
    std::lock_guard<std::mutex> currentUserObserverLock(currentUserObserverMutex_);
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto doNotDisturbHelper =
        CreateDataShareHelper(ReplaceUserIdForUri(USER_SETTING_SECURE_URI_PROXY, g_currentUserId));
    if (doNotDisturbHelper == nullptr) {
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
        HiSysEventWrite(HiSysEvent::Domain::MISCDEVICE, "DATASHARE_EXCEPTION", HiSysEvent::EventType::FAULT,
            "PKG_NAME", "UnregisterUserObserver", "ERROR_CODE", MISC_NO_INIT_ERR);
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        currentUserObserver_ = nullptr;
        MISC_HILOGE("doNotDisturbHelper is nullptr");
        return MISC_NO_INIT_ERR;
    }
    auto doNotDisturbSwitch = DoNotDisturbAssembleUri(DO_NOT_DISTURB_SWITCH);
    doNotDisturbHelper->UnregisterObserver(doNotDisturbSwitch, currentUserObserver_);
    auto doNotDisturbWhiteList = DoNotDisturbAssembleUri(DO_NOT_DISTURB_WHITE_LIST);
    doNotDisturbHelper->UnregisterObserver(doNotDisturbWhiteList, currentUserObserver_);
    ReleaseDataShareHelper(doNotDisturbHelper);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    currentUserObserver_ = nullptr;
    MISC_HILOGI("Succeed to UnregisterUserObserver observer");
    return ERR_OK;
}

std::string VibrationPriorityManager::ReplaceUserIdForUri(std::string uri, int32_t userId)
{
    std::string tempUri = uri;
    std::regex pattern(USERID_REPLACE);
    return std::regex_replace(tempUri, pattern, std::to_string(userId));
}

Uri VibrationPriorityManager::DoNotDisturbAssembleUri(const std::string &key)
{
    Uri uri(ReplaceUserIdForUri(USER_SETTING_SECURE_URI_PROXY, g_currentUserId) + "&key=" + key);
    return uri;
}

int32_t VibrationPriorityManager::GetDoNotDisturbStringValue(const std::string &key, std::string &value)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto helper = CreateDataShareHelper(ReplaceUserIdForUri(USER_SETTING_SECURE_URI_PROXY, g_currentUserId));
    if (helper == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        MISC_HILOGE("helper is nullptr");
        return MISC_NO_INIT_ERR;
    }
    std::vector<std::string> columns = { SETTING_COLUMN_VALUE };
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(DoNotDisturbAssembleUri(key));
    auto resultSet = helper->Query(uri, predicates, columns);
    ReleaseDataShareHelper(helper);
    if (resultSet == nullptr) {
        MISC_HILOGE("resultSet is nullptr");
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return MISC_INVALID_OPERATION_ERR;
    }
    int32_t count;
    resultSet->GetRowCount(count);
    if (count == 0) {
        MISC_HILOGW("Not found value, key:%{public}s, count:%{public}d", key.c_str(), count);
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return MISC_NAME_NOT_FOUND_ERR;
    }
    const int32_t index = 0;
    resultSet->GoToRow(index);
    int32_t ret = resultSet->GetString(index, value);
    if (ret != ERR_OK) {
        MISC_HILOGW("GetString failed, ret:%{public}d", ret);
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERROR;
    }
    resultSet->Close();
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    return ERR_OK;
}

int32_t VibrationPriorityManager::GetDoNotDisturbIntValue(const std::string &key, int32_t &value)
{
    int64_t valueLong;
    int32_t ret = GetDoNotDisturbLongValue(key, valueLong);
    if (ret != ERR_OK) {
        MISC_HILOGE("GetDoNotDisturbLongValue failed, ret:%{public}d", ret);
        return ret;
    }
    value = static_cast<int32_t>(valueLong);
    return ERR_OK;
}

int32_t VibrationPriorityManager::GetDoNotDisturbLongValue(const std::string &key, int64_t &value)
{
    std::string valueStr;
    int32_t ret = GetDoNotDisturbStringValue(key, valueStr);
    if (ret != ERR_OK) {
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
        HiSysEventWrite(HiSysEvent::Domain::MISCDEVICE, "DATASHARE_EXCEPTION", HiSysEvent::EventType::FAULT,
            "PKG_NAME", "GetDoNotDisturbStringValue", "ERROR_CODE", ret);
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
        MISC_HILOGE("GetDoNotDisturbStringValue failed, ret:%{public}d", ret);
        return ret;
    }
    value = static_cast<int64_t>(strtoll(valueStr.c_str(), nullptr, DECEM_BASE));
    return ERR_OK;
}

int32_t VibrationPriorityManager::GetWhiteListValue(const std::string &key, std::vector<WhiteListAppInfo> &value)
{
    std::string valueStr;
    int32_t ret = GetDoNotDisturbStringValue(key, valueStr);
    if (ret != ERR_OK) {
        MISC_HILOGE("GetDoNotDisturbStringValue failed, ret:%{public}d", ret);
        return ret;
    }
    if (valueStr.empty()) {
        MISC_HILOGE("String value empty");
        return ERROR;
    }
    cJSON *jsonArray = nullptr;
    jsonArray = cJSON_Parse(valueStr.c_str());
    if (!cJSON_IsArray(jsonArray)) {
        MISC_HILOGE("The string value is not array");
        DeleteCJSONValue(jsonArray);
        return ERROR;
    }
    int32_t size = cJSON_GetArraySize(jsonArray);
    for (int32_t i = 0; i < size; ++i) {
        cJSON *valJson = cJSON_GetArrayItem(jsonArray, i);
        if (!cJSON_IsObject(valJson)) {
            MISC_HILOGE("The json is not object");
            DeleteCJSONValue(jsonArray);
            return ERROR;
        }
        if (!cJSON_HasObjectItem(valJson, WHITE_LIST_KEY_BUNDLE.c_str()) ||
            !cJSON_HasObjectItem(valJson, WHITE_LIST_KEY_UID.c_str())) {
            MISC_HILOGE("The json is not bundle or uid");
            DeleteCJSONValue(jsonArray);
            return ERROR;
        }
        WhiteListAppInfo whiteListAppInfo;
        cJSON *valBundle = cJSON_GetObjectItem(valJson, WHITE_LIST_KEY_BUNDLE.c_str());
        cJSON *valUid = cJSON_GetObjectItem(valJson, WHITE_LIST_KEY_UID.c_str());
        if ((!cJSON_IsString(valBundle)) || !cJSON_IsNumber(valUid)) {
            MISC_HILOGE("The value of index %{public}d is not match", i);
            DeleteCJSONValue(jsonArray);
            return ERROR;
        }
        whiteListAppInfo.bundle = valBundle->valuestring;
        whiteListAppInfo.uid = static_cast<int64_t>(valUid->valueint);
        value.push_back(whiteListAppInfo);
    }
    DeleteCJSONValue(jsonArray);
    return ERR_OK;
}

void VibrationPriorityManager::DeleteCJSONValue(cJSON *jsonValue)
{
    if (jsonValue != nullptr) {
        cJSON_Delete(jsonValue);
    }
}

bool VibrationPriorityManager::IgnoreAppVibrations(const VibrateInfo &vibrateInfo)
{
    if (vibrateInfo.usage != USAGE_NOTIFICATION && vibrateInfo.usage != USAGE_RING) {
        MISC_HILOGD("Vibration is not ignored , usage:%{public}d", vibrateInfo.usage);
        return false;
    }
    if (doNotDisturbSwitch_ != DONOTDISTURB_SWITCH_ON) {
        MISC_HILOGD("DoNotDisturbSwitch is off");
        return false;
    }
    std::lock_guard<std::mutex> whiteListLock(whiteListMutex_);
    for (const WhiteListAppInfo &whiteListAppInfo : doNotDisturbWhiteList_) {
        if (vibrateInfo.packageName == whiteListAppInfo.bundle) {
            MISC_HILOGD("Not ignore app vibration, the app is on the whitelist, bundleName::%{public}s",
                vibrateInfo.packageName.c_str());
            return false;
        }
    }
    MISC_HILOGI("Ignore app vibration, bundleName::%{public}s", vibrateInfo.packageName.c_str());
    return true;
}
#endif // OHOS_BUILD_ENABLE_DO_NOT_DISTURB

#ifdef OHOS_BUILD_ENABLE_VIBRATOR_CROWN
void VibrationPriorityManager::MiscCrownIntensityFeedbackInit(void)
{
    int32_t crownfeedback = miscCrownFeedback_;
    if (GetIntValue(SETTING_CROWN_FEEDBACK_KEY, crownfeedback) != ERR_OK) {
        MISC_HILOGE("Get crownfeedback failed");
    }
    miscCrownFeedback_ = crownfeedback;
 
    int32_t intensity = miscIntensity_;
    if (GetIntValue(SETTING_VIBRATE_INTENSITY_KEY, intensity) != ERR_OK) {
        intensity = FEEDBACK_INTENSITY_STRONGE;
        MISC_HILOGE("Get intensity failed");
    }
    miscIntensity_ = intensity;
}
 
bool VibrationPriorityManager::ShouldIgnoreByIntensity(const VibrateInfo &vibrateInfo)
{
    std::string effect = vibrateInfo.effect;
    if (effect.find("crown") != std::string::npos) {
        if (miscCrownFeedback_ == FEEDBACK_MODE_OFF) {
            return true;
        }
    } else {
        if (miscIntensity_ == FEEDBACK_INTENSITY_NONE) {
            if ((effect.find("short") != std::string::npos) || (effect.find("feedback") != std::string::npos)) {
                return false;
            }
            return true;
        }
    }
    return false;
}
#endif

int32_t VibrationPriorityManager::GetIntValue(const std::string &key, int32_t &value)
{
    int64_t valueLong;
    int32_t ret = GetLongValue(key, valueLong);
    if (ret != ERR_OK) {
        return ret;
    }
    value = static_cast<int32_t>(valueLong);
    return ERR_OK;
}

int32_t VibrationPriorityManager::GetLongValue(const std::string &key, int64_t &value)
{
    std::string valueStr;
    int32_t ret = GetStringValue(key, valueStr);
    if (ret != ERR_OK) {
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
        HiSysEventWrite(HiSysEvent::Domain::MISCDEVICE, "DATASHARE_EXCEPTION", HiSysEvent::EventType::FAULT,
            "PKG_NAME", "GetStringValue", "ERROR_CODE", ret);
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
        MISC_HILOGE("GetStringValue failed, ret:%{public}d", ret);
        return ret;
    }
    value = static_cast<int64_t>(strtoll(valueStr.c_str(), nullptr, DECEM_BASE));
    return ERR_OK;
}

int32_t VibrationPriorityManager::GetStringValue(const std::string &key, std::string &value)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto helper = CreateDataShareHelper(SETTING_URI_PROXY);
    if (helper == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return MISC_NO_INIT_ERR;
    }
    std::vector<std::string> columns = {SETTING_COLUMN_VALUE};
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(AssembleUri(key));
    auto resultSet = helper->Query(uri, predicates, columns);
    ReleaseDataShareHelper(helper);
    if (resultSet == nullptr) {
        MISC_HILOGE("resultSet is nullptr");
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return MISC_INVALID_OPERATION_ERR;
    }
    int32_t count;
    resultSet->GetRowCount(count);
    if (count == 0) {
        MISC_HILOGW("Not found value, key:%{public}s, count:%{public}d", key.c_str(), count);
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return MISC_NAME_NOT_FOUND_ERR;
    }
    const int32_t index = 0;
    resultSet->GoToRow(index);
    int32_t ret = resultSet->GetString(index, value);
    if (ret != ERR_OK) {
        MISC_HILOGW("GetString failed, ret:%{public}d", ret);
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERROR;
    }
    resultSet->Close();
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    return ERR_OK;
}

void VibrationPriorityManager::UpdateStatus()
{
    if (miscFeedback_ == FEEDBACK_MODE_INVALID) {
        int32_t feedback = FEEDBACK_MODE_INVALID;
        if (GetIntValue(SETTING_FEEDBACK_KEY, feedback) != ERR_OK) {
            feedback = FEEDBACK_MODE_ON;
            MISC_HILOGE("Get feedback failed");
        }
        miscFeedback_ = feedback;
    }
    if (miscAudioRingerMode_ == RINGER_MODE_INVALID) {
        int32_t ringerMode = RINGER_MODE_INVALID;
        if (GetIntValue(SETTING_RINGER_MODE_KEY, ringerMode) != ERR_OK) {
            ringerMode = RINGER_MODE_NORMAL;
            MISC_HILOGE("Get ringerMode failed");
        }
        miscAudioRingerMode_ = ringerMode;
    }
#ifdef OHOS_BUILD_ENABLE_DO_NOT_DISTURB
    if (doNotDisturbSwitch_ == DONOTDISTURB_SWITCH_INVALID) {
        InitDoNotDisturbData();
    }
#endif // OHOS_BUILD_ENABLE_DO_NOT_DISTURB
#ifdef OHOS_BUILD_ENABLE_VIBRATOR_CROWN
    if (miscCrownFeedback_ == FEEDBACK_MODE_INVALID) {
        int32_t corwnfeedback = FEEDBACK_MODE_INVALID;
        if (GetIntValue(SETTING_CROWN_FEEDBACK_KEY, corwnfeedback) != ERR_OK) {
            corwnfeedback = FEEDBACK_MODE_ON;
            MISC_HILOGE("Get corwnfeedback failed");
        }
        miscCrownFeedback_ = corwnfeedback;
    }
    if (miscIntensity_ == FEEDBACK_INTENSITY_INVALID) {
        int32_t intensity = FEEDBACK_INTENSITY_INVALID;
        if (GetIntValue(SETTING_VIBRATE_INTENSITY_KEY, intensity) != ERR_OK) {
            intensity = FEEDBACK_INTENSITY_STRONGE;
            MISC_HILOGE("Get intensity failed");
        }
        miscIntensity_ = intensity;
    }
#endif
}

bool VibrationPriorityManager::IsSystemServiceCalling()
{
    const auto tokenId = IPCSkeleton::GetCallingTokenID();
    const auto flag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (flag == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        MISC_HILOGD("System service calling, flag: %{public}u", flag);
        return true;
    }
    return false;
}

bool VibrationPriorityManager::IsSystemCalling()
{
    if (IsSystemServiceCalling()) {
        return true;
    }
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(IPCSkeleton::GetCallingFullTokenID());
}

#ifdef OHOS_BUILD_ENABLE_VIBRATOR_INPUT_METHOD
bool VibrationPriorityManager::ShouldIgnoreInputMethod(const VibrateInfo &vibrateInfo)
{
    if (vibrateInfo.packageName == SCENEBOARD_BUNDLENAME) {
        MISC_HILOGD("Can not ignore for %{public}s", vibrateInfo.packageName.c_str());
        return false;
    }
    int32_t pid = vibrateInfo.pid;
    AppExecFwk::RunningProcessInfo processinfo{};
    appMgrClientPtr_ = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClientPtr_ == nullptr) {
        MISC_HILOGE("appMgrClientPtr is nullptr");
        return false;
    }
    int32_t ret = appMgrClientPtr_->AppExecFwk::AppMgrClient::GetRunningProcessInfoByPid(pid, processinfo);
    if (ret != ERR_OK) {
        MISC_HILOGE("Getrunningprocessinfobypid failed");
        return false;
    }
    if (processinfo.extensionType_ == AppExecFwk::ExtensionAbilityType::INPUTMETHOD) {
        return true;
    }
    std::vector<int32_t> activeUserIds;
    int retId = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeUserIds);
    if (retId != 0) {
        MISC_HILOGE("QueryActiveOsAccountIds failed %{public}d", retId);
        return false;
    }
    if (activeUserIds.empty()) {
        MISC_HILOGE("activeUserId empty");
        return false;
    }
    for (const auto &bundleName : processinfo.bundleNames) {
        MISC_HILOGD("bundleName = %{public}s", bundleName.c_str());
        AppExecFwk::BundleMgrClient bundleMgrClient;
        AppExecFwk::BundleInfo bundleInfo;
        auto res = bundleMgrClient.AppExecFwk::BundleMgrClient::GetBundleInfo(bundleName,
            AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO, bundleInfo, activeUserIds[0]);
        if (!res) {
            MISC_HILOGE("Getbundleinfo fail");
            return false;
        }
        for (const auto &extensionInfo : bundleInfo.extensionInfos) {
            if (extensionInfo.type == AppExecFwk::ExtensionAbilityType::INPUTMETHOD) {
                MISC_HILOGD("extensioninfo type is %{public}d", extensionInfo.type);
                return true;
            }
        }
    }
    return false;
}
#endif // OHOS_BUILD_ENABLE_VIBRATOR_INPUT_METHOD

VibrateStatus VibrationPriorityManager::ShouldIgnoreVibrate(const VibrateInfo &vibrateInfo,
    const std::shared_ptr<VibratorThread> &vibratorThread, const VibratorIdentifierIPC& identifier)
{
    UpdateStatus();
    if (!IsSystemCalling() || vibrateInfo.systemUsage == false) {
#ifdef OHOS_BUILD_ENABLE_DO_NOT_DISTURB
    if (IgnoreAppVibrations(vibrateInfo)) {
        MISC_HILOGD("Vibration is ignored for doNotDisturb, usage:%{public}d", vibrateInfo.usage);
        return IGNORE_GLOBAL_SETTINGS;
    }
#endif // OHOS_BUILD_ENABLE_DO_NOT_DISTURB
    if ((vibrateInfo.usage == USAGE_ALARM || vibrateInfo.usage == USAGE_RING
        || vibrateInfo.usage == USAGE_NOTIFICATION || vibrateInfo.usage == USAGE_COMMUNICATION)
        && (miscAudioRingerMode_ == RINGER_MODE_SILENT)) {
        MISC_HILOGD("Vibration is ignored for ringer mode:%{public}d", static_cast<int32_t>(miscAudioRingerMode_));
        return IGNORE_RINGER_MODE;
    }
    if (((vibrateInfo.usage == USAGE_TOUCH || vibrateInfo.usage == USAGE_MEDIA || vibrateInfo.usage == USAGE_UNKNOWN
        || vibrateInfo.usage == USAGE_PHYSICAL_FEEDBACK || vibrateInfo.usage == USAGE_SIMULATE_REALITY)
        && (miscFeedback_ == FEEDBACK_MODE_OFF))
#ifdef OHOS_BUILD_ENABLE_VIBRATOR_INPUT_METHOD
        && !ShouldIgnoreInputMethod(vibrateInfo)) {
#else // OHOS_BUILD_ENABLE_VIBRATOR_INPUT_METHOD
        ) {
#endif // OHOS_BUILD_ENABLE_VIBRATOR_INPUT_METHOD
        MISC_HILOGD("Vibration is ignored for feedback:%{public}d", static_cast<int32_t>(miscFeedback_));
        return IGNORE_FEEDBACK;
        }
    }
#ifdef OHOS_BUILD_ENABLE_VIBRATOR_CROWN
    if (ShouldIgnoreByIntensity(vibrateInfo)) {
        MISC_HILOGI("ShouldIgnoreByIntensity: vibrateInfo.effect:%{public}s", vibrateInfo.effect.c_str());
        return IGNORE_FEEDBACK;
    }
#endif
    if (vibratorThread == nullptr) {
        MISC_HILOGD("There is no vibration, it can vibrate");
        return VIBRATION;
    }
    if (!IsCurrentVibrate(vibratorThread, identifier)) {
        MISC_HILOGD("There is no vibration at the moment, it can vibrate");
        return VIBRATION;
    }
    if (IsLoopVibrate(vibrateInfo)) {
        MISC_HILOGD("Can vibrate, loop priority is high");
        return VIBRATION;
    }
    return ShouldIgnoreVibrate(vibrateInfo, vibratorThread->GetCurrentVibrateInfo());
}

bool VibrationPriorityManager::IsCurrentVibrate(const std::shared_ptr<VibratorThread> &vibratorThread,
    const VibratorIdentifierIPC& identifier) const
{
#if defined(OHOS_BUILD_ENABLE_VIBRATOR_CUSTOM) && defined(HDF_DRIVERS_INTERFACE_VIBRATOR)
    return ((vibratorThread != nullptr) && (vibratorThread->IsRunning() ||
        VibratorDevice.IsVibratorRunning(identifier)));
#else
    return ((vibratorThread != nullptr) && (vibratorThread->IsRunning()));
#endif // OHOS_BUILD_ENABLE_VIBRATOR_CUSTOM && HDF_DRIVERS_INTERFACE_VIBRATOR
}

bool VibrationPriorityManager::IsLoopVibrate(const VibrateInfo &vibrateInfo) const
{
    return ((vibrateInfo.mode == "preset") && (vibrateInfo.count > 1));
}

VibrateStatus VibrationPriorityManager::ShouldIgnoreVibrate(const VibrateInfo &vibrateInfo,
    VibrateInfo currentVibrateInfo) const
{
    if (currentVibrateInfo.usage == USAGE_ALARM) {
        MISC_HILOGD("Vibration is ignored for alarm");
        return IGNORE_ALARM;
    }
    if (IsLoopVibrate(currentVibrateInfo)) {
        MISC_HILOGD("Vibration is ignored for repeat");
        return IGNORE_REPEAT;
    }
    if ((currentVibrateInfo.usage != vibrateInfo.usage) && (vibrateInfo.usage == USAGE_UNKNOWN)) {
        MISC_HILOGD("Vibration is ignored, unknown has a low priority");
        return IGNORE_UNKNOWN;
    }
    return VIBRATION;
}

sptr<MiscDeviceObserver> VibrationPriorityManager::CreateObserver(const MiscDeviceObserver::UpdateFunc &func)
{
    sptr<MiscDeviceObserver> observer = new MiscDeviceObserver();
    if (observer == nullptr) {
        MISC_HILOGE("observer is null");
        return observer;
    }
    observer->SetUpdateFunc(func);
    return observer;
}

Uri VibrationPriorityManager::AssembleUri(const std::string &key)
{
    Uri uri(SETTING_URI_PROXY + "&key=" + key);
    return uri;
}

std::shared_ptr<DataShare::DataShareHelper> VibrationPriorityManager::CreateDataShareHelper(const std::string &tableUrl)
{
    if (remoteObj_ == nullptr) {
        MISC_HILOGE("remoteObj_ is nullptr");
        return nullptr;
    }
    auto [ret, helper] = DataShare::DataShareHelper::Create(remoteObj_, tableUrl, SETTINGS_DATA_EXT_URI);
    if (ret == DATA_SHARE_READY) {
        return helper;
    } else if (ret == DATA_SHARE_NOT_READY) {
        MISC_HILOGE("Create data_share helper failed, uri proxy:%{public}s", tableUrl.c_str());
        return nullptr;
    }
    MISC_HILOGI("Data_share create unknown");
    return nullptr;
}

bool VibrationPriorityManager::ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper> &helper)
{
    if (!helper->Release()) {
        MISC_HILOGW("Release helper fail");
        return false;
    }
    return true;
}

void VibrationPriorityManager::ExecRegisterCb(const sptr<MiscDeviceObserver> &observer)
{
    if (observer == nullptr) {
        MISC_HILOGE("observer is nullptr");
        return;
    }
    observer->OnChange();
}

int32_t VibrationPriorityManager::RegisterObserver(const sptr<MiscDeviceObserver> &observer)
{
    if (observer == nullptr) {
        MISC_HILOGE("observer is nullptr");
        return MISC_NO_INIT_ERR;
    }
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto helper = CreateDataShareHelper(SETTING_URI_PROXY);
    if (helper == nullptr) {
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
        HiSysEventWrite(HiSysEvent::Domain::MISCDEVICE, "DATASHARE_EXCEPTION", HiSysEvent::EventType::FAULT,
            "PKG_NAME", "RegisterObserver", "ERROR_CODE", MISC_NO_INIT_ERR);
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return MISC_NO_INIT_ERR;
    }
    auto uriFeedback = AssembleUri(SETTING_FEEDBACK_KEY);
    helper->RegisterObserver(uriFeedback, observer);
    helper->NotifyChange(uriFeedback);
#ifdef OHOS_BUILD_ENABLE_VIBRATOR_CROWN
    auto uriCrownFeedback = AssembleUri(SETTING_CROWN_FEEDBACK_KEY);
    helper->RegisterObserver(uriCrownFeedback, observer);
    helper->NotifyChange(uriCrownFeedback);
    auto uriIntensityContrl = AssembleUri(SETTING_CROWN_FEEDBACK_KEY);
    helper->RegisterObserver(uriIntensityContrl, observer);
    helper->NotifyChange(uriIntensityContrl);
#endif
    auto uriRingerMode = AssembleUri(SETTING_RINGER_MODE_KEY);
    helper->RegisterObserver(uriRingerMode, observer);
    helper->NotifyChange(uriRingerMode);
    std::thread execCb(VibrationPriorityManager::ExecRegisterCb, observer);
    execCb.detach();
    ReleaseDataShareHelper(helper);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    MISC_HILOGI("Succeed to register observer of uri");
    return ERR_OK;
}

int32_t VibrationPriorityManager::UnregisterObserver(const sptr<MiscDeviceObserver> &observer)
{
    if (observer == nullptr) {
        MISC_HILOGE("observer is nullptr");
        return MISC_NO_INIT_ERR;
    }
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto helper = CreateDataShareHelper(SETTING_URI_PROXY);
    if (helper == nullptr) {
#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
        HiSysEventWrite(HiSysEvent::Domain::MISCDEVICE, "DATASHARE_EXCEPTION", HiSysEvent::EventType::FAULT,
            "PKG_NAME", "UnregisterObserver", "ERROR_CODE", MISC_NO_INIT_ERR);
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return MISC_NO_INIT_ERR;
    }
    auto uriFeedback = AssembleUri(SETTING_FEEDBACK_KEY);
    helper->UnregisterObserver(uriFeedback, observer);
#ifdef OHOS_BUILD_ENABLE_VIBRATOR_CROWN
    auto uriCrownnFeedback = AssembleUri(SETTING_CROWN_FEEDBACK_KEY);
    helper->UnregisterObserver(uriCrownnFeedback, observer);
    auto uriIntensityContrl = AssembleUri(SETTING_CROWN_FEEDBACK_KEY);
    helper->UnregisterObserver(uriIntensityContrl, observer);
#endif
    auto uriRingerMode = AssembleUri(SETTING_RINGER_MODE_KEY);
    helper->UnregisterObserver(uriRingerMode, observer);
    ReleaseDataShareHelper(helper);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    MISC_HILOGI("Succeed to unregister observer");
    return ERR_OK;
}
}  // namespace Sensors
}  // namespace OHOS