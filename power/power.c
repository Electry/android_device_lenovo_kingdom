/*
 * Copyright (C) 2017-2018 The LineageOS Project
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
#define LOG_TAG "PowerHAL"

#include <hardware/hardware.h>
#include <hardware/power.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#include <log/log.h>
#include <cutils/properties.h>

#include "power.h"

#define GESTURE_NODE_PATH    "/sys/class/touchscreen/device/gesture"

#define CPUFREQ_LIMIT_PATH   "/sys/kernel/cpufreq_limit/"
#define INTERACTIVE_PATH     "/sys/devices/system/cpu/cpufreq/interactive/"
#define CPUBOOST_PATH        "/sys/module/cpu_boost/parameters/"
#define KGSL_PATH            "/sys/class/kgsl/kgsl-3d0/"

#define PROFILE_PROP         "persist.sys.perf.profile"

#define BUF_SIZE             80

static int boostpulse_fd = -1;
static int boost_fd = -1;
static int current_power_profile = PROFILE_BALANCED;

static int sysfs_write_str(char *path, char *s)
{
    char buf[BUF_SIZE];
    int len;
    int ret = 0;
    int fd;

    fd = open(path, O_WRONLY);
    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return -1;
    }

    len = write(fd, s, strlen(s));
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);
        ret = -1;
    }

    close(fd);

    return ret;
}

static int sysfs_write_int(char *path, int value)
{
    char buf[BUF_SIZE];
    snprintf(buf, sizeof(buf), "%d", value);
    return sysfs_write_str(path, buf);
}

static int sysfs_open_w(char *path, int *fd)
{
    if (*fd < 0) {
        *fd = open(path, O_WRONLY);
    }

    return *fd;
}

static int is_profile_valid(int profile)
{
    return profile >= 0 && profile < PROFILE_MAX;
}

static bool is_interactive(void)
{
    struct stat s;
    int err = stat(INTERACTIVE_PATH, &s);
    if (err != 0) return false;
    if (S_ISDIR(s.st_mode)) return true;
    return false;
}

/*
 * 1: lock min_freq to hispeed_freq
 * 0: set min_freq back to normal
 */
static void boost(bool on)
{
    char buf[BUF_SIZE];
    int len;

    if (sysfs_open_w(INTERACTIVE_PATH "boost", &boost_fd) >= 0) {
        snprintf(buf, sizeof(buf), "%d", on);

        len = write(boost_fd, &buf, sizeof(buf));
        if (len < 0) {
            strerror_r(errno, buf, sizeof(buf));
            ALOGE("Error writing to boost: %s\n", buf);

            close(boost_fd);
            boost_fd = -1;
        }
    }
}

/*
 * lock min_freq to hispeed_freq for boostpulse_duration (us)
 */
static void boostpulse()
{
    char buf[BUF_SIZE];
    int len;

    if (sysfs_open_w(INTERACTIVE_PATH "boostpulse",
            &boostpulse_fd) >= 0) {
        snprintf(buf, sizeof(buf), "%d", 1);

        len = write(boostpulse_fd, &buf, sizeof(buf));
        if (len < 0) {
            strerror_r(errno, buf, sizeof(buf));
            ALOGE("Error writing to boostpulse: %s\n", buf);

            close(boostpulse_fd);
            boostpulse_fd = -1;
        }
    }
}

void set_interactive(int on)
{
    if (!is_interactive())
        return;

    ALOGI("%s: setting interactive: %d", __func__, on);

    if (on) {
        /* interactive */
        sysfs_write_int(INTERACTIVE_PATH "hispeed_freq",
                        profiles[current_power_profile].hispeed_freq);
        sysfs_write_int(INTERACTIVE_PATH "go_hispeed_load",
                        profiles[current_power_profile].go_hispeed_load);
        sysfs_write_str(INTERACTIVE_PATH "target_loads",
                        profiles[current_power_profile].target_loads);
        /* cpufreq */
        sysfs_write_int(CPUFREQ_LIMIT_PATH "limited_min_freq",
                        profiles[current_power_profile].scaling_min_freq);
    } else {
        /* interactive */
        sysfs_write_int(INTERACTIVE_PATH "hispeed_freq",
                        profiles[current_power_profile].hispeed_freq_off);
        sysfs_write_int(INTERACTIVE_PATH "go_hispeed_load",
                        profiles[current_power_profile].go_hispeed_load_off);
        sysfs_write_str(INTERACTIVE_PATH "target_loads",
                        profiles[current_power_profile].target_loads_off);
        /* cpufreq */
        sysfs_write_int(CPUFREQ_LIMIT_PATH "limited_min_freq",
                        profiles[current_power_profile].scaling_min_freq_off);
    }
}

