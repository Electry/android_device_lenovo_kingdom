/*
 * Copyright (C) 2016 The CyanogenMod Project
 *           (C) 2017 The LineageOS Project
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

package org.lineageos.hardware;

import org.lineageos.internal.util.FileUtils;

/**
 * Adaptive backlight support (this refers to technologies like NVIDIA SmartDimmer,
 * QCOM CABL or Samsung CABC).
 */
public class AdaptiveBacklight {
    private static final String CONTROL_PATH = "/sys/class/graphics/fb0/cabc";

    /**
     * Whether device supports an adaptive backlight technology.
     *
     * @return boolean Supported devices must return always true
     */
    public static boolean isSupported() {
        return FileUtils.isFileWritable(CONTROL_PATH);
    }

    /**
     * This method return the current activation status of the adaptive backlight technology.
     *
     * @return boolean Must be false when adaptive backlight is not supported or not activated, or
     * the operation failed while reading the status; true in any other case.
     */
    public static boolean isEnabled() {
        return Integer.parseInt(FileUtils.readOneLine(CONTROL_PATH)) > 0 ? true : false;
    }

    /**
     * This method allows to setup adaptive backlight technology status.
     *
     * @param status The new adaptive backlight status
     * @return boolean Must be false if adaptive backlight is not supported or the operation
     * failed; true in any other case.
     */
    public static boolean setEnabled(boolean status) {
        return FileUtils.writeLine(CONTROL_PATH, (status ? "3" : "0")); // CABC 3
    }

}
