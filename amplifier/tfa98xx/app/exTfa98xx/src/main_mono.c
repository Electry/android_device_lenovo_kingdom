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
#define LOCATION_FILES "/etc/tfa98xx/"
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

static void load_all_settings(Tfa98xx_handle_t handle, Tfa98xx_SpeakerParameters_t speakerBytes, const char* configFile, const char* presetFile, const char* eqFile)
{
    Tfa98xx_Error_t err;

    /* load fullmodel */
    err = Tfa98xx_DspWriteSpeakerParameters(handle, sizeof(Tfa98xx_SpeakerParameters_t), speakerBytes);
    if (err != Tfa98xx_Error_Ok) {
        TFA_LOGE("Tfa98xx_DspWriteSpeakerParameters failed(%d)", err);
    }

    /* load the settings */
    setConfig(handle, configFile);
    /* load a preset */
    setPreset(handle, presetFile);
    /* load an EQ file */
    setEQ(handle, eqFile);
}

int audio_smartpa_enable(char enable)
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle;
    int fd = 0;
    int ret = 0;
    Tfa98xx_Mute_t mute;
	float vol;

	TFA_LOGD("audio_smartpa_enable = %d", enable);
	if(enable){
		ret = audio_smartpa_enable_clk(1);
		if (0 != ret) {
			ALOGE("%s:%u  error = %d",__func__, __LINE__,ret);
			return ret;
		}
		tfa9890_SpeakerOn();
	}else{
		tfa9890_SpeakerOff();
		audio_smartpa_enable_clk(0);
	}

    return 0;
}

int audio_smartpa_enable_clk(char enable)
{
	int fd, ret=0;

	fd = open("/dev/tfa9890", O_RDWR | O_NONBLOCK, 0);
    if ( fd < 0 ) {
        TFA_LOGE("Can't open i2c /dev/tfa9890 fd:%d, errno:%d\n",fd, errno);
        return -1;
    }
	if(enable){
	    TFA_LOGD("To ENABLE_MI2S_CLK\n");
	    ret = ioctl(fd, ENABLE_MI2S_CLK, 1);
	    if ( ret < 0 ) {
	        TFA_LOGE( "Can't ioctl i2c\n ret:%d", ret);
	        return -1;
	    }
	}else{
		TFA_LOGD("To DISABLE_MI2S_CLK\n");
		ret = ioctl(fd, ENABLE_MI2S_CLK, 0);
	    if ( ret < 0 ) {
	        TFA_LOGE("Can't ioctl to set i2c\n");
	        return -1;
	    }
	}
	ret = close(fd);
	if ( ret < 0 ) {
        TFA_LOGE("Can't close i2c\n");
        return -1;
    }
    return 0;
}
int audio_smartpa_volume_set(int voldB)
{
	int fd, ret=0;
	FIXEDPT vol;
    Tfa98xx_handle_t handle;

	fd = open("/dev/tfa9890", O_RDWR | O_NONBLOCK, 0);
    if ( ret < 0 ) {
        TFA_LOGE("Can't open i2c /dev/tfa9890\n");
        return -1;
    }

    TFA_LOGD("To ENABLE_MI2S_CLK\n");
    ret = ioctl(fd, ENABLE_MI2S_CLK, 1);
    if ( ret < 0 ) {
        TFA_LOGE( "Can't ioctl i2c\n");
        return -1;
    }
	/* use the generic slave address for optimizations */
    /* create handle */
    ret = Tfa98xx_Open(I2C_ADDRESS, &handle);
    if ( ret != Tfa98xx_Error_Ok)
    {
        TFA_LOGD("Unable to connect to the device. Check the I2C address and/or device is powered ON.\n");
    }
    assert(ret == Tfa98xx_Error_Ok);
	TFA_LOGD("SmartPA set Volume:%d", voldB);
	ret = Tfa98xx_SetVolume(handle, voldB);
	assert(ret == Tfa98xx_Error_Ok);
	ret = Tfa98xx_GetVolume(handle, &vol);
	TFA_LOGD("SmartPA Volume:%d", vol);
	ret = Tfa98xx_Close(handle);
    assert(ret == Tfa98xx_Error_Ok);
	TFA_LOGD("To DISABLE_MI2S_CLK\n");
	ret = ioctl(fd, ENABLE_MI2S_CLK, 0);
    if ( ret < 0 ) {
        TFA_LOGE("Can't ioctl to set i2c\n");
        return -1;
    }

	ret = close(fd);
	if ( ret < 0 ) {
        TFA_LOGE("Can't close i2c\n");
        return -1;
    }
    return 0;

}

