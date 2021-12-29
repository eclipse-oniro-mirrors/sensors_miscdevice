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

#ifndef SENSORS_ERRORS_H
#define SENSORS_ERRORS_H

#include <errors.h>

namespace OHOS {
namespace Sensors {
enum {
    MODULE_COMMON = 0x00,
    MODULE_MISCDEVICE_DEVICE = 0x01,
    MODULE_MISCDEVICE_UTILS = 0x02,
    MODULE_MISCDEVICE_SERVICE = 0x03,
    MODULE_MISCDEVICE_NATIVE = 0X04,
};

// Error code for common
constexpr ErrCode COMMON_ERR_OFFSET = ErrCodeOffset(SUBSYS_SENSORS, MODULE_COMMON);

enum {
    ERROR = -1,
    SUCCESS = 0,
    COMMON_ERR = COMMON_ERR_OFFSET,
};

// Error code for adapter
constexpr ErrCode DEVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_SENSORS, MODULE_MISCDEVICE_DEVICE);

enum {
    DEVICE_ERR = DEVICE_ERR_OFFSET,
    DEVICE_ENABLE_SENSOR_ERR = DEVICE_ERR_OFFSET + 1,
    DEVICE_DISABLE_SENSOR_ERR = DEVICE_ENABLE_SENSOR_ERR + 1,
    DEVICE_RUN_COMMAND_ERR = DEVICE_DISABLE_SENSOR_ERR + 1,
    DEVICE_SET_SENSOR_CONFIG_ERR = DEVICE_RUN_COMMAND_ERR + 1,
    DEVICE_NOT_SUPPORT_CMD_ERR = DEVICE_SET_SENSOR_CONFIG_ERR + 1,
};

// Error code for miscdevice service
constexpr ErrCode MISCDEVICE_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_SENSORS, MODULE_MISCDEVICE_SERVICE);

enum {
    LIGHT_HDF_CONNECT_ERR = MISCDEVICE_SERVICE_ERR_OFFSET,
    LIGHT_ID_NOT_SUPPORT = LIGHT_HDF_CONNECT_ERR + 1,
    LIGHT_ERR = LIGHT_ID_NOT_SUPPORT + 1,
    LIGHT_PLAY_EFFECT_ERROR = LIGHT_ERR + 1,
    LIGHT_STOP_EFFECT_ERROR = LIGHT_PLAY_EFFECT_ERROR + 1,
    LIGHT_END_ERROR = LIGHT_STOP_EFFECT_ERROR,
    VIBRATOR_HDF_CONNECT_ERR = LIGHT_END_ERROR + 1,
    VIBRATOR_ON_ERR = VIBRATOR_HDF_CONNECT_ERR + 1,
    VIBRATOR_OFF_ERR = VIBRATOR_ON_ERR + 1,
    VIBRATOR_PLAY_EFFECT_ERR = VIBRATOR_OFF_ERR + 1,
    VIBRATOR_STOP_EFFECT_ERR = VIBRATOR_PLAY_EFFECT_ERR + 1,
    VIBRATOR_SET_PARA_ERR = VIBRATOR_STOP_EFFECT_ERR + 1,
    WRITE_MSG_ERR = VIBRATOR_SET_PARA_ERR + 1,
};

// Error code for Sensor native
constexpr ErrCode MISCDEVICE_NATIVE_ERR_OFFSET = ErrCodeOffset(SUBSYS_SENSORS, MODULE_MISCDEVICE_NATIVE);

enum {
    MISCDEVICE_NATIVE_SAM_ERR = MISCDEVICE_NATIVE_ERR_OFFSET,
    MISC_NATIVE_GET_SERVICE_ERR = MISCDEVICE_NATIVE_SAM_ERR + 1,
    MISC_NATIVE_SAM_ERR = MISC_NATIVE_GET_SERVICE_ERR + 1,
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // SENSORS_ERRORS_H
