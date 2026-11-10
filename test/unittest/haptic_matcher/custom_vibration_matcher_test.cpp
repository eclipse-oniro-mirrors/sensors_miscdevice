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
#include <map>
#include <memory>
#include <vector>

#include "custom_vibration_matcher.h"
#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "CustomVibrationMatcherTest"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;

namespace {
constexpr int32_t WAVE_ID_FIRST = 1;
constexpr int32_t WAVE_ID_SECOND = 2;
constexpr int32_t WAVE_DURATION = 50;
constexpr int32_t EVENT_INTENSITY_MIDDLE = 50;
constexpr int32_t EVENT_INTENSITY_HIGH = 80;
constexpr int32_t EVENT_FREQUENCY = 40;
constexpr int32_t EVENT_TIME_LATER = 200;
constexpr int32_t EVENT_DURATION = 100;
constexpr int32_t CLAMP_MIN_DURATION = 15;
// Mirror the anonymous-namespace constants in custom_vibration_matcher.cpp, which are invisible here.
constexpr int32_t VIBRATOR_DELAY = 20;
constexpr int32_t INTENSITY_MAX = 100;
constexpr int32_t FREQUENCY_MAX = 100;
constexpr int32_t CONTINUOUS_GRADE_MASK = 100;
}  // namespace

class CustomVibrationMatcherTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() {}

    void TearDown() {}

protected:
    // Build a wave information list whose normalized result is:
    // waveId 1 -> [100, 0, 50], waveId 2 -> [0, 100, 50].
    std::vector<HdfWaveInformation> BuildWaveInfos()
    {
        std::vector<HdfWaveInformation> waveInfos;
        HdfWaveInformation first;
        first.waveId = WAVE_ID_FIRST;
        first.intensity = INTENSITY_MAX;
        first.frequency = 0;
        first.duration = WAVE_DURATION;
        HdfWaveInformation second;
        second.waveId = WAVE_ID_SECOND;
        second.intensity = 0;
        second.frequency = FREQUENCY_MAX;
        second.duration = WAVE_DURATION;
        waveInfos.push_back(first);
        waveInfos.push_back(second);
        return waveInfos;
    }

    VibrateEvent BuildContinuousEvent(int32_t time, int32_t duration, int32_t intensity, int32_t frequency)
    {
        VibrateEvent event;
        event.tag = EVENT_TAG_CONTINUOUS;
        event.time = time;
        event.duration = duration;
        event.intensity = intensity;
        event.frequency = frequency;
        return event;
    }
};