int audio_smartpa_enable_init(char enable)
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle;
	int fd, ret;
	int tries = 0;
    int ready = 0;
    Tfa98xx_Mute_t mute;
	float vol;

	TFA_LOGD("[%s][%d]", __func__, __LINE__);
	fd = open("/dev/tfa9890", O_RDWR | O_NONBLOCK, 0);
    if ( ret < 0 ) {
        TFA_LOGE("Can't open i2c /dev/tfa9890\n");
        return -1;
    }
    TFA_LOGD("To ENABLE_MI2S_CLK\n");
    ret = ioctl(fd, ENABLE_MI2S_CLK, 1);
    if ( ret < 0 ) {
        TFA_LOGE( "Can't ioctl i2c\n");
        return -1;
    }

	TFA_LOGD("audio_smartpa_enable = %d", enable);
	/* use the generic slave address for optimizations */
    /* create handle */
    err = Tfa98xx_Open(I2C_ADDRESS, &handle);
    if ( err != Tfa98xx_Error_Ok)
    {
        TFA_LOGD("Unable to connect to the device. Check the I2C address and/or device is powered ON.\n");
    }
    assert(err == Tfa98xx_Error_Ok);

	if(enable){
		//tfa9890_SpeakerOn();

		err = Tfa98xx_SetSampleRate(handle, 48000);
		ALOGI("%s Tfa98xx_SetSampleRate:%d",__func__, 48000);

		err = Tfa98xx_Powerdown(handle, 0);

	    if(err != Tfa98xx_Error_Ok) {
	        TFA_LOGD("Tfa98xx_Powerdown to 0 error(%d)", err);
	    }
	    // Sleep 15ms to wait for DSP stabled

	    do {
	        err = Tfa98xx_DspSystemStable(handle, &ready);
	        assert(err == Tfa98xx_Error_Ok);
	        tries++;
	    } while ((0 == ready) && (tries < 100));
	    TFA_LOGD("%s : %u : err = 0x%08x tries = %d", __func__, __LINE__, err, tries);

		err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Off);
		assert(err == Tfa98xx_Error_Ok);
	}else{
		//tfa9890_SpeakerOff();
	    muteAmplifier(handle);
	    err = Tfa98xx_Powerdown(handle, 1);
	    if(err != Tfa98xx_Error_Ok) {
	        TFA_LOGD("Tfa98xx_Powerdown to 1 error(%d)", err);
	    }
	}

	err = Tfa98xx_Close(handle);
    assert(err == Tfa98xx_Error_Ok);

	TFA_LOGD("To DISABLE_MI2S_CLK\n");
	err = ioctl(fd, ENABLE_MI2S_CLK, 0);
    if ( ret < 0 ) {
        TFA_LOGE("Can't ioctl to set i2c\n");
        return -1;
    }

	err = close(fd);
	if ( ret < 0 ) {
        TFA_LOGE("Can't close i2c\n");
        return -1;
    }
    return 0;
}

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
	ALOGD("TFA98xx ID=%#x");
	err = Tfa98xx_Close(handle);
}

int main(int argc, char* argv[])
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle;
    int i, fd, ret;
    Tfa98xx_Mute_t mute;
    Tfa98xx_SpeakerParameters_t lsModel;
    Tfa98xx_StateInfo_t stateInfo;
    Tfa98xx_SpeakerParameters_t loadedSpeaker;
    unsigned short status;

    float re25;
	int val=0;
	int id = 0;
    int calibrateDone = 0;

    if(argc >= 1){
        if(strcmp("SmartPA ON", argv[1])==0){
            TFA_LOGD("To set SMARTPA ON\n");
            audio_smartpa_enable_init(1);
        }else if(strcmp("SmartPA OFF", argv[1])==0){
            TFA_LOGD("To set SMARTPA OFF\n");
            audio_smartpa_enable_init(0);
        }else if(strcmp("SmartPA CLK_ON", argv[1])==0){
			audio_smartpa_enable_clk(1);
		}else if(strcmp("SmartPA CLK_OFF", argv[1])==0){
			audio_smartpa_enable_clk(0);
		}else if(strcmp("SmartPA Volume", argv[1])==0){
			val= atoi(argv[2]);
			audio_smartpa_volume_set(val);
		}else if(strcmp("SmartPA ID", argv[1])==0){
			read_tfa98xx_id(&id);
		}
    }
return Tfa98xx_Error_Ok;
#if 0
    /* use the generic slave address for optimizations */
    /* create handle */
    err = Tfa98xx_Open(I2C_ADDRESS, &handle);
    if ( err != Tfa98xx_Error_Ok)
    {
        TFA_LOGD("Unable to connect to the device. Check the I2C address and/or device is powered ON.\n");
    }
    assert(err == Tfa98xx_Error_Ok);
#if 0         // uncomment below code if scanning for I2C devices is required
    unsigned char slave_address;
   /* try all possible addresses, stop at the first found */
    for (slave_address = 0x68; slave_address <= 0x6E ; slave_address+=2)
    {
        TFA_LOGD("Trying slave address 0x%x\n", slave_address);
        err = Tfa98xx_Open(slave_address, &handle);
        if (err == Tfa98xx_Error_Ok)
        {
            break;
        }
    }
#endif
    /* should have found a device */
    assert(err == Tfa98xx_Error_Ok);

#ifdef SPDIF_AUDIO
    InitSpdifAudio();
#endif
#ifdef USB_AUDIO
    InitUsbAudio();
#endif

#ifdef     PATCH_FILENAME
    setPatch(LOCATION_FILES PATCH_FILENAME);
#endif
	fd = open("/dev/tfa9890", O_RDWR | O_NONBLOCK, 0);
    if ( ret < 0 ) {
        TFA_LOGD( "Can't open i2c\n");
        return -1;
    }
    TFA_LOGD("To ENABLE_MI2S_CLK\n");
    ret = ioctl(fd, ENABLE_MI2S_CLK, 1);
    if ( ret < 0 ) {
        TFA_LOGD( "Can't ioctl i2c\n");
        return -1;
    }
   /* cold boot, need to load all parameters and patches */
   coldStartup(handle, SAMPLE_RATE, LOCATION_FILES "coldboot.patch");
   /*Set to calibration once*/
   /* Only needed for really first time calibration */
   setOtc(handle, 1);

   loadSpeakerFile(LOCATION_FILES SPEAKER_FILENAME, loadedSpeaker);

   /* Check if MTPEX bit is set for calibration once mode */
   if(checkMTPEX(handle) == 0)
   {
       TFA_LOGD("DSP not yet calibrated. Calibration will start.\n");

       /* ensure no audio during special calibration */
       err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);
       assert(err == Tfa98xx_Error_Ok);
    }
    else
    {
        TFA_LOGD("DSP already calibrated. Calibration skipped and previous calibration results loaded from MTP.\n");
    }

    /* Load all settings (for TFA9887: this is the 2nd time. Now speaker model contains tCoefA. */
    load_all_settings(handle, loadedSpeaker, LOCATION_FILES CONFIG_FILENAME, LOCATION_FILES PRESET_FILENAME, LOCATION_FILES EQ_FILENAME);

    /* do calibration (again), if needed */
    err = Tfa98xx_SetConfigured(handle);
    assert(err == Tfa98xx_Error_Ok);

   /* Wait until the calibration is done.
   * The MTPEX bit would be set and remain as 1 if MTPOTC is set to 1 */
   waitCalibration(handle, &calibrateDone);
   if (calibrateDone)
   {
      Tfa98xx_DspGetCalibrationImpedance(handle,&re25);
   }
   else
   {
      re25 = 0;
   }
   TFA_LOGD("Calibration value is %2.2f ohm\n", re25);

   /*Checking the current status for DSP status and DCPVP */
   statusCheck(handle);

   /*Remark:*/
   /*Reset the MTPEX bit to do the calibration for new speaker*/
   /*resetMtpEx(handle);*/

    err = ioctl(fd, ENABLE_MI2S_CLK, 0);
    if ( ret < 0 ) {
        TFA_LOGD( "Can't ioctl to set i2c\n");
        return -1;
    }

	err = Tfa98xx_Close(handle);
    assert(err == Tfa98xx_Error_Ok);

	audio_smartpa_enable(0);
    return err;
#endif
}

int EQset_Impl(int mode)
{
    ALOGI("%s mode: %d -> %d+",__func__, g_EQMode, mode);
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle = g_handle;

	if (handle != -1) {
	    switch(mode){
	        case 0:{//normal
	            setPreset(handle,"/etc/tfa98xx/Lenovo_HQ.preset");
	            setEQ(handle,"/etc/tfa98xx/Lenovo_HQ.eq");
	            ALOGI("[%s] set normal preset and eq.", __func__);
	            break;
	        }
			case 1:{//incall
			    setPreset(handle,"/etc/tfa98xx/Lenovo_LOUD.preset");
	            setEQ(handle,"/etc/tfa98xx/Lenovo_LOUD.eq");
	            ALOGI("[%s] set incall preset and eq.", __func__);
				break;
			}
			case 2:{//incall brightness
			    setPreset(handle,"/etc/tfa98xx/Lenovo_LOUD_BT.preset");
	            setEQ(handle,"/etc/tfa98xx/Lenovo_LOUD_BT.eq");
	            ALOGI("[%s] set incall preset and eq.", __func__);
				break;
			}
	        default:
	            ALOGI("[%s] mode error!", __func__);
	            break;
	    }

	    usleep(1);
	}
EXIT:
    ALOGI("%s -",__func__);
    return 1;
}

