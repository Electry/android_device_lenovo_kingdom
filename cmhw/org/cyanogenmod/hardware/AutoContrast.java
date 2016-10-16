/*
 * Copyright (C) 2015 The CyanogenMod Project
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

package org.cyanogenmod.hardware;

/**
 * Auto contrast optimization support
 */
public class AutoContrast {

    /**
     * Whether device supports auto contrast optimization.
     *
     * @return boolean Supported devices must return always true
     */
    public static boolean isSupported() {
        return false;
    }

    /**
     * This method return the current activation status of the auto contrast optimization.
     *
     * @return boolean Must be false when color enhancement is not supported or not activated, or
     * the operation failed while reading the status; true in any other case.
     */
    public static boolean isEnabled() {
        return false;
    }

    /**
     * This method allows to setup auto contrast optimization.
     *
     * @param status The new auto contrast status
     * @return boolean Must be false if auto contrast is not supported or the operation
     * failed; true in any other case.
     */
    public static boolean setEnabled(boolean status) {
        return false;
    }
}
