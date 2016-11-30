/*
   Copyright (c) 2016, The CyanogenMod Project

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

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <cutils/klog.h>
#include <cutils/properties.h>

#define LOG_TAG	"init_kingdom"

#define HWID_PATH       "/sys/class/lenovo/nv/nv_hwid"
#define HWID_SIZE       4

#define PROP_SIZE	64

static int read_file2(const char *fname, char *data, int max_size)
{
    int fd, rc;

    if (max_size < 1)
        return 0;

    fd = open(fname, O_RDONLY);
    if (fd < 0) {
        KLOG_ERROR(LOG_TAG, "Failed to open '%s'\n", fname);
        return 0;
    }

    rc = read(fd, data, max_size - 1);
    if ((rc > 0) && (rc < max_size))
        data[rc] = '\0';
    else
        data[0] = '\0';
    close(fd);

    return 1;
}

int main()
{
    char hwid[HWID_SIZE];
    char device[PROP_SIZE];

    if (!read_file2(HWID_PATH, hwid, HWID_SIZE)) {
        KLOG_ERROR(LOG_TAG, "Failed to read hwid, defaulting to ROW variant\n");
        goto set_variant_row;
    }

    if (strncmp(hwid, "0001", HWID_SIZE) == 0) {
        /* China */
        strncpy(device, "kingdomt", PROP_SIZE);

        property_set("ro.build.product", device);
        property_set("ro.product.model", "K920 (CN)");
        property_set("ro.product.device", device);
        property_set("ro.product.name", device);

        property_set("ro.telephony.default_network", "20,20");
        property_set("persist.radio.multisim.config", "dsda");

        property_set("ro.build.description",
            "kingdomt-user 5.0.2 LRX22G VIBEUI_V2.5_1627_5.1894.1_ST_K920 release-keys");
        property_set("ro.build.fingerprint",
            "Lenovo/kingdomt/kingdomt:5.0.2/LRX22G/VIBEUI_V2.5_1627_5.1894.1_ST_K920:user/release-keys");

    } else if (strncmp(hwid, "0100", HWID_SIZE) == 0) {
set_variant_row:
        /* Rest of the World */
        strncpy(device, "kingdom_row", PROP_SIZE);

        property_set("ro.build.product", device);
        property_set("ro.product.model", "K920 (ROW)");
        property_set("ro.product.device", device);
        property_set("ro.product.name", device);

        property_set("ro.telephony.default_network", "20,1");
        property_set("persist.radio.multisim.config", "dsds");

        property_set("ro.build.description",
            "kingdom_row-user 5.0.2 LRX22G K920_S288_160224_ROW release-keys");
        property_set("ro.build.fingerprint",
            "Lenovo/kingdom_row/kingdom_row:5.0.2/LRX22G/K920_S288_160224_ROW:user/release-keys");

    } else {
        KLOG_ERROR(LOG_TAG, "Unknown hwid [%s], defaulting to ROW variant\n",
                    hwid);
        goto set_variant_row;
    }

    KLOG_INFO(LOG_TAG, "Found hwid [%s] setting build properties for [%s] device\n",
                hwid, device);

    return 0;
}

