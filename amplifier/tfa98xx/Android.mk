LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/hal/inc \
    $(LOCAL_PATH)/tfa/inc \
    $(LOCAL_PATH)/hal/src/lxScribo \
    $(LOCAL_PATH)

LOCAL_SRC_FILES := \
    hal/src/NXP_I2C_linux.c  \
    hal/src/lxScribo/lxScribo.c \
    hal/src/lxScribo/lxDummy.c  \
    hal/src/lxScribo/lxScriboSerial.c  \
    hal/src/lxScribo/lxScriboSocket.c\
    hal/src/lxScribo/lxI2c.c \
    hal/src/lxScribo/scribosrv/i2cserver.c \
    hal/src/lxScribo/scribosrv/cmd.c

LOCAL_MODULE := libtfahal
LOCAL_SHARED_LIBRARIES := libcutils libutils
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/tfa/inc \
    $(LOCAL_PATH)/hal/inc \
    $(LOCAL_PATH)/hal/src \
    $(LOCAL_PATH)/hal/src/lxScribo \
    $(LOCAL_PATH)/hal/src/lxScribo/scribosrv

LOCAL_SRC_FILES := \
    tfa/src/initTfa9890.c \
    tfa/src/initTfa9890N1B12.c \
    tfa/src/Tfa98xx.c \
    tfa/src/Tfa98xx_TextSupport.c

LOCAL_MODULE := libtfa
LOCAL_SHARED_LIBRARIES := libcutils libutils
LOCAL_STATIC_LIBRARIES := libtfahal
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/srv/inc \
    $(LOCAL_PATH)/tfa/inc \
    $(LOCAL_PATH)/hal/inc \
    $(LOCAL_PATH)/hal/src \
    $(LOCAL_PATH)/hal/src/lxScribo \
    $(LOCAL_PATH)

LOCAL_SRC_FILES := \
    srv/src/nxpTfa98xx.c \
    srv/src/tfa98xxRuntime.c \
    srv/src/tfa98xxCalibration.c \
    srv/src/tfa98xxDiagnostics.c \
    srv/src/tfa98xxLiveData.c

LOCAL_MODULE := libtfasrv
LOCAL_SHARED_LIBRARIES := libcutils libutils
LOCAL_STATIC_LIBRARIES := libtfahal libtfa
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES:= \
    $(LOCAL_PATH)/srv/inc \
    $(LOCAL_PATH)/tfa/inc \
    $(LOCAL_PATH)/hal/inc \
    $(LOCAL_PATH)/hal/src \
    $(LOCAL_PATH)/hal/src/lxScribo \
    $(LOCAL_PATH)/app/exTfa98xx/inc

LOCAL_SRC_FILES := \
    app/exTfa98xx/src/main_mono.c \
    app/exTfa98xx/src/common.c

LOCAL_MODULE := libtfa98xx
LOCAL_SHARED_LIBRARIES := libcutils libutils liblog
LOCAL_STATIC_LIBRARIES := libtfasrv libtfa libtfahal
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/srv/inc \
    $(LOCAL_PATH)/tfa/inc \
    $(LOCAL_PATH)/hal/inc \
    $(LOCAL_PATH)/hal/src \
    $(LOCAL_PATH)/hal/src/lxScribo \
    $(LOCAL_PATH)/app/exTfa98xx/inc

LOCAL_SRC_FILES := \
    app/exTfa98xx/src/main_mono.c \
    app/exTfa98xx/src/common.c

LOCAL_MODULE := climax_init
LOCAL_SHARED_LIBRARIES:= libcutils libutils liblog
LOCAL_STATIC_LIBRARIES:= libtfasrv libtfa libtfahal
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/app/climax/src/cli \
    $(LOCAL_PATH)/app/climax/inc \
    $(LOCAL_PATH)/srv/inc \
    $(LOCAL_PATH)/tfa/inc \
    $(LOCAL_PATH)/hal/inc \
    $(LOCAL_PATH)/hal/src \
    $(LOCAL_PATH)

LOCAL_SRC_FILES := \
    app/climax/src/climax.c \
    app/climax/src/cliCommands.c \
    app/climax/src/cli/cmdline.c

LOCAL_MODULE := climax
LOCAL_SHARED_LIBRARIES := libcutils libutils
LOCAL_STATIC_LIBRARIES := libtfasrv libtfa libtfahal
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/srv/inc \
    $(LOCAL_PATH)/tfa/inc \
    $(LOCAL_PATH)/hal/inc \
    $(LOCAL_PATH)/hal/src \
    $(LOCAL_PATH)/hal/src/lxScribo \
    $(LOCAL_PATH)/app/exTfa98xx/inc

LOCAL_SRC_FILES := \
    app/exTfa98xx/src/mono_static.c \
    tfa/src/Tfa98xx.c \
    hal/src/NXP_I2C_linux.c  \
    app/exTfa98xx/src/common.c \
    hal/src/lxScribo/lxScribo.c \
    hal/src/lxScribo/lxDummy.c  \
    hal/src/lxScribo/lxScriboSerial.c  \
    hal/src/lxScribo/lxScriboSocket.c \
    hal/src/lxScribo/lxI2c.c \
    hal/src/lxScribo/scribosrv/i2cserver.c \
    hal/src/lxScribo/scribosrv/cmd.c

LOCAL_MODULE := climax_static
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

