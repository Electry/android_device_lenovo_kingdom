/*
 * Copyright 2017 The LineageOS Project
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

#define LOG_TAG "android.hardware.light@2.0-service.kingdom"

#include <hidl/HidlTransportSupport.h>
#include <utils/Errors.h>

#include "Light.h"

// libhwbinder:
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

// Generated HIDL files
using android::hardware::light::V2_0::ILight;
using android::hardware::light::V2_0::implementation::Light;

const static std::string kBacklightPath = "/sys/class/leds/lcd-backlight/brightness";

const static std::string kRedLedPath = "/sys/class/leds/led:rgb_red/brightness";
const static std::string kGreenLedPath = "/sys/class/leds/led:rgb_green/brightness";
const static std::string kBlueLedPath = "/sys/class/leds/led:rgb_blue/brightness";

const static std::string kRedDutyPctsPath = "/sys/class/leds/led:rgb_red/duty_pcts";
const static std::string kGreenDutyPctsPath = "/sys/class/leds/led:rgb_green/duty_pcts";
const static std::string kBlueDutyPctsPath = "/sys/class/leds/led:rgb_blue/duty_pcts";

const static std::string kRedStartIdxPath = "/sys/class/leds/led:rgb_red/start_idx";
const static std::string kGreenStartIdxPath = "/sys/class/leds/led:rgb_green/start_idx";
const static std::string kBlueStartIdxPath = "/sys/class/leds/led:rgb_blue/start_idx";

const static std::string kRedPauseLoPath = "/sys/class/leds/led:rgb_red/pause_lo";
const static std::string kGreenPauseLoPath = "/sys/class/leds/led:rgb_green/pause_lo";
const static std::string kBluePauseLoPath = "/sys/class/leds/led:rgb_blue/pause_lo";

const static std::string kRedPauseHiPath = "/sys/class/leds/led:rgb_red/pause_hi";
const static std::string kGreenPauseHiPath = "/sys/class/leds/led:rgb_green/pause_hi";
const static std::string kBluePauseHiPath = "/sys/class/leds/led:rgb_blue/pause_hi";

const static std::string kRedRampStepMsPath = "/sys/class/leds/led:rgb_red/ramp_step_ms";
const static std::string kGreenRampStepMsPath = "/sys/class/leds/led:rgb_green/ramp_step_ms";
const static std::string kBlueRampStepMsPath = "/sys/class/leds/led:rgb_blue/ramp_step_ms";

const static std::string kRedBlinkPath = "/sys/class/leds/led:rgb_red/blink";
const static std::string kGreenBlinkPath = "/sys/class/leds/led:rgb_green/blink";
const static std::string kBlueBlinkPath = "/sys/class/leds/led:rgb_blue/blink";

int main() {
    std::ofstream backlight(kBacklightPath);
    if (!backlight) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kBacklightPath.c_str(), error, strerror(error));
        return -error;
    }

    // Brightness
    std::ofstream redLed(kRedLedPath);
    if (!redLed) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kRedLedPath.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream greenLed(kGreenLedPath);
    if (!greenLed) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kGreenLedPath.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream blueLed(kBlueLedPath);
    if (!blueLed) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kBlueLedPath.c_str(), error, strerror(error));
        return -error;
    }

    // DutyPcts
    std::ofstream redDutyPcts(kRedDutyPctsPath);
    if (!redDutyPcts) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kRedDutyPctsPath.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream greenDutyPcts(kGreenDutyPctsPath);
    if (!greenDutyPcts) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kGreenDutyPctsPath.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream blueDutyPcts(kBlueDutyPctsPath);
    if (!blueDutyPcts) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kBlueDutyPctsPath.c_str(), error, strerror(error));
        return -error;
    }

    // StartIdx
    std::ofstream redStartIdx(kRedStartIdxPath);
    if (!redStartIdx) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kRedStartIdxPath.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream greenStartIdx(kGreenStartIdxPath);
    if (!greenStartIdx) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kGreenStartIdxPath.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream blueStartIdx(kBlueStartIdxPath);
    if (!blueStartIdx) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kBlueStartIdxPath.c_str(), error, strerror(error));
        return -error;
    }

    // PauseLo
    std::ofstream redPauseLo(kRedPauseLoPath);
    if (!redPauseLo) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kRedPauseLoPath.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream greenPauseLo(kGreenPauseLoPath);
    if (!greenPauseLo) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kGreenPauseLoPath.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream bluePauseLo(kBluePauseLoPath);
    if (!bluePauseLo) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kBluePauseLoPath.c_str(), error, strerror(error));
        return -error;
    }

    // PauseHi
    std::ofstream redPauseHi(kRedPauseHiPath);
    if (!redPauseHi) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kRedPauseHiPath.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream greenPauseHi(kGreenPauseHiPath);
    if (!greenPauseHi) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kGreenPauseHiPath.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream bluePauseHi(kBluePauseHiPath);
    if (!bluePauseHi) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kBluePauseHiPath.c_str(), error, strerror(error));
        return -error;
    }

    // RampStepMs
    std::ofstream redRampStepMs(kRedRampStepMsPath);
    if (!redRampStepMs) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kRedRampStepMsPath.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream greenRampStepMs(kGreenRampStepMsPath);
    if (!greenRampStepMs) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kGreenRampStepMsPath.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream blueRampStepMs(kBlueRampStepMsPath);
    if (!blueRampStepMs) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kBlueRampStepMsPath.c_str(), error, strerror(error));
        return -error;
    }

    // Blink
    std::ofstream redBlink(kRedBlinkPath);
    if (!redBlink) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kRedBlinkPath.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream greenBlink(kGreenBlinkPath);
    if (!greenBlink) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kGreenBlinkPath.c_str(), error, strerror(error));
        return -error;
    }

    std::ofstream blueBlink(kBlueBlinkPath);
    if (!blueBlink) {
        int error = errno;
        ALOGE("Failed to open %s (%d): %s", kBlueBlinkPath.c_str(), error, strerror(error));
        return -error;
    }

    android::sp<ILight> service = new Light(std::move(backlight),
                                            // Brightness
                                            std::move(redLed),
                                            std::move(greenLed),
                                            std::move(blueLed),
                                            // DutyPcts
                                            std::move(redDutyPcts),
                                            std::move(greenDutyPcts),
                                            std::move(blueDutyPcts),
                                            // StartIdx
                                            std::move(redStartIdx),
                                            std::move(greenStartIdx),
                                            std::move(blueStartIdx),
                                            // PauseLo
                                            std::move(redPauseLo),
                                            std::move(greenPauseLo),
                                            std::move(bluePauseLo),
                                            // PauseHi
                                            std::move(redPauseHi),
                                            std::move(greenPauseHi),
                                            std::move(bluePauseHi),
                                            // RampStepMs
                                            std::move(redRampStepMs),
                                            std::move(greenRampStepMs),
                                            std::move(blueRampStepMs),
                                            // Blink
                                            std::move(redBlink),
                                            std::move(greenBlink),
                                            std::move(blueBlink));

    configureRpcThreadpool(1, true);

    android::status_t status = service->registerAsService();

    if (status != android::OK) {
        ALOGE("Cannot register Light HAL service");
        return 1;
    }

    ALOGI("Light HAL Ready.");
    joinRpcThreadpool();
    // Under normal cases, execution will not reach this line.
    ALOGE("Light HAL failed to join thread pool.");
    return 1;
}
