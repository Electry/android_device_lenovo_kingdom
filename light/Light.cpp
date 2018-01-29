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

#define LOG_TAG "light"

#include "Light.h"

#include <log/log.h>

namespace {
using android::hardware::light::V2_0::LightState;

static int BRIGHTNESS_RAMP[] = {
    0, 12, 25, 37, 50, 72, 85, 100
};
#define RAMP_SIZE (sizeof(BRIGHTNESS_RAMP)/sizeof(BRIGHTNESS_RAMP[0]))
#define RAMP_STEP_DURATION 50

static uint32_t rgbToBrightness(const LightState& state) {
    uint32_t color = state.color & 0x00ffffff;
    return ((77 * ((color >> 16) & 0xff)) + (150 * ((color >> 8) & 0xff)) +
            (29 * (color & 0xff))) >> 8;
}

static bool isLit(const LightState& state) {
    return (state.color & 0x00ffffff);
}

static char* get_scaled_duty_pcts(int brightness) {
    char *buf = (char*) malloc(5 * RAMP_SIZE * sizeof(char));
    char *pad = (char*) "";
    uint8_t i = 0;

    memset(buf, 0, 5 * RAMP_SIZE * sizeof(char));

    for (i = 0; i < RAMP_SIZE; i++) {
        char temp[5] = "";
        snprintf(temp, sizeof(temp), "%s%d", pad, (BRIGHTNESS_RAMP[i] * brightness / 255));
        strcat(buf, temp);
        pad = (char*) ",";
    }
    ALOGV("%s: brightness=%d duty=%s", __func__, brightness, buf);
    return buf;
}
} // anonymous namespace

namespace android {
namespace hardware {
namespace light {
namespace V2_0 {
namespace implementation {

Light::Light(std::ofstream&& backlight,
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
             std::ofstream&& blueBlink) :
    mBacklight(std::move(backlight)),
    // Brightness
    mRedLed(std::move(redLed)),
    mGreenLed(std::move(greenLed)),
    mBlueLed(std::move(blueLed)),
    // DutyPcts
    mRedDutyPcts(std::move(redDutyPcts)),
    mGreenDutyPcts(std::move(greenDutyPcts)),
    mBlueDutyPcts(std::move(blueDutyPcts)),
    // StartIdx
    mRedStartIdx(std::move(redStartIdx)),
    mGreenStartIdx(std::move(greenStartIdx)),
    mBlueStartIdx(std::move(blueStartIdx)),
    // PauseLo
    mRedPauseLo(std::move(redPauseLo)),
    mGreenPauseLo(std::move(greenPauseLo)),
    mBluePauseLo(std::move(bluePauseLo)),
    // PauseHi
    mRedPauseHi(std::move(redPauseHi)),
    mGreenPauseHi(std::move(greenPauseHi)),
    mBluePauseHi(std::move(bluePauseHi)),
    // RampStepMs
    mRedRampStepMs(std::move(redRampStepMs)),
    mGreenRampStepMs(std::move(greenRampStepMs)),
    mBlueRampStepMs(std::move(blueRampStepMs)),
    // Blink
    mRedBlink(std::move(redBlink)),
    mGreenBlink(std::move(greenBlink)),
    mBlueBlink(std::move(blueBlink)) {
    auto attnFn(std::bind(&Light::setAttentionLight, this, std::placeholders::_1));
    auto backlightFn(std::bind(&Light::setBacklight, this, std::placeholders::_1));
    auto batteryFn(std::bind(&Light::setBatteryLight, this, std::placeholders::_1));
    auto notifFn(std::bind(&Light::setNotificationLight, this, std::placeholders::_1));
    mLights.emplace(std::make_pair(Type::ATTENTION, attnFn));
    mLights.emplace(std::make_pair(Type::BACKLIGHT, backlightFn));
    mLights.emplace(std::make_pair(Type::BATTERY, batteryFn));
    mLights.emplace(std::make_pair(Type::NOTIFICATIONS, notifFn));
}

// Methods from ::android::hardware::light::V2_0::ILight follow.
Return<Status> Light::setLight(Type type, const LightState& state) {
    auto it = mLights.find(type);

    if (it == mLights.end()) {
        return Status::LIGHT_NOT_SUPPORTED;
    }

    it->second(state);

    return Status::SUCCESS;
}

Return<void> Light::getSupportedTypes(getSupportedTypes_cb _hidl_cb) {
    std::vector<Type> types;

    for (auto const& light : mLights) {
        types.push_back(light.first);
    }

    _hidl_cb(types);

    return Void();
}

void Light::setAttentionLight(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    mAttentionState = state;
    setSpeakerBatteryLightLocked();
}

void Light::setBacklight(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);