int tfa9890_EQset(int mode)
{
    ALOGI("%s mode: %d -> %d+",__func__, g_EQMode, mode);
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle = g_handle;
    if (-1 != handle) {
		pthread_mutex_lock(&g_tfaMutex);
	    if((mode != g_EQMode) && g_CalibrationStatus)   {
	        g_EQMode = mode;
			g_EQSwitch = 1;
	    } else {
	        ALOGI("[tfa9890_EQset] the same mode return.\n");
	    }
		pthread_mutex_unlock(&g_tfaMutex);
    }
    ALOGI("%s -",__func__);
    return 1;
}


int tfa9890_deinit(void)
{
    Tfa98xx_handle_t handle = g_handle;

    if (handle != -1) {
        Tfa98xx_Error_t err = Tfa98xx_Error_Ok;

        muteAmplifier(handle);

        err = Tfa98xx_Powerdown(handle, 1);
        assert(err == Tfa98xx_Error_Ok);

        err = Tfa98xx_Close(handle);
        assert(err == Tfa98xx_Error_Ok);

        pthread_mutexattr_destroy(&g_tfaMutex);
    }

    g_handle = -1;

	g_CalibrationStatus = 0;
    return 1;
}

int init(int sRate)
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle;
    Tfa98xx_SpeakerParameters_t loadedSpeaker;
    Tfa98xx_StateInfo_t stateInfo;
    unsigned short status = 0;
    float re25 = 0.0, tCoefA = 0.0;
    int calibrateDone = 0;
    int rc = 1;

    ALOGI("%s +",__func__);

    if(g_handle != -1)
    {
        ALOGI("%s already init,skip it",__func__);
        rc = 1;
        goto EXIT;
    }

    if (!((Tfa98xx_Supported_08000 == sRate)
        ||(Tfa98xx_Supported_16000 == sRate)
        ||(Tfa98xx_Supported_44100 == sRate)
        ||(Tfa98xx_Supported_48000 == sRate))) {
        ALOGD("%s Samplerate = %d is not support",__func__, sRate);
        rc = -1;
        goto EXIT;
    }

    /* create handle */
    err = Tfa98xx_Open(I2C_ADDRESS, &handle);
    if(err != Tfa98xx_Error_Ok) {
        ALOGE("[%s] Tfa98xx_Open failed! err = %d, I2C_ADDRESS = %#x", __func__, err,I2C_ADDRESS);
        rc = -1;
        goto EXIT;
    }

	g_handle = handle;

    rc = pthread_mutex_init(&g_tfaMutex, NULL);
    if (rc != 0) {
        ALOGE("Failed to initialize pthread tfaMutex");
        goto EXIT;
    }
	pthread_mutex_lock(&g_tfaMutex);
#ifdef     PATCH_FILENAME
    setPatch(LOCATION_FILES PATCH_FILENAME);
#endif

    /* cold boot, need to load all parameters and patches */
    coldStartup(handle, sRate, LOCATION_FILES "coldboot.patch");

	/*set AEC for tfa98xx*/
	/*set DCDC for Default value 0x8fe5 */
	Tfa98xx_WriteRegister16(handle,TFA98XX_AUDIO_CTR, 0x10f);
	Tfa98xx_WriteRegister16(handle,TFA98XX_DCDCBOOST, 0x8fe5);
	Tfa98xx_SelectI2SOutputLeft(handle,Tfa98xx_I2SOutputSel_DSP_AEC);
	Tfa98xx_SelectI2SOutputRight(handle, Tfa98xx_I2SOutputSel_DSP_AEC);
	Tfa98xx_SelectI2SOutput(handle,TFA98XX_I2S_SEL_REG_SPKR, 0x0);

    g_sRate = sRate;

    /* load current speaker file to structure */
    loadSpeakerFile(LOCATION_FILES SPEAKER_FILENAME, loadedSpeaker);

    /*Set to calibration once*/
    /* Only needed for really first time calibration */
    setOtc(handle, 1);

    /* Check if MTPEX bit is set for calibration once mode */
    if(checkMTPEX(handle) == 0)
    {
        TFA_LOGD("DSP not yet calibrated. Calibration will start.\n");

        /* ensure no audio during special calibration */
        err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);
        if (err != Tfa98xx_Error_Ok) {
            TFA_LOGE("Tfa98xx_SetMute to Tfa98xx_Mute_Digital failed(%d)", err);
        }
    }
    else
    {
        TFA_LOGD("DSP already calibrated. Calibration skipped and previous calibration results loaded from MTP.\n");
    }

    /* Load all settings (for TFA9887: this is the 2nd time. Now speaker model contains tCoefA. */
    load_all_settings(handle, loadedSpeaker, LOCATION_FILES CONFIG_FILENAME, LOCATION_FILES PRESET_FILENAME, LOCATION_FILES EQ_FILENAME);

    /* all settings loaded, signal the DSP to start calibration */
    err = Tfa98xx_SetConfigured(handle);
    if (err != Tfa98xx_Error_Ok) {
        TFA_LOGE("Tfa98xx_SetConfigured error");
    }

    waitCalibration(handle, &calibrateDone);
    if (calibrateDone)
    {
        Tfa98xx_DspGetCalibrationImpedance(handle,&re25);
        TFA_LOGD("Calibration value is %2.2f ohm\n", re25);
        /*Checking the current status for DSP status and DCPVP */
        statusCheck(handle);
        err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);
        assert(err == Tfa98xx_Error_Ok);
    }
    else
    {
        re25 = 0;
        TFA_LOGE("Calibration failed\n");
    }

    if((re25 < TFA98XX_NOMINAL_IMPEDANCE_MIN)
        || (re25 > TFA98XX_NOMINAL_IMPEDANCE_MAX))    {
        TFA_LOGD("Calibration Value error, reset MtpEx");
		g_CalibrationStatus = 0;
        resetMtpEx(handle);
	    muteAmplifier(handle);
	    err = Tfa98xx_Powerdown(handle, 1);
    } else {
	    /* check LS model */
	    err = Tfa98xx_DspReadSpeakerParameters(handle, sizeof(Tfa98xx_SpeakerParameters_t), loadedSpeaker);
	    assert(err == Tfa98xx_Error_Ok);
	    tCoefA = tCoefFromSpeaker(loadedSpeaker);
	    TFA_LOGD("current calibration tCoefA: %f", tCoefA);

	    err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);

	    if(err == Tfa98xx_Error_Ok) {
	        ALOGI("%s init ok",__func__);
	    }
		g_CalibrationStatus = 1;
    }
EXIT:
    ALOGI("%s -",__func__);
	pthread_mutex_unlock(&g_tfaMutex);
    return rc;
}

int tfa9890_init(int sRate)
{
	int rc = 0;

	rc = audio_smartpa_enable_clk(1);
	if (0 != rc) {
		ALOGE("%s:%u  error = %d",__func__, __LINE__,rc);
		return rc;
	}
	rc = init(sRate);
	if (0 != rc) {
		ALOGE("%s:%u init error = %d",__func__, __LINE__,rc);
		return rc;
	}
	audio_smartpa_enable_clk(0);
	return rc;
}

int tfa9890_init_N(int sRate)
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle;
    Tfa98xx_SpeakerParameters_t loadedSpeaker;
    Tfa98xx_StateInfo_t stateInfo;
    unsigned short status = 0;
    float re25 = 0.0, tCoefA = 0.0;
    int calibrateDone = 0;
	int fd, ret;
    int rc = 1;

    ALOGI("%s +",__func__);

	TFA_LOGD("[%s][%d]", __func__, __LINE__);
	fd = open("/dev/tfa9890", O_RDWR | O_NONBLOCK, 0);
    if ( ret < 0 ) {
        TFA_LOGE("Can't open i2c /dev/tfa9890\n");
        return -1;
    }
    TFA_LOGD("To ENABLE_MI2S_CLK\n");
    ret = ioctl(fd, ENABLE_MI2S_CLK, 1);
    if ( ret < 0 ) {
        TFA_LOGE( "Can't ioctl i2c\n");
        return -1;
    }
    if(g_handle != -1)
    {
        ALOGI("%s already init,skip it",__func__);
        rc = 1;
        goto EXIT;
    }

    if (!((Tfa98xx_Supported_08000 == sRate)
        ||(Tfa98xx_Supported_16000 == sRate)
        ||(Tfa98xx_Supported_44100 == sRate)
        ||(Tfa98xx_Supported_48000 == sRate))) {
        ALOGD("%s Samplerate = %d is not support",__func__, sRate);
        rc = -1;
        goto EXIT;
    }

    /* create handle */
    err = Tfa98xx_Open(I2C_ADDRESS, &handle);
    if(err != Tfa98xx_Error_Ok) {
        ALOGE("[%s] Tfa98xx_Open failed!", __func__);
        rc = -1;
        goto EXIT;
    }

	g_handle = handle;

    rc = pthread_mutex_init(&g_tfaMutex, NULL);
    if (rc != 0) {
        ALOGE("Failed to initialize pthread tfaMutex");
        goto EXIT;
    }
	pthread_mutex_lock(&g_tfaMutex);
#ifdef     PATCH_FILENAME
    setPatch(LOCATION_FILES PATCH_FILENAME);
#endif

    /* cold boot, need to load all parameters and patches */
    coldStartup(handle, sRate, LOCATION_FILES "coldboot.patch");

    g_sRate = sRate;

    /* load current speaker file to structure */
    loadSpeakerFile(LOCATION_FILES SPEAKER_FILENAME, loadedSpeaker);

    /*Set to calibration once*/
    /* Only needed for really first time calibration */
    setOtc(handle, 1);

    /* Check if MTPEX bit is set for calibration once mode */
    if(checkMTPEX(handle) == 0)
    {
        TFA_LOGD("DSP not yet calibrated. Calibration will start.\n");

        /* ensure no audio during special calibration */
        err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);
        if (err != Tfa98xx_Error_Ok) {
            TFA_LOGE("Tfa98xx_SetMute to Tfa98xx_Mute_Digital failed(%d)", err);
        }
    }
    else
    {
        TFA_LOGD("DSP already calibrated. Calibration skipped and previous calibration results loaded from MTP.\n");
    }

    /* Load all settings (for TFA9887: this is the 2nd time. Now speaker model contains tCoefA. */
    load_all_settings(handle, loadedSpeaker, LOCATION_FILES CONFIG_FILENAME, LOCATION_FILES PRESET_FILENAME, LOCATION_FILES EQ_FILENAME);

    /* all settings loaded, signal the DSP to start calibration */
    err = Tfa98xx_SetConfigured(handle);
    if (err != Tfa98xx_Error_Ok) {
        TFA_LOGE("Tfa98xx_SetConfigured error");
    }

    waitCalibration(handle, &calibrateDone);
    if (calibrateDone)
    {
        Tfa98xx_DspGetCalibrationImpedance(handle,&re25);
        TFA_LOGD("Calibration value is %2.2f ohm\n", re25);
        /*Checking the current status for DSP status and DCPVP */
        statusCheck(handle);
        err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);
        assert(err == Tfa98xx_Error_Ok);
    }
    else
    {
        re25 = 0;
        TFA_LOGE("Calibration failed\n");
    }

    if((re25 < TFA98XX_NOMINAL_IMPEDANCE_MIN)
        || (re25 > TFA98XX_NOMINAL_IMPEDANCE_MAX))    {
        TFA_LOGD("Calibration Value error, reset MtpEx");
        resetMtpEx(handle); 
		//tfa9890_deinit(handle);
		tfa9890_deinit();
		//Todo: switch from high to low to rst pin
    } else {
	    /* check LS model */
	    err = Tfa98xx_DspReadSpeakerParameters(handle, sizeof(Tfa98xx_SpeakerParameters_t), loadedSpeaker);
	    assert(err == Tfa98xx_Error_Ok);
	    tCoefA = tCoefFromSpeaker(loadedSpeaker);
	    TFA_LOGD("current calibration tCoefA: %f", tCoefA);
		err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);
	    err = Tfa98xx_Powerdown(handle, 1);
	    ALOGI("%s init ok",__func__);
		g_CalibrationStatus = 1;
		g_SpeakerOn =0;
    }	
EXIT:
    ALOGI("%s -",__func__);
	pthread_mutex_unlock(&g_tfaMutex);

	TFA_LOGD("To DISABLE_MI2S_CLK\n");
	err = ioctl(fd, ENABLE_MI2S_CLK, 0);
    if ( ret < 0 ) {
        TFA_LOGE("Can't ioctl to set i2c\n");
        return -1;
    }
    return rc;
}


