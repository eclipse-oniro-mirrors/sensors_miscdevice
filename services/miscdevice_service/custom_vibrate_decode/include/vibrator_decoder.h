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

#ifndef VIBRATOR_DECODER_H
#define VIBRATOR_DECODER_H

#include <cstdint>
#include <vector>
#include "refbase.h"
#include "vibrator_infos.h"

namespace OHOS {
namespace Sensors {
class VibratorDecoder : public RefBase{
public:
    VibratorDecoder() = default;
    virtual ~VibratorDecoder() = default;
    virtual int32_t DecodeEffect(int32_t fd, std::vector<VibrateEvent> &vibrateSequence) = 0;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // VIBRATOR_DECODER_H