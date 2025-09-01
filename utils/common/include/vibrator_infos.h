/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef VIBRATOR_INFOS_H
#define VIBRATOR_INFOS_H

#include "parcel.h"
namespace OHOS {
namespace Sensors {
constexpr int32_t MAX_EVENT_SIZE = 16;
constexpr int32_t MAX_POINT_SIZE = 16;
const std::string VIBRATE_BUTT = "butt";
const std::string VIBRATE_TIME = "time";
const std::string VIBRATE_PRESET = "preset";
const std::string VIBRATE_CUSTOM_HD = "custom.hd";
const std::string VIBRATE_CUSTOM_COMPOSITE_EFFECT = "custom.composite.effect";
const std::string VIBRATE_CUSTOM_COMPOSITE_TIME = "custom.composite.time";

enum VibrateUsage {
    USAGE_UNKNOWN = 0,
    USAGE_ALARM = 1,
    USAGE_RING = 2,
    USAGE_NOTIFICATION = 3,
    USAGE_COMMUNICATION = 4,
    USAGE_TOUCH = 5,
    USAGE_MEDIA = 6,
    USAGE_PHYSICAL_FEEDBACK = 7,
    USAGE_SIMULATE_REALITY = 8,
    USAGE_MAX = 9,
};

enum VibrateTag {
    EVENT_TAG_UNKNOWN = -1,
    EVENT_TAG_CONTINUOUS = 0,
    EVENT_TAG_TRANSIENT = 1,
};

enum VibrateCustomMode {
    VIBRATE_MODE_HD = 0,
    VIBRATE_MODE_MAPPING = 1,
    VIBRATE_MODE_TIMES = 2,
};

struct VibrateCurvePoint {
    bool operator<(const VibrateCurvePoint &rhs) const
    {
        return time < rhs.time;
    }
    int32_t time = 0;
    int32_t intensity = 0;
    int32_t frequency = 0;
};

struct VibrateEvent {
    bool operator<(const VibrateEvent &rhs) const
    {
        return time < rhs.time;
    }

    VibrateTag tag;
    int32_t time = 0;
    int32_t duration = 0;
    int32_t intensity = 0;
    int32_t frequency = 0;
    int32_t index = 0;
    std::vector<VibrateCurvePoint> points;
};

struct VibratePattern : public Parcelable {
    bool operator<(const VibratePattern &rhs) const
    {
        return startTime < rhs.startTime;
    }
    int32_t startTime = 0;
    int32_t patternDuration = 0;
    std::vector<VibrateEvent> events;
    void Dump() const;
    bool Marshalling(Parcel &parcel) const;
    static VibratePattern* Unmarshalling(Parcel &data);
};

struct VibratePackage {
    std::vector<VibratePattern> patterns;
    int32_t packageDuration = 0;
    void Dump() const;
};

struct VibratePackageIPC : public Parcelable {
    int32_t packageDuration = 0;
    std::vector<VibratePattern> patterns;
    void Dump() const;
    bool Marshalling(Parcel &parcel) const;
    static VibratePackageIPC* Unmarshalling(Parcel &data);
};

struct VibratorCapacity : public Parcelable {
    bool isSupportHdHaptic = false;
    bool isSupportPresetMapping = false;
    bool isSupportTimeDelay = false;
    void Dump() const;
    int32_t GetVibrateMode();
    bool Marshalling(Parcel &parcel) const;
    static VibratorCapacity* Unmarshalling(Parcel &data);
};

struct VibrateSlice {
    int32_t time = 0;
    int32_t duration = 0;
    int32_t intensity = 0;
    int32_t frequency = 0;
};

struct VibrateInfo {
    std::string mode;
    std::string packageName;
    int32_t pid = -1;
    int32_t uid = -1;
    int32_t usage = 0;
    bool systemUsage = false;
    int32_t duration = 0;
    std::string effect;
    int32_t count = 0;
    int32_t intensity = 0;
    uint32_t sessionId = 0;
    VibratePackage package;
    VibratePackageIPC packageIPC;
};

struct VibrateParameter : public Parcelable {
    int32_t intensity = 100;  // from 0 to 100
    int32_t frequency = 0;    // from -100 to 100
    int32_t reserved = 0;
    uint32_t sessionId = 0;
    void Dump() const;
    bool Marshalling(Parcel &parcel) const;
    static VibrateParameter* Unmarshalling(Parcel &data);
};


struct VibratorInfoIPC : public Parcelable {
    int32_t deviceId = -1;
    int32_t vibratorId = -1;
    std::string deviceName = "";
    bool isSupportHdHaptic;
    bool isLocalVibrator;
    int32_t position = 0;
    void Dump() const;
    bool Marshalling(Parcel &parcel) const;
    static VibratorInfoIPC* Unmarshalling(Parcel &data);
};

struct VibratorIdentifierIPC : public Parcelable {
    int32_t deviceId = -1;
    int32_t vibratorId = -1;
    int32_t position = 0;
    bool isLocalVibrator;
    void Dump() const;
    bool Marshalling(Parcel &parcel) const;
    static VibratorIdentifierIPC* Unmarshalling(Parcel &data);
};

struct EffectInfoIPC : public Parcelable {
    int32_t duration = -1;
    bool isSupportEffect;
    void Dump() const;
    bool Marshalling(Parcel &parcel) const;
    static EffectInfoIPC* Unmarshalling(Parcel &data);
};

struct CustomHapticInfoIPC : public Parcelable {
    int32_t usage = 0;
    bool systemUsage;
    VibrateParameter parameter;
    void Dump() const;
    bool Marshalling(Parcel &parcel) const;
    static CustomHapticInfoIPC* Unmarshalling(Parcel &data);
};

struct PrimitiveEffectIPC : public Parcelable {
    int32_t intensity = 0;
    int32_t usage = 0;
    bool systemUsage;
    int32_t count = 0;
    void Dump() const;
    bool Marshalling(Parcel &parcel) const;
    static PrimitiveEffectIPC* Unmarshalling(Parcel &data);
};
} // namespace Sensors
} // namespace OHOS
#endif // VIBRATOR_INFOS_H
