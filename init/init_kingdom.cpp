/*
   Copyright (c) 2017-2018, The LineageOS Project

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include <android-base/strings.h>
#include <android-base/file.h>
#include <android-base/logging.h>

#include "vendor_init.h"
#include "property_service.h"

using android::base::Trim;
using android::base::ReadFileToString;
using android::init::property_set;

#define LOG_TAG         "init_kingdom"

#define HWID_PATH       "/sys/class/lenovo/nv/nv_hwid"

#define RETRY_MS        500
#define RETRY_COUNT     20


void property_override(char const prop[], char const value[])
{
    prop_info *pi;

    pi = (prop_info*) __system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

void property_override_dual(char const system_prop[], char const vendor_prop[], char const value[])
{
    property_override(system_prop, value);
    property_override(vendor_prop, value);
}

void vendor_load_properties()
{
    std::string hwid;
    std::string device;

    int retry = RETRY_COUNT;

    while (retry && (!ReadFileToString(HWID_PATH, &hwid) || !hwid.length())) {
        retry--;
        LOG(INFO) << LOG_TAG << ": Waiting for nv_hwid...";
        usleep(RETRY_MS * 1000);
    }

    if (!retry) {
        LOG(ERROR) << LOG_TAG << ": Failed to read hwid";
        goto set_variant_row;
    }

    LOG(INFO) << LOG_TAG << ": Found hwid=" << hwid;

    if (Trim(hwid) == "0001") {
        /* China */
        device = "kingdomt";
        property_override("ro.product.model", "K920 (CN)");

        property_set("persist.radio.multisim.config", "dsda");

        property_override("ro.build.description",
            "kingdomt-user 5.0.2 LRX22G VIBEUI_V2.5_1627_5.1894.1_ST_K920 release-keys");
        property_override("ro.build.fingerprint",
            "Lenovo/kingdomt/kingdomt:5.0.2/LRX22G/VIBEUI_V2.5_1627_5.1894.1_ST_K920:user/release-keys");

    } else if (Trim(hwid) == "0100") {
set_variant_row:
        /* Rest of the World */
        device = "kingdom_row";
        property_override("ro.product.model", "K920 (ROW)");

        property_set("persist.radio.multisim.config", "dsds");

        property_override("ro.build.description",
            "kingdom_row-user 5.0.2 LRX22G K920_S288_160224_ROW release-keys");
        property_override("ro.build.fingerprint",
            "Lenovo/kingdom_row/kingdom_row:5.0.2/LRX22G/K920_S288_160224_ROW:user/release-keys");

    } else {
        LOG(ERROR) << LOG_TAG << ": Unknown hwid=" << hwid;
        goto set_variant_row;
    }

    property_override("ro.build.product", device.c_str());
    property_override("ro.product.device", device.c_str());
    property_override("ro.product.name", device.c_str());

    // LTE+3G+2G on both SIMs
    property_set("ro.telephony.default_network", "9,9");

    LOG(INFO) << LOG_TAG << ": Build properties set for " << device << " device";
}