Tfa98xx_Error_t ReColdStartup(Tfa98xx_handle_t handle, int sRate, int mode)
{
    ALOGI("%s +",__func__);
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_SpeakerParameters_t loadedSpeaker;

    float re25 = 0.0;
    int calibrateDone = 0;

    /* use the generic slave address for optimizations */
    /* calibrate behand the tfa9890_init,
           we execute tfa9890_init in the AudioMTKHardware init*/
    if (handle != -1) {
		goto EXIT;
    }

    /* cold boot, need to load all parameters and patches */
    coldStartup(handle, sRate, LOCATION_FILES "coldboot.patch");

    /*Set to calibration once*/
    /* Only needed for really first time calibration */

    //resetMtpEx(handle);
    loadSpeakerFile(LOCATION_FILES SPEAKER_FILENAME, loadedSpeaker);

    /* Check if MTPEX bit is set for calibration once mode */
    if(checkMTPEX(handle) == 0)
    {
        ALOGI("DSP not yet calibrated. Calibration will start.\n");

        /* ensure no audio during special calibration */
        err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);
        assert(err == Tfa98xx_Error_Ok);
    }
    else
    {
        ALOGI("DSP already calibrated. Calibration skipped and previous calibration results loaded from MTP.\n");
    }


    /* Load all settings (for TFA9887: this is the 2nd time. Now speaker model contains tCoefA. */
	switch(mode){
        case 0:{//normal
			load_all_settings(handle, loadedSpeaker, LOCATION_FILES CONFIG_FILENAME,
				LOCATION_FILES PRESET_FILENAME, LOCATION_FILES EQ_FILENAME);
            ALOGI("[%s] set normal preset and eq.", __func__);
            break;
        }
    //TBD: TODO
        default:
            ALOGI("[%s] mode error!", __func__);
            break;
    }

    usleep(1);

    /* do calibration  */
    err = Tfa98xx_SetConfigured(handle);
    assert(err == Tfa98xx_Error_Ok);

    /* Wait until the calibration is done.
    * The MTPEX bit would be set and remain as 1 if MTPOTC is set to 1 */
    waitCalibration(handle, &calibrateDone);
	if (calibrateDone)
    {
        Tfa98xx_DspGetCalibrationImpedance(handle,&re25);
        TFA_LOGD("Calibration value is %2.2f ohm\n", re25);
        /*Checking the current status for DSP status and DCPVP */
        statusCheck(handle);
        err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);
        assert(err == Tfa98xx_Error_Ok);
    }
    else
    {
        re25 = 0;
        TFA_LOGE("Calibration failed\n");
    }

    if((re25 < TFA98XX_NOMINAL_IMPEDANCE_MIN)
        || (re25 > TFA98XX_NOMINAL_IMPEDANCE_MAX))    {
        TFA_LOGD("Calibration Value error, reset MtpEx and, do not open device");
		g_CalibrationStatus = 0;
        resetMtpEx(handle);
		muteAmplifier(handle);
	    err = Tfa98xx_Powerdown(handle, 1);
		err = Tfa98xx_Error_Other;
    } else {
    	err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Off);
	    if(err != Tfa98xx_Error_Ok) {
	        TFA_LOGD("Tfa98xx_SetMute to Tfa98xx_Mute_Off error(%d)", err);
	    }
    }
EXIT:
    ALOGI("%s -",__func__);
    return err;
}

