/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ANDROID_HARDWARE_LIGHT_V2_0_LIGHT_H
#define ANDROID_HARDWARE_LIGHT_V2_0_LIGHT_H

#include <android/hardware/light/2.0/ILight.h>
#include <hidl/Status.h>

#include <fstream>
#include <mutex>
#include <unordered_map>

namespace android {
namespace hardware {
namespace light {
namespace V2_0 {
namespace implementation {

struct Light : public ILight {
    Light(std::ofstream&& backlight,
          // Brightness
          std::ofstream&& redLed,
          std::ofstream&& greenLed,
          std::ofstream&& blueLed,
          // DutyPcts
          std::ofstream&& redDutyPcts,
          std::ofstream&& greenDutyPcts,
          std::ofstream&& blueDutyPcts,
          // StartIdx
          std::ofstream&& redStartIdx,
          std::ofstream&& greenStartIdx,
          std::ofstream&& blueStartIdx,
          // PauseLo
          std::ofstream&& redPauseLo,
          std::ofstream&& greenPauseLo,
          std::ofstream&& bluePauseLo,
          // PauseHi
          std::ofstream&& redPauseHi,
          std::ofstream&& greenPauseHi,
          std::ofstream&& bluePauseHi,
          // RampStepMs
          std::ofstream&& redRampStepMs,
          std::ofstream&& greenRampStepMs,
          std::ofstream&& blueRampStepMs,
          // Blink
          std::ofstream&& redBlink,
          std::ofstream&& greenBlink,
          std::ofstream&& blueBlink);

    // Methods from ::android::hardware::light::V2_0::ILight follow.
    Return<Status> setLight(Type type, const LightState& state)  override;
    Return<void> getSupportedTypes(getSupportedTypes_cb _hidl_cb)  override;

private:
    void setAttentionLight(const LightState& state);
    void setBacklight(const LightState& state);
    void setBatteryLight(const LightState& state);
    void setNotificationLight(const LightState& state);
    void setSpeakerBatteryLightLocked();
    void setSpeakerLightLocked(const LightState& state);

    std::ofstream mBacklight;
    // Brightness
    std::ofstream mRedLed;
    std::ofstream mGreenLed;
    std::ofstream mBlueLed;
    // DutyPcts
    std::ofstream mRedDutyPcts;
    std::ofstream mGreenDutyPcts;
    std::ofstream mBlueDutyPcts;
    // StartIdx
    std::ofstream mRedStartIdx;
    std::ofstream mGreenStartIdx;
    std::ofstream mBlueStartIdx;
    // PauseLo
    std::ofstream mRedPauseLo;
    std::ofstream mGreenPauseLo;
    std::ofstream mBluePauseLo;
    // PauseHi
    std::ofstream mRedPauseHi;
    std::ofstream mGreenPauseHi;
    std::ofstream mBluePauseHi;
    // RampStepMs
    std::ofstream mRedRampStepMs;
    std::ofstream mGreenRampStepMs;
    std::ofstream mBlueRampStepMs;
    // Blink
    std::ofstream mRedBlink;
    std::ofstream mGreenBlink;
    std::ofstream mBlueBlink;

    LightState mAttentionState;
    LightState mBatteryState;
    LightState mNotificationState;

    std::unordered_map<Type, std::function<void(const LightState&)>> mLights;
    std::mutex mLock;
};
}  // namespace implementation
}  // namespace V2_0
}  // namespace light
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_LIGHT_V2_0_LIGHT_H
