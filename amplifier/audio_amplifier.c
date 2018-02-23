/*
 * Copyright (C) 2013-2014, The CyanogenMod Project
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


#include <time.h>
#include <system/audio.h>
#include <platform.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <dlfcn.h>
#include <sys/ioctl.h>

#define LOG_TAG "tfa98xx"
#include <log/log.h>

#include <hardware/audio_amplifier.h>

static amplifier_device_t *tfa9890_dev = NULL;

#define SAMPLE_RATE 48000
int audio_smartpa_enable(char enable);
int tfa9890_EQset(int mode);
int tfa9890_init(int sRate);

static int is_speaker(uint32_t snd_device) {
    int speaker = 0;

    switch (snd_device) {
        case SND_DEVICE_OUT_SPEAKER:
        case SND_DEVICE_OUT_SPEAKER_REVERSE:
        case SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES:
        case SND_DEVICE_OUT_VOICE_SPEAKER:
        case SND_DEVICE_OUT_SPEAKER_AND_HDMI:
        case SND_DEVICE_OUT_SPEAKER_AND_USB_HEADSET:
        case SND_DEVICE_OUT_SPEAKER_AND_ANC_HEADSET:
            speaker = 1;
            break;
    }

    return speaker;
}

static int is_voice_speaker(uint32_t snd_device) {
    return snd_device == SND_DEVICE_OUT_VOICE_SPEAKER;
}

static int amp_enable_output_devices(hw_device_t *device, uint32_t devices, bool enable) {
    amplifier_device_t *tfa9890 = (amplifier_device_t*) device;

    if (is_speaker(devices)) {
        if (enable) {
            audio_smartpa_enable(1);
            if (is_voice_speaker(devices)) {
                tfa9890_EQset(1);
            } else {
                tfa9890_EQset(0);
            }
        } else {
            audio_smartpa_enable(0);
        }
    }
    return 0;
}

static int amp_dev_close(hw_device_t *device) {
    amplifier_device_t *tfa9890 = (amplifier_device_t*) device;

    if (tfa9890) {
        free(tfa9890);
    }

    return 0;
}

static int amp_init(amplifier_device_t *tfa9890) {
    size_t i;
    int subscribe = 1;

    tfa9890_init(SAMPLE_RATE);

    return 0;
}

static int amp_module_open(const hw_module_t *module,
        __attribute__((unused)) const char *name, hw_device_t **device)
{
    if (tfa9890_dev) {
        ALOGE("%s:%d: Unable to open second instance of TFA9890 amplifier\n",
                __func__, __LINE__);
        return -EBUSY;
    }

    tfa9890_dev = calloc(1, sizeof(amplifier_device_t));
    if (!tfa9890_dev) {
        ALOGE("%s:%d: Unable to allocate memory for amplifier device\n",
                __func__, __LINE__);
        return -ENOMEM;
    }

    tfa9890_dev->common.tag = HARDWARE_DEVICE_TAG;
    tfa9890_dev->common.module = (hw_module_t *) module;
    tfa9890_dev->common.version = HARDWARE_DEVICE_API_VERSION(1, 0);
    tfa9890_dev->common.close = amp_dev_close;

    tfa9890_dev->enable_output_devices = amp_enable_output_devices;

    if (amp_init(tfa9890_dev)) {
        free(tfa9890_dev);
        return -ENODEV;
    }

    *device = (hw_device_t *) tfa9890_dev;

    return 0;
}

static struct hw_module_methods_t hal_module_methods = {
    .open = amp_module_open,
};

amplifier_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = AMPLIFIER_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = AMPLIFIER_HARDWARE_MODULE_ID,
        .name = "K920 audio amplifier HAL",
        .author = "The LineageOS Project",
        .methods = &hal_module_methods,
    },
};