void tfa9890_SpeakerOn(void)
{
    TFA_LOGD("+");

	Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle = g_handle;
    int mtp = 0;
    int status = 0;
    int tries = 0;
    int ready = 0;
	int rc = 0;
	int sRate = 0;

    if (-1 == handle) {
	    rc = init(g_sRate);
		if (0 != rc) {
			ALOGE("%s:%u init error = %d",__func__, __LINE__,rc);
			return;
		}
		handle = g_handle;
	    pthread_mutex_lock(&g_tfaMutex);

	    ALOGI("%s after init g_SpeakerOn = %d",__func__, g_SpeakerOn);
	    g_SpeakerOn = 1;

	    // Sleep 15ms to wait for DSP stabled
	    do {
	        err = Tfa98xx_DspSystemStable(handle, &ready);
	        assert(err == Tfa98xx_Error_Ok);
	        tries++;
	    } while ((0 == ready) && (tries < 100));
	    ALOGI("%s : %u : err = 0x%08x tries = %d", __func__, __LINE__, err, tries);

	    err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
	    err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
	    TFA_LOGD("%s: %u:err = 0x%08x status val:0x%04x mtp val:0x%04x\n",
	        __func__, __LINE__, err, status, mtp);

	    if ((status & TFA98XX_STATUSREG_ACS) == TFA98XX_STATUSREG_ACS)    {
	        TFA_LOGD("%s: %u: ACS was trigged\n", __func__, __LINE__);
	        err = ReColdStartup(handle, g_sRate, g_EQMode);
  	    } else {
	    	err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Off);
		    if(err != Tfa98xx_Error_Ok) {
		        TFA_LOGD("Tfa98xx_SetMute to Tfa98xx_Mute_Off error(%d)", err);
		    }
	    }

EXIT_INIT:
	    g_EQSwitch = 0;
	    pthread_mutex_unlock(&g_tfaMutex);
	} else {
	    pthread_mutex_lock(&g_tfaMutex);
        if (g_CalibrationStatus) {
		    if(g_SpeakerOn)
		    {
		        ALOGI("%s speaker already open return",__func__);
		        goto EXIT_NORMAL;
		    }
		    g_SpeakerOn = 1;

			err = Tfa98xx_GetSampleRate( handle, &sRate );
			ALOGI("%s sRate = %d",__func__, sRate);
			if (sRate != 0 && sRate != g_sRate) {
				err = Tfa98xx_SetSampleRate(handle, g_sRate);
				ALOGI("%s Tfa98xx_SetSampleRate:%d",__func__, g_sRate);
			}
			err = Tfa98xx_Powerdown(handle, 0);

		    if(err != Tfa98xx_Error_Ok) {
		        TFA_LOGD("Tfa98xx_Powerdown to 0 error(%d)", err);
		    }
		    // Sleep 15ms to wait for DSP stabled
#if 0
		    usleep(TFA98XX_DSP_STABLE_TIME);
#else
		    do {
		        err = Tfa98xx_DspSystemStable(handle, &ready);
		        assert(err == Tfa98xx_Error_Ok);
		        tries++;
		    } while ((0 == ready) && (tries < 100));
		    TFA_LOGD("%s : %u : err = 0x%08x tries = %d", __func__, __LINE__, err, tries);

		    err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
		    err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
		    TFA_LOGD("%s: %u:err = 0x%08x status val:0x%04x mtp val:0x%04x\n",
		        __func__, __LINE__, err, status, mtp);

		    if ((status & TFA98XX_STATUSREG_ACS) == TFA98XX_STATUSREG_ACS)    {
		        TFA_LOGD("%s: %u: ACS was trigged\n", __func__, __LINE__);
		        ReColdStartup(handle, g_sRate, g_EQMode);
				g_EQSwitch = 0;
		    }
#endif
            if(1 == g_EQSwitch) {
				EQset_Impl(g_EQMode);
				g_EQSwitch = 0;
			}

			err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Off);
			assert(err == Tfa98xx_Error_Ok);
	    }else {
			TFA_LOGD(" Calibration status abnormal", __func__, __LINE__);
		}
EXIT_NORMAL:
	    pthread_mutex_unlock(&g_tfaMutex);
    }
    TFA_LOGD("-");
}

void tfa9890_SpeakerOff(void)
{
    TFA_LOGD("+");
	Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handle = g_handle;
	if (handle != -1) {
	    pthread_mutex_lock(&g_tfaMutex);

	    if(g_SpeakerOn && g_CalibrationStatus)
	    {
		    muteAmplifier(handle);

			g_SpeakerOn = 0;

		    err = Tfa98xx_Powerdown(handle, 1);

		    if(err != Tfa98xx_Error_Ok) {
		        TFA_LOGD("Tfa98xx_Powerdown to 1 error(%d)", err);
		    }
	    } else {
	    	ALOGI("%s speaker already off return", __func__);
	    }
EXIT:
	    pthread_mutex_unlock(&g_tfaMutex);
	}
    TFA_LOGD("-");
}

//set samplerate at tfa9890 powerdown
void tfa9890_setSamplerate(int sRate)
{
    ALOGI("%s samplerate:%d +",__func__, sRate);
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;

    if ((Tfa98xx_Supported_08000 == sRate)
        ||(Tfa98xx_Supported_16000 == sRate)
      ||(Tfa98xx_Supported_44100 == sRate)
      ||(Tfa98xx_Supported_48000 == sRate)) {
        g_sRate = sRate;
    } else {
        ALOGI("%s Samplerate = %d is not support",__func__, sRate);
    }
    ALOGI("%s -",__func__);
}

EXPORT_SYMBOL(tfa9890_init);
EXPORT_SYMBOL(tfa9890_deinit);
EXPORT_SYMBOL(tfa9890_EQset);
EXPORT_SYMBOL(tfa9890_SpeakerOff);
EXPORT_SYMBOL(tfa9890_SpeakerOn);
EXPORT_SYMBOL(tfa9890_setSamplerate);