/**
 * @tc.name: GetInstance_001
 * @tc.desc: Test GetInstance, the singleton is constructed with the global hdi connection
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, GetInstance_001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(CustomVibrationMatcher::GetInstance());
    EXPECT_EQ(&(CustomVibrationMatcher::GetInstance()), &(CustomVibrationMatcher::GetInstance()));
}

/**
 * @tc.name: Construct_001
 * @tc.desc: Test the constructor with an empty wave information list, no normalization is performed
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, Construct_001, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    EXPECT_TRUE(matcher.hdfWaveInfos_.empty());
    EXPECT_TRUE(matcher.waveInfos_.empty());
}

/**
 * @tc.name: Construct_002
 * @tc.desc: Test the constructor normalizes the given wave information list
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, Construct_002, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos = BuildWaveInfos();
    CustomVibrationMatcher matcher(identifier, waveInfos);
    EXPECT_EQ(matcher.hdfWaveInfos_.size(), 2UL);
    ASSERT_EQ(matcher.waveInfos_.size(), 2UL);
    EXPECT_EQ(matcher.waveInfos_[WAVE_ID_FIRST][0], INTENSITY_MAX);  // 100 / (100 / 100)
    EXPECT_EQ(matcher.waveInfos_[WAVE_ID_FIRST][1], 0);   // (0 - 0) / ((100 - 0) / 100)
    EXPECT_EQ(matcher.waveInfos_[WAVE_ID_FIRST][2], WAVE_DURATION);
    EXPECT_EQ(matcher.waveInfos_[WAVE_ID_SECOND][0], 0);
    EXPECT_EQ(matcher.waveInfos_[WAVE_ID_SECOND][1], FREQUENCY_MAX);
    EXPECT_EQ(matcher.waveInfos_[WAVE_ID_SECOND][2], WAVE_DURATION);
}

/**
 * @tc.name: NormalizedWaveInfo_001
 * @tc.desc: Test NormalizedWaveInfo exits early when all intensities are zero
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, NormalizedWaveInfo_001, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    HdfWaveInformation wave;
    wave.waveId = WAVE_ID_FIRST;
    wave.intensity = 0;
    wave.frequency = 0;
    wave.duration = WAVE_DURATION;
    waveInfos.push_back(wave);
    CustomVibrationMatcher matcher(identifier, waveInfos);
    EXPECT_TRUE(matcher.waveInfos_.empty());
}

/**
 * @tc.name: NormalizedWaveInfo_002
 * @tc.desc: Test NormalizedWaveInfo exits early when all frequencies are equal
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, NormalizedWaveInfo_002, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    HdfWaveInformation wave;
    wave.waveId = WAVE_ID_FIRST;
    wave.intensity = INTENSITY_MAX;
    wave.frequency = 50;
    wave.duration = WAVE_DURATION;
    waveInfos.push_back(wave);
    CustomVibrationMatcher matcher(identifier, waveInfos);
    EXPECT_TRUE(matcher.waveInfos_.empty());
}

/**
 * @tc.name: TransformTime_001
 * @tc.desc: Test TransformTime with an empty package, which returns ERROR
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, TransformTime_001, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibratePackage package;
    std::vector<CompositeEffect> compositeEffects;
    EXPECT_EQ(matcher.TransformTime(package, compositeEffects), ERROR);
    EXPECT_TRUE(compositeEffects.empty());
}

/**
 * @tc.name: TransformTime_002
 * @tc.desc: Test TransformTime with a single continuous event
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, TransformTime_002, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibratePattern pattern;
    pattern.events.push_back(BuildContinuousEvent(0, EVENT_DURATION, EVENT_INTENSITY_MIDDLE, EVENT_FREQUENCY));
    VibratePackage package;
    package.patterns.push_back(pattern);
    std::vector<CompositeEffect> compositeEffects;
    EXPECT_EQ(matcher.TransformTime(package, compositeEffects), SUCCESS);
    ASSERT_EQ(compositeEffects.size(), 2UL);
    const TimeEffect &first = compositeEffects[0].timeEffect;
    EXPECT_EQ(first.delay, 0);
    EXPECT_EQ(first.time, EVENT_DURATION);
    EXPECT_EQ(first.intensity, EVENT_INTENSITY_MIDDLE);
    EXPECT_EQ(first.frequency, EVENT_FREQUENCY);
    const TimeEffect &last = compositeEffects[1].timeEffect;
    EXPECT_EQ(last.delay, EVENT_DURATION);
    EXPECT_EQ(last.time, 0);
    EXPECT_EQ(last.intensity, 0);
    EXPECT_EQ(last.frequency, 0);
}

/**
 * @tc.name: TransformTime_003
 * @tc.desc: Test TransformTime with two events separated by a large gap
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, TransformTime_003, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibratePattern pattern;
    pattern.events.push_back(BuildContinuousEvent(0, WAVE_DURATION, EVENT_INTENSITY_MIDDLE, 0));
    pattern.events.push_back(
        BuildContinuousEvent(EVENT_TIME_LATER, WAVE_DURATION, EVENT_INTENSITY_HIGH, 0));
    VibratePackage package;
    package.patterns.push_back(pattern);
    std::vector<CompositeEffect> compositeEffects;
    EXPECT_EQ(matcher.TransformTime(package, compositeEffects), SUCCESS);
    ASSERT_EQ(compositeEffects.size(), 3UL);
    EXPECT_EQ(compositeEffects[0].timeEffect.delay, 0);
    EXPECT_EQ(compositeEffects[1].timeEffect.delay, EVENT_TIME_LATER);
    EXPECT_EQ(compositeEffects[2].timeEffect.delay, WAVE_DURATION);
    EXPECT_EQ(compositeEffects[2].timeEffect.time, 0);
}

/**
 * @tc.name: TransformTime_004
 * @tc.desc: Test TransformTime with two overlapped events merged into one
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, TransformTime_004, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibratePattern pattern;
    pattern.events.push_back(BuildContinuousEvent(0, EVENT_DURATION, EVENT_INTENSITY_MIDDLE, 0));
    pattern.events.push_back(
        BuildContinuousEvent(EVENT_DURATION / 2, EVENT_DURATION, EVENT_INTENSITY_HIGH, 0));
    VibratePackage package;
    package.patterns.push_back(pattern);
    std::vector<CompositeEffect> compositeEffects;
    EXPECT_EQ(matcher.TransformTime(package, compositeEffects), SUCCESS);
    // overlapped events are merged: max(0 + 100, 50 + 100) - 0 = 150
    ASSERT_EQ(compositeEffects.size(), 2UL);
    EXPECT_EQ(compositeEffects[0].timeEffect.time, EVENT_DURATION + EVENT_DURATION / 2);
    EXPECT_EQ(compositeEffects[1].timeEffect.delay, EVENT_DURATION + EVENT_DURATION / 2);
}

/**
 * @tc.name: TransformEffect_001
 * @tc.desc: Test TransformEffect with an empty package, which returns ERROR
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, TransformEffect_001, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibratePackage package;
    std::vector<CompositeEffect> compositeEffects;
    EXPECT_EQ(matcher.TransformEffect(package, compositeEffects), ERROR);
    EXPECT_TRUE(compositeEffects.empty());
}

/**
 * @tc.name: TransformEffect_002
 * @tc.desc: Test TransformEffect with a continuous event when no wave info is matched
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, TransformEffect_002, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibratePattern pattern;
    pattern.events.push_back(BuildContinuousEvent(0, WAVE_DURATION, EVENT_INTENSITY_MIDDLE, EVENT_FREQUENCY));
    VibratePackage package;
    package.patterns.push_back(pattern);
    std::vector<CompositeEffect> compositeEffects;
    EXPECT_EQ(matcher.TransformEffect(package, compositeEffects), SUCCESS);
    ASSERT_EQ(compositeEffects.size(), 2UL);
    const PrimitiveEffect &first = compositeEffects[0].primitiveEffect;
    EXPECT_EQ(first.delay, 0);
    EXPECT_EQ(first.effectId, WAVE_DURATION);
    EXPECT_EQ(first.intensity, EVENT_INTENSITY_MIDDLE);
    const PrimitiveEffect &last = compositeEffects[1].primitiveEffect;
    EXPECT_EQ(last.delay, WAVE_DURATION);
    EXPECT_EQ(last.effectId, 0);
    EXPECT_EQ(last.intensity, 0);
}

/**
 * @tc.name: TransformEffect_003
 * @tc.desc: Test TransformEffect with a transient event matched to the nearest wave
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, TransformEffect_003, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos = BuildWaveInfos();
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibratePattern pattern;
    VibrateEvent event;
    event.tag = EVENT_TAG_TRANSIENT;
    event.time = 0;
    event.duration = WAVE_DURATION;
    event.intensity = 90;
    event.frequency = 10;
    pattern.events.push_back(event);
    VibratePackage package;
    package.patterns.push_back(pattern);
    std::vector<CompositeEffect> compositeEffects;
    EXPECT_EQ(matcher.TransformEffect(package, compositeEffects), SUCCESS);
    ASSERT_EQ(compositeEffects.size(), 2UL);
    // wave 1 normalized to [100, 0, 50], distance = 0.5 * |90 - 100| + 0.5 * |10 - 0| = 10
    // wave 2 normalized to [0, 100, 50], distance = 0.5 * |90 - 0| + 0.5 * |10 - 100| = 90
    EXPECT_EQ(compositeEffects[0].primitiveEffect.effectId, -WAVE_ID_FIRST);
    EXPECT_EQ(compositeEffects[0].primitiveEffect.intensity, INTENSITY_MAX);
    EXPECT_EQ(compositeEffects[1].primitiveEffect.delay, WAVE_DURATION);
}

/**
 * @tc.name: TransformEffect_004
 * @tc.desc: Test TransformEffect with an unknown event tag, which returns ERROR
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, TransformEffect_004, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos = BuildWaveInfos();
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibratePattern pattern;
    VibrateEvent event;
    event.tag = EVENT_TAG_UNKNOWN;
    event.time = 0;
    event.duration = WAVE_DURATION;
    event.intensity = EVENT_INTENSITY_MIDDLE;
    pattern.events.push_back(event);
    VibratePackage package;
    package.patterns.push_back(pattern);
    std::vector<CompositeEffect> compositeEffects;
    EXPECT_EQ(matcher.TransformEffect(package, compositeEffects), ERROR);
}

/**
 * @tc.name: MixedWaveProcess_001
 * @tc.desc: Test MixedWaveProcess with an empty package
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, MixedWaveProcess_001, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibratePackage package;
    VibratePattern flatPattern = matcher.MixedWaveProcess(package);
    EXPECT_TRUE(flatPattern.events.empty());
}

/**
 * @tc.name: MixedWaveProcess_002
 * @tc.desc: Test MixedWaveProcess adds the pattern start time to each event
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, MixedWaveProcess_002, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibratePattern pattern;
    pattern.startTime = EVENT_TIME_LATER;
    pattern.events.push_back(BuildContinuousEvent(0, WAVE_DURATION, EVENT_INTENSITY_MIDDLE, 0));
    VibratePackage package;
    package.patterns.push_back(pattern);
    VibratePattern flatPattern = matcher.MixedWaveProcess(package);
    ASSERT_EQ(flatPattern.events.size(), 1UL);
    EXPECT_EQ(flatPattern.events[0].time, EVENT_TIME_LATER);
}

/**
 * @tc.name: MixedWaveProcess_003
 * @tc.desc: Test MixedWaveProcess appends an event when the last output event is transient
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, MixedWaveProcess_003, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibratePattern pattern;
    VibrateEvent transient;
    transient.tag = EVENT_TAG_TRANSIENT;
    transient.time = 0;
    transient.duration = WAVE_DURATION / 2;
    transient.intensity = EVENT_INTENSITY_MIDDLE;
    pattern.events.push_back(transient);
    pattern.events.push_back(BuildContinuousEvent(0, WAVE_DURATION, EVENT_INTENSITY_HIGH, 0));
    VibratePackage package;
    package.patterns.push_back(pattern);
    VibratePattern flatPattern = matcher.MixedWaveProcess(package);
    ASSERT_EQ(flatPattern.events.size(), 2UL);
}

/**
 * @tc.name: MixedWaveProcess_004
 * @tc.desc: Test MixedWaveProcess shortens the gap when the interval is less than the delay threshold
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, MixedWaveProcess_004, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibratePattern pattern;
    pattern.events.push_back(BuildContinuousEvent(0, EVENT_DURATION, EVENT_INTENSITY_MIDDLE, 0));
    // interval = 105 - 100 = 5 < VIBRATOR_DELAY(20), duration is shortened by (5 - 20)
    pattern.events.push_back(
        BuildContinuousEvent(EVENT_DURATION + 5, WAVE_DURATION, EVENT_INTENSITY_HIGH, 0));
    VibratePackage package;
    package.patterns.push_back(pattern);
    VibratePattern flatPattern = matcher.MixedWaveProcess(package);
    ASSERT_EQ(flatPattern.events.size(), 2UL);
    EXPECT_EQ(flatPattern.events[0].duration, EVENT_DURATION - (VIBRATOR_DELAY - 5));
    EXPECT_EQ(flatPattern.events[1].time, EVENT_DURATION + 5);
}

/**
 * @tc.name: PreProcessEvent_001
 * @tc.desc: Test PreProcessEvent fills default curve points when the event has no points
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, PreProcessEvent_001, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibrateEvent event = BuildContinuousEvent(10, WAVE_DURATION * 2, 50, 0);
    event.points.clear();
    matcher.PreProcessEvent(event);
    ASSERT_EQ(event.points.size(), 2UL);
    EXPECT_EQ(event.points[0].time, 10);
    EXPECT_EQ(event.points[0].intensity, 50);  // 100 * (50 / 100)
    EXPECT_EQ(event.points[1].time, 10 + WAVE_DURATION * 2);
    EXPECT_EQ(event.points[1].intensity, 50);
}

/**
 * @tc.name: PreProcessEvent_002
 * @tc.desc: Test PreProcessEvent clamps the duration to the minimum vibration duration
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, PreProcessEvent_002, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibrateEvent event = BuildContinuousEvent(0, CLAMP_MIN_DURATION / 3, EVENT_INTENSITY_MIDDLE, 0);
    matcher.PreProcessEvent(event);
    EXPECT_EQ(event.duration, CLAMP_MIN_DURATION);
}

/**
 * @tc.name: PreProcessEvent_003
 * @tc.desc: Test PreProcessEvent clamps intensity and frequency into their valid ranges
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, PreProcessEvent_003, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibrateEvent event = BuildContinuousEvent(0, WAVE_DURATION * 2, 10, -300);
    VibrateCurvePoint point;
    point.time = 0;
    point.intensity = 200;
    point.frequency = 200;
    event.points.push_back(point);
    matcher.PreProcessEvent(event);
    ASSERT_EQ(event.points.size(), 1UL);
    EXPECT_EQ(event.points[0].intensity, 20);  // min(200 * (10 / 100), 100)
    EXPECT_EQ(event.points[0].frequency, 0);    // max(min(200 - 300, 100), 0)
}

/**
 * @tc.name: MergeCurve_001
 * @tc.desc: Test MergeCurve returns the right curve when the left curve is empty
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, MergeCurve_001, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    std::vector<VibrateCurvePoint> left;
    std::vector<VibrateCurvePoint> right;
    VibrateCurvePoint point;
    point.time = 0;
    point.intensity = EVENT_INTENSITY_MIDDLE;
    right.push_back(point);
    std::vector<VibrateCurvePoint> merged = matcher.MergeCurve(left, right);
    EXPECT_EQ(merged.size(), right.size());
}

/**
 * @tc.name: MergeCurve_002
 * @tc.desc: Test MergeCurve returns the left curve when the right curve is empty
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, MergeCurve_002, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    std::vector<VibrateCurvePoint> left;
    VibrateCurvePoint point;
    point.time = 0;
    point.intensity = EVENT_INTENSITY_MIDDLE;
    left.push_back(point);
    std::vector<VibrateCurvePoint> right;
    std::vector<VibrateCurvePoint> merged = matcher.MergeCurve(left, right);
    EXPECT_EQ(merged.size(), left.size());
}

/**
 * @tc.name: MergeCurve_003
 * @tc.desc: Test MergeCurve with two non-overlapping curves
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, MergeCurve_003, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    std::vector<VibrateCurvePoint> left;
    VibrateCurvePoint leftPoint;
    leftPoint.time = 0;
    leftPoint.intensity = 10;
    left.push_back(leftPoint);
    std::vector<VibrateCurvePoint> right;
    VibrateCurvePoint rightPoint;
    rightPoint.time = 50;
    rightPoint.intensity = 20;
    right.push_back(rightPoint);
    std::vector<VibrateCurvePoint> merged = matcher.MergeCurve(left, right);
    // overlap interval [max(0, 50), min(0, 50)] is empty, both points are kept in order
    ASSERT_EQ(merged.size(), 2UL);
    EXPECT_EQ(merged[0].time, 0);
    EXPECT_EQ(merged[0].intensity, 10);
    EXPECT_EQ(merged[1].time, 50);
    EXPECT_EQ(merged[1].intensity, 20);
}

/**
 * @tc.name: Interpolation_001
 * @tc.desc: Test Interpolation returns y1 when x1 equals x2
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, Interpolation_001, TestSize.Level2)
{
    EXPECT_EQ(CustomVibrationMatcher::Interpolation(5, 5, 10, 20, 100), 10);
}

/**
 * @tc.name: Interpolation_002
 * @tc.desc: Test Interpolation with regular linear interpolation
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, Interpolation_002, TestSize.Level1)
{
    EXPECT_EQ(CustomVibrationMatcher::Interpolation(0, 10, 0, 20, 5), 10);
}

/**
 * @tc.name: ProcessContinuousEventSlice_001
 * @tc.desc: Test ProcessContinuousEventSlice with the maximum intensity, grade is the top one
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, ProcessContinuousEventSlice_001, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibrateSlice slice = {0, WAVE_DURATION, INTENSITY_MAX, 0};
    int32_t preStartTime = 0;
    int32_t preDuration = 0;
    std::vector<CompositeEffect> compositeEffects;
    matcher.ProcessContinuousEventSlice(slice, preStartTime, preDuration, compositeEffects);
    ASSERT_EQ(compositeEffects.size(), 1UL);
    EXPECT_EQ(compositeEffects[0].primitiveEffect.effectId, WAVE_DURATION * CONTINUOUS_GRADE_MASK + 7);
    EXPECT_EQ(preStartTime, 0);
    EXPECT_EQ(preDuration, WAVE_DURATION);
}

/**
 * @tc.name: ProcessContinuousEventSlice_002
 * @tc.desc: Test ProcessContinuousEventSlice with a middle intensity, grade is scaled down
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, ProcessContinuousEventSlice_002, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibrateSlice slice = {0, WAVE_DURATION, EVENT_INTENSITY_MIDDLE, 0};
    int32_t preStartTime = 0;
    int32_t preDuration = 0;
    std::vector<CompositeEffect> compositeEffects;
    matcher.ProcessContinuousEventSlice(slice, preStartTime, preDuration, compositeEffects);
    ASSERT_EQ(compositeEffects.size(), 1UL);
    // round(50 / 12.5 + 0.5) - 1 = round(4.5) - 1 = 4
    EXPECT_EQ(compositeEffects[0].primitiveEffect.effectId % CONTINUOUS_GRADE_MASK, 4);
}

/**
 * @tc.name: ProcessContinuousEventSlice_003
 * @tc.desc: Test ProcessContinuousEventSlice merges the adjacent slice with the same grade
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, ProcessContinuousEventSlice_003, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    int32_t preStartTime = 0;
    int32_t preDuration = 0;
    std::vector<CompositeEffect> compositeEffects;
    VibrateSlice first = {0, WAVE_DURATION * 2, EVENT_INTENSITY_MIDDLE, 0};
    matcher.ProcessContinuousEventSlice(first, preStartTime, preDuration, compositeEffects);
    VibrateSlice second = {WAVE_DURATION * 2, WAVE_DURATION * 2, EVENT_INTENSITY_MIDDLE, 0};
    matcher.ProcessContinuousEventSlice(second, preStartTime, preDuration, compositeEffects);
    ASSERT_EQ(compositeEffects.size(), 1UL);
    EXPECT_EQ(compositeEffects[0].primitiveEffect.effectId, WAVE_DURATION * 4 * CONTINUOUS_GRADE_MASK + 4);
    EXPECT_EQ(preDuration, WAVE_DURATION * 4);
}

/**
 * @tc.name: ProcessContinuousEventSlice_004
 * @tc.desc: Test ProcessContinuousEventSlice does not merge when the merged duration exceeds the limit
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, ProcessContinuousEventSlice_004, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    int32_t preStartTime = 0;
    int32_t preDuration = 0;
    std::vector<CompositeEffect> compositeEffects;
    VibrateSlice first = {0, 1000, EVENT_INTENSITY_MIDDLE, 0};
    matcher.ProcessContinuousEventSlice(first, preStartTime, preDuration, compositeEffects);
    VibrateSlice second = {1000, 1000, EVENT_INTENSITY_MIDDLE, 0};
    matcher.ProcessContinuousEventSlice(second, preStartTime, preDuration, compositeEffects);
    EXPECT_EQ(compositeEffects.size(), 2UL);
}

/**
 * @tc.name: ProcessContinuousEventSlice_005
 * @tc.desc: Test ProcessContinuousEventSlice does not merge slices with different grades
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, ProcessContinuousEventSlice_005, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    int32_t preStartTime = 0;
    int32_t preDuration = 0;
    std::vector<CompositeEffect> compositeEffects;
    VibrateSlice first = {0, WAVE_DURATION * 2, EVENT_INTENSITY_MIDDLE, 0};
    matcher.ProcessContinuousEventSlice(first, preStartTime, preDuration, compositeEffects);
    VibrateSlice second = {WAVE_DURATION * 2, WAVE_DURATION * 2, 30, 0};
    matcher.ProcessContinuousEventSlice(second, preStartTime, preDuration, compositeEffects);
    EXPECT_EQ(compositeEffects.size(), 2UL);
}

/**
 * @tc.name: ProcessContinuousEvent_001
 * @tc.desc: Test ProcessContinuousEvent with a short event processed as one slice
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, ProcessContinuousEvent_001, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibrateEvent event = BuildContinuousEvent(0, WAVE_DURATION, EVENT_INTENSITY_MIDDLE, 0);
    VibrateCurvePoint point = {0, EVENT_INTENSITY_MIDDLE, 0};
    event.points.push_back(point);
    VibrateCurvePoint endPoint = {WAVE_DURATION, EVENT_INTENSITY_MIDDLE, 0};
    event.points.push_back(endPoint);
    int32_t preStartTime = 0;
    int32_t preDuration = 0;
    std::vector<CompositeEffect> compositeEffects;
    matcher.ProcessContinuousEvent(event, preStartTime, preDuration, compositeEffects);
    ASSERT_EQ(compositeEffects.size(), 1UL);
    EXPECT_EQ(compositeEffects[0].primitiveEffect.effectId, WAVE_DURATION * CONTINUOUS_GRADE_MASK + 4);
}

/**
 * @tc.name: ProcessContinuousEvent_002
 * @tc.desc: Test ProcessContinuousEvent returns directly when the curve is empty
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, ProcessContinuousEvent_002, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibrateEvent event = BuildContinuousEvent(0, EVENT_DURATION * 2, EVENT_INTENSITY_MIDDLE, 0);
    event.points.clear();
    int32_t preStartTime = 0;
    int32_t preDuration = 0;
    std::vector<CompositeEffect> compositeEffects;
    matcher.ProcessContinuousEvent(event, preStartTime, preDuration, compositeEffects);
    EXPECT_TRUE(compositeEffects.empty());
}

/**
 * @tc.name: ProcessContinuousEvent_003
 * @tc.desc: Test ProcessContinuousEvent slices a long event and merges slices with the same grade
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, ProcessContinuousEvent_003, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibrateEvent event = BuildContinuousEvent(0, EVENT_DURATION * 2, EVENT_INTENSITY_HIGH, EVENT_FREQUENCY);
    event.points.push_back({0, EVENT_INTENSITY_HIGH, EVENT_FREQUENCY});
    event.points.push_back({EVENT_DURATION, EVENT_INTENSITY_HIGH, EVENT_FREQUENCY});
    event.points.push_back({EVENT_DURATION * 2, EVENT_INTENSITY_HIGH, EVENT_FREQUENCY});
    int32_t preStartTime = 0;
    int32_t preDuration = 0;
    std::vector<CompositeEffect> compositeEffects;
    matcher.ProcessContinuousEvent(event, preStartTime, preDuration, compositeEffects);
    // intensity 80 -> grade round(80 / 12.5 + 0.5) - 1 = 6, two adjacent slices are merged
    ASSERT_EQ(compositeEffects.size(), 1UL);
    EXPECT_EQ(compositeEffects[0].primitiveEffect.effectId, EVENT_DURATION * 2 * CONTINUOUS_GRADE_MASK + 6);
}

/**
 * @tc.name: ProcessTransientEvent_001
 * @tc.desc: Test ProcessTransientEvent with no wave info, effectId defaults to zero
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, ProcessTransientEvent_001, TestSize.Level2)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    VibrateEvent event;
    event.tag = EVENT_TAG_TRANSIENT;
    event.time = 0;
    event.duration = WAVE_DURATION;
    event.intensity = EVENT_INTENSITY_MIDDLE;
    event.frequency = 0;
    int32_t preStartTime = 0;
    int32_t preDuration = 0;
    std::vector<CompositeEffect> compositeEffects;
    matcher.ProcessTransientEvent(event, preStartTime, preDuration, compositeEffects);
    ASSERT_EQ(compositeEffects.size(), 1UL);
    EXPECT_EQ(compositeEffects[0].primitiveEffect.effectId, 0);
    EXPECT_EQ(compositeEffects[0].primitiveEffect.intensity, INTENSITY_MAX);
}

/**
 * @tc.name: ProcessTransientEvent_002
 * @tc.desc: Test ProcessTransientEvent matches the wave with the minimum weight sum
 * @tc.type: FUNC
 */