    uint32_t brightness = rgbToBrightness(state);

    mBacklight << brightness;
}

void Light::setBatteryLight(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    mBatteryState = state;
    setSpeakerBatteryLightLocked();
}

void Light::setNotificationLight(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    mNotificationState = state;
    setSpeakerBatteryLightLocked();
}

void Light::setSpeakerBatteryLightLocked() {
    ALOGE("YIISSS");

    if (isLit(mNotificationState)) {
        setSpeakerLightLocked(mNotificationState);
    } else if (isLit(mAttentionState)) {
        setSpeakerLightLocked(mAttentionState);
    } else if (isLit(mBatteryState)) {
        ALOGE("Battery");
        setSpeakerLightLocked(mBatteryState);
    } else {
        ALOGE("Off");
        /* Lights off */
        mRedLed << 0;
        mGreenLed << 0;
        mBlueLed << 0;
        mRedBlink << 0;
        mGreenBlink << 0;
        mBlueBlink << 0;
    }
}

void Light::setSpeakerLightLocked(const LightState& state) {
    uint8_t onMs, offMs;
    uint8_t stepDuration, pauseHi;
    uint8_t red, green, blue, blink;

    switch (state.flashMode) {
        case Flash::TIMED:
            onMs = state.flashOnMs;
            offMs = state.flashOffMs;
            break;
        default:
            onMs = 0;
            offMs = 0;
            break;
    }

    red = (state.color >> 16) & 0xFF;
    green = (state.color >> 8) & 0xFF;
    blue = state.color & 0xFF;
    blink = onMs > 0 && offMs > 0;

    ALOGE("%d %d %d %d", red, green, blue, blink);

    // Disable all blinking
    mRedBlink << 0;
    mGreenBlink << 0;
    mBlueBlink << 0;

    if (blink) {
        stepDuration = RAMP_STEP_DURATION;
        pauseHi = onMs - (stepDuration * RAMP_SIZE * 2);
        if (stepDuration * RAMP_SIZE * 2 > onMs) {
            stepDuration = onMs / (RAMP_SIZE * 2);
            pauseHi = 0;
        }

        mRedStartIdx << 0;
        mRedDutyPcts << get_scaled_duty_pcts(red);
        mRedPauseLo << offMs;
        mRedPauseHi << pauseHi;
        mRedRampStepMs << stepDuration;

        mGreenStartIdx << RAMP_SIZE;
        mGreenDutyPcts << get_scaled_duty_pcts(green);
        mGreenPauseLo << offMs;
        mGreenPauseHi << pauseHi;
        mGreenRampStepMs << stepDuration;

        mBlueStartIdx << RAMP_SIZE * 2;
        mBlueDutyPcts << get_scaled_duty_pcts(blue);
        mBluePauseLo << offMs;
        mBluePauseHi << pauseHi;
        mBlueRampStepMs << stepDuration;

        if (red) {
            mRedBlink << 1;
        }
        if (green) {
            mGreenBlink << 1;
        }
        if (blue) {
            mBlueBlink << 1;
        }
    } else {
        ALOGE("Full led");
        mRedLed << std::to_string(red);
        mGreenLed << std::to_string(green);
        mBlueLed << std::to_string(blue);
    }
}

} // namespace implementation
}  // namespace V2_0
}  // namespace light
}  // namespace hardware
}  // namespace android