static void set_power_profile(int profile)
{
    char tmp_str[PROPERTY_VALUE_MAX];

    if (!is_profile_valid(profile)) {
        ALOGE("%s: unknown profile: %d", __func__, profile);
        return;
    }

    ALOGI("%s: setting profile: %d", __func__, profile);

    // Update persist property
    snprintf(tmp_str, sizeof(tmp_str), "%d", profile);
    property_set(PROFILE_PROP, tmp_str);

    /* interactive */
    sysfs_write_int(INTERACTIVE_PATH "boostpulse_duration",
                    profiles[profile].boostpulse_duration);
    sysfs_write_int(INTERACTIVE_PATH "go_hispeed_load",
                    profiles[profile].go_hispeed_load);
    sysfs_write_int(INTERACTIVE_PATH "hispeed_freq",
                    profiles[profile].hispeed_freq);
    sysfs_write_int(INTERACTIVE_PATH "io_is_busy",
                    profiles[profile].io_is_busy);
    sysfs_write_int(INTERACTIVE_PATH "min_sample_time",
                    profiles[profile].min_sample_time);
    sysfs_write_int(INTERACTIVE_PATH "sampling_down_factor",
                    profiles[profile].sampling_down_factor);
    sysfs_write_str(INTERACTIVE_PATH "target_loads",
                    profiles[profile].target_loads);

    /* cpufreq */
    sysfs_write_int(CPUFREQ_LIMIT_PATH "limited_max_freq",
                    profiles[profile].scaling_max_freq);
    sysfs_write_int(CPUFREQ_LIMIT_PATH "limited_min_freq",
                    profiles[profile].scaling_min_freq);

    /* cpuboost */
    sysfs_write_int(CPUBOOST_PATH "input_boost_freq",
                    profiles[profile].input_boost_freq);
    sysfs_write_int(CPUBOOST_PATH "input_boost_ms",
                    profiles[profile].input_boost_ms);

    /* kgsl */
    sysfs_write_int(KGSL_PATH "max_gpuclk",
                    profiles[profile].max_gpuclk);

    current_power_profile = profile;
}

void power_init(void)
{
    char tmp_str[PROPERTY_VALUE_MAX];
    int profile;

    ALOGI("%s", __func__);

    // Read persist property
    property_get(PROFILE_PROP, tmp_str, "1");
    sscanf(tmp_str, "%d", &profile);

    ALOGI("%s: Setting profile %d based on " PROFILE_PROP, __func__, profile);
    set_power_profile(profile);
}

void power_hint(power_hint_t hint, void *data)
{
    if (!is_interactive())
        return;

    switch (hint) {
    case POWER_HINT_VSYNC:
        break;

    case POWER_HINT_INTERACTION:
        /* input_boost used as equiv. */
        break;

    case POWER_HINT_LOW_POWER:
        ALOGV("POWER_HINT_LOW_POWER: %d", data ? *(int32_t *)data : 0);
        /* handled by the framework */
        break;

    case POWER_HINT_SUSTAINED_PERFORMANCE: // boost
        ALOGV("POWER_HINT_SUSTAINED_PERFORMANCE: %d", data ? *(int32_t *)data : 0);
        //if (!profiles[current_power_profile].boost_allow)
        //    break;
        //boost((*(bool *)data));
        break;

    case POWER_HINT_VR_MODE:
        ALOGV("POWER_HINT_VR_MODE: %d", data ? *(int32_t *)data : 0);
        break;

    case POWER_HINT_DISABLE_TOUCH:
        ALOGV("POWER_HINT_DISABLE_TOUCH: %d", data ? *(int32_t *)data : 0);
        break;

    case POWER_HINT_CPU_BOOST:
        ALOGV("POWER_HINT_CPU_BOOST: %dus", (*(int32_t *)data));
        if (!profiles[current_power_profile].boost_allow)
            break;
        boost(true);
        usleep((*(int32_t *)data));
        boost(false);
        break;

    case POWER_HINT_LAUNCH:
        ALOGV("POWER_HINT_LAUNCH");
        if (!profiles[current_power_profile].boostpulse_duration)
            break;
        boostpulse();
        break;

    case POWER_HINT_SET_PROFILE:
        ALOGV("POWER_HINT_SET_PROFILE: %d", (*(int32_t *)data));
        set_power_profile(*(int32_t *)data);
        break;

    default:
        break;
    }
}

void set_feature(feature_t feature, int state)
{
    switch (feature) {
    case POWER_FEATURE_DOUBLE_TAP_TO_WAKE:
        sysfs_write_int(GESTURE_NODE_PATH, state);
        break;
    default:
        ALOGW("%s: Feature %d doesn't exist\n", __func__, feature);
        break;
    }
}

int get_number_of_profiles()
{
    return PROFILE_MAX;
}
