/*
 * Copyright (C) 2017 The LineageOS Project
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

enum {
    PROFILE_POWER_SAVE = 0,
    PROFILE_BALANCED,
    PROFILE_HIGH_PERFORMANCE,
    PROFILE_BIAS_POWER_SAVE,
    PROFILE_BIAS_PERFORMANCE,
    PROFILE_MAX
};

typedef struct governor_settings {
    int boost_allow;
    /* interactive */
    int boostpulse_duration;
    int go_hispeed_load;
    int go_hispeed_load_off;
    int hispeed_freq;
    int hispeed_freq_off;
    int io_is_busy;
    int min_sample_time;
    int sampling_down_factor;
    char *target_loads;
    char *target_loads_off;
    /* cpufreq */
    int scaling_max_freq;
    int scaling_min_freq;
    int scaling_min_freq_off;
    /* cpuboost */
    int input_boost_freq;
    int input_boost_ms;
    /* kgsl */
    int max_gpuclk;
} power_profile;

static power_profile profiles[PROFILE_MAX] = {
    [PROFILE_POWER_SAVE] = {
        .boost_allow =            0,
        /* interactive */
        .boostpulse_duration =    0,
        .go_hispeed_load =        95,
        .go_hispeed_load_off =    95,
        .hispeed_freq =           729600,
        .hispeed_freq_off =       729600,
        .io_is_busy =             0,
        .min_sample_time =        60000,
        .sampling_down_factor =   100000,
        .target_loads =           "95",
        .target_loads_off =       "95",
        /* cpufreq */
        .scaling_max_freq =       1267200,
        .scaling_min_freq =       300000,
        .scaling_min_freq_off =   300000,
        /* cpuboost */
        .input_boost_freq =       0,
        .input_boost_ms =         0,
        /* kgsl */
        .max_gpuclk =             389000000,
    },
    [PROFILE_BALANCED] = {
        .boost_allow =            1,
        /* interactive */
        .boostpulse_duration =    200000,
        .go_hispeed_load =        95,
        .go_hispeed_load_off =    95,
        .hispeed_freq =           1497600,
        .hispeed_freq_off =       960000,
        .io_is_busy =             1,
        .min_sample_time =        40000,
        .sampling_down_factor =   100000,
        .target_loads =           "70 1497600:90 1958400:93 2265600:96",
        .target_loads_off =       "90 1574400:99",
        /* cpufreq */
        .scaling_max_freq =       2457600,
        .scaling_min_freq =       300000,
        .scaling_min_freq_off =   300000,
        /* cpuboost */
        .input_boost_freq =       1497600,
        .input_boost_ms =         40,
        /* kgsl */
        .max_gpuclk =             578000000,
    },
    [PROFILE_HIGH_PERFORMANCE] = {
        .boost_allow =            1,
        /* interactive */
        .boostpulse_duration =    1000000,
        .go_hispeed_load =        75,
        .go_hispeed_load_off =    75,
        .hispeed_freq =           1958400,
        .hispeed_freq_off =       1190400,
        .io_is_busy =             1,
        .min_sample_time =        40000,
        .sampling_down_factor =   100000,
        .target_loads =           "70",
        .target_loads_off =       "70",
        /* cpufreq */
        .scaling_max_freq =       2457600,
        .scaling_min_freq =       1497600,
        .scaling_min_freq_off =   300000,
        /* cpuboost */
        .input_boost_freq =       1728000,
        .input_boost_ms =         60,
        /* kgsl */
        .max_gpuclk =             578000000,
    },
    [PROFILE_BIAS_POWER_SAVE] = {
        .boost_allow =            1,
        /* interactive */
        .boostpulse_duration =    100000,
        .go_hispeed_load =        95,
        .go_hispeed_load_off =    95,
        .hispeed_freq =           960000,
        .hispeed_freq_off =       960000,
        .io_is_busy =             1,
        .min_sample_time =        40000,
        .sampling_down_factor =   100000,
        .target_loads =           "70 960000:85 1190400:90 1267200:92 "
                                  "1497600:94 1574400:96 1728000:98",
        .target_loads_off =       "95",
        /* cpufreq */
        .scaling_max_freq =       1958400,
        .scaling_min_freq =       300000,
        .scaling_min_freq_off =   300000,
        /* cpuboost */
        .input_boost_freq =       1190400,
        .input_boost_ms =         40,
        /* kgsl */
        .max_gpuclk =             578000000,
    },
    [PROFILE_BIAS_PERFORMANCE] = {
        .boost_allow =            1,
        /* interactive */
        .boostpulse_duration =    500000,
        .go_hispeed_load =        90,
        .go_hispeed_load_off =    90,
        .hispeed_freq =           1574400,
        .hispeed_freq_off =       1190400,
        .io_is_busy =             1,
        .min_sample_time =        40000,
        .sampling_down_factor =   100000,
        .target_loads =           "70 1574400:80 1958400:85 2265600:90",
        .target_loads_off =       "90",
        /* cpufreq */
        .scaling_max_freq =       2457600,
        .scaling_min_freq =       300000,
        .scaling_min_freq_off =   300000,
        /* cpuboost */
        .input_boost_freq =       1574400,
        .input_boost_ms =         60,
        /* kgsl */
        .max_gpuclk =             578000000,
    },
};
