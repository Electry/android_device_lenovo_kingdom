#include <Tfa98xx.h>
#include <Tfa98xx_Registers.h>
#include <assert.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>                                                                                                            
#include <fcntl.h>
#include <math.h>
#include <sys/stat.h>
#include <i2c-dev.h> 
#include <NXP_I2C.h>
#ifndef WIN32
// need PIN access
#include <lxScribo.h>
#endif

#include "common.h"
#include "audioif.h"

#ifdef WIN32
// cwd = dir where vcxproj is
#define LOCATION_FILES "../../../../settings/"
#else
// cwd = linux dir
#define LOCATION_FILES "/vendor/etc/tfa98xx/"
#endif

/* the base speaker file, containing tCoef */
#define SPEAKER_FILENAME "Lenovo.speaker"
#define PRESET_FILENAME "Lenovo_HQ.preset"
#define CONFIG_FILENAME "TFA9890_N1B12_N1C3_v2.config"
#define EQ_FILENAME "Lenovo_HQ.eq"
#define PATCH_FILENAME "TFA9890_N1C3_1_7_1.patch"
// nor B12 #define PATCH_FILENAME "TFA9890_N1B12_3_2_3.patch"

typedef enum Tfa98xx_Supported_Sample_Rate {
    Tfa98xx_Supported_08000 =  8000,
    Tfa98xx_Supported_16000 = 16000,
    Tfa98xx_Supported_44100 = 44100,
    Tfa98xx_Supported_48000 = 48000
}Tfa98xx_Supported_Sample_Rate_t;

extern regdef_t regdefs[];
extern unsigned char  tfa98xxI2cSlave; // global for i2c access

#include <utils/threads.h>
static pthread_mutex_t g_tfaMutex;
static Tfa98xx_handle_t g_handle = -1;
static int g_EQMode = 0;
static int g_SpeakerOn = 0;
static int g_sRate = 44100;
static int g_CalibrationStatus = 0;
static int g_EQSwitch = 0;

/* *INDENT-OFF* */
regdef_t regdefs[] = {
        { 0x00, 0x081d, 0xfeff, "statusreg"}, //ignore MTP busy bit
        { 0x01, 0x0, 0x0, "batteryvoltage"},
        { 0x02, 0x0, 0x0, "temperature"},
        { 0x03, 0x0012, 0xffff, "revisionnumber"},
        { 0x04, 0x888b, 0xffff, "i2sreg"},
        { 0x05, 0x13aa, 0xffff, "bat_prot"},
        { 0x06, 0x001f, 0xffff, "audio_ctr"},
        { 0x07, 0x0fe6, 0xffff, "dcdcboost"},
        { 0x08, 0x0800, 0x3fff, "spkr_calibration"}, //NOTE: this is a software fix to 0xcoo
        { 0x09, 0x041d, 0xffff, "sys_ctrl"},
        { 0x0a, 0x3ec3, 0x7fff, "i2s_sel_reg"},
        { 0x40, 0x0, 0x00ff, "hide_unhide_key"},
        { 0x41, 0x0, 0x0, "pwm_control"},
        { 0x46, 0x0, 0x0, "currentsense1"},
        { 0x47, 0x0, 0x0, "currentsense2"},
        { 0x48, 0x0, 0x0, "currentsense3"},
        { 0x49, 0x0, 0x0, "currentsense4"},
        { 0x4c, 0x0, 0xffff, "abisttest"},
        { 0x62, 0x0, 0, "mtp_copy"},
        { 0x70, 0x0, 0xffff, "cf_controls"},
        { 0x71, 0x0, 0, "cf_mad"},
        { 0x72, 0x0, 0, "cf_mem"},
        { 0x73, 0x00ff, 0xffff, "cf_status"},
        { 0x80, 0x0, 0, "mtp"},
        { 0x83, 0x0, 0, "mtp_re0"},
        { 0xff, 0,0, NULL}
};

int audio_smartpa_enable_clk(char enable);

void read_tfa98xx_id(int *id)
{
	Tfa98xx_handle_t handle;
	int err;
    /* create handle */
    err = Tfa98xx_Open(I2C_ADDRESS, &handle);
    if(err != Tfa98xx_Error_Ok) {
        ALOGE("[%s] Tfa98xx_Open failed! err = %d, I2C_ADDRESS = %#x", __func__, err,I2C_ADDRESS);
    }
	err = Tfa98xx_ReadRegister16(handle, TFA98XX_REVISIONNUMBER, id);
	ALOGD("TFA98xx ID=%#x",*id);
	err = Tfa98xx_Close(handle);
}