HWTEST_F(CustomVibrationMatcherTest, ProcessTransientEvent_002, TestSize.Level1)
{
    VibratorIdentifierIPC identifier;
    std::vector<HdfWaveInformation> waveInfos;
    CustomVibrationMatcher matcher(identifier, waveInfos);
    matcher.waveInfos_[WAVE_ID_FIRST] = {90, 10, WAVE_DURATION};
    matcher.waveInfos_[WAVE_ID_SECOND] = {10, 90, WAVE_DURATION};
    VibrateEvent event;
    event.tag = EVENT_TAG_TRANSIENT;
    event.time = EVENT_TIME_LATER;
    event.duration = WAVE_DURATION;
    event.intensity = EVENT_INTENSITY_HIGH;
    event.frequency = EVENT_FREQUENCY;
    int32_t preStartTime = 0;
    int32_t preDuration = 0;
    std::vector<CompositeEffect> compositeEffects;
    matcher.ProcessTransientEvent(event, preStartTime, preDuration, compositeEffects);
    ASSERT_EQ(compositeEffects.size(), 1UL);
    // wave 1 distance = 0.5 * |80 - 90| + 0.5 * |40 - 10| = 20, wave 2 distance = 65
    EXPECT_EQ(compositeEffects[0].primitiveEffect.effectId, -WAVE_ID_FIRST);
    EXPECT_EQ(compositeEffects[0].primitiveEffect.delay, EVENT_TIME_LATER);
    EXPECT_EQ(preStartTime, EVENT_TIME_LATER);
    EXPECT_EQ(preDuration, WAVE_DURATION);
}
}  // namespace Sensors
}  // namespace OHOS
