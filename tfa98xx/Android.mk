LOCAL_PATH:= $(call my-dir)

#NOTE: for now we use static libs


############################## libhal
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= 	external/climax/hal/inc \
                        external/climax/tfa/inc \
			external/climax/hal/src/lxScribo \
			external/climax
LOCAL_SRC_FILES:= 	hal/src/NXP_I2C_linux.c  \
			hal/src/lxScribo/lxScribo.c \
			hal/src/lxScribo/lxDummy.c  \
			hal/src/lxScribo/lxScriboSerial.c  \
			hal/src/lxScribo/lxScriboSocket.c\
	                hal/src/lxScribo/lxI2c.c \
			hal/src/lxScribo/scribosrv/i2cserver.c \
			hal/src/lxScribo/scribosrv/cmd.c
LOCAL_MODULE := libtfahal
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

############################### libtfa
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= 	external/climax/tfa/inc\
		        external/climax/hal/inc\
			external/climax/hal/src\
			external/climax/hal/src/lxScribo\
                        external/climax/hal/src/lxScribo/scribosrv
LOCAL_SRC_FILES := 	\
			tfa/src/initTfa9890.c\
			tfa/src/initTfa9890N1B12.c\
			tfa/src/Tfa98xx.c\
			tfa/src/Tfa98xx_TextSupport.c

LOCAL_MODULE := libtfa
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libtfahal
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

############################### libsrv
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= 	external/climax/srv/inc\
                        external/climax/tfa/inc\
                        external/climax/hal/inc\
                        external/climax/hal/src\
                        external/climax/hal/src/lxScribo\
                        external/climax
LOCAL_SRC_FILES:= 	srv/src/nxpTfa98xx.c\
			srv/src/tfa98xxRuntime.c \
			srv/src/tfa98xxCalibration.c \
			srv/src/tfa98xxDiagnostics.c \
			srv/src/tfa98xxLiveData.c
LOCAL_MODULE := libtfasrv
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libtfahal libtfa
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

############################## libtfa9890
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= 	external/climax/srv/inc\
                        external/climax/tfa/inc\
                        external/climax/hal/inc\
                        external/climax/hal/src\
                        external/climax/hal/src/lxScribo\
                        external/climax/app/exTfa98xx/inc
LOCAL_SRC_FILES:= 	app/exTfa98xx/src/main_mono.c \
                    app/exTfa98xx/src/common.c
LOCAL_MODULE := libtfa98xx
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libtfasrv libtfa libtfahal
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

############################## tfa9890
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= 	external/climax/srv/inc\
                        external/climax/tfa/inc\
                        external/climax/hal/inc\
                        external/climax/hal/src\
                        external/climax/hal/src/lxScribo\
                        external/climax/app/exTfa98xx/inc
LOCAL_SRC_FILES:= 	app/exTfa98xx/src/main_mono.c \
                    app/exTfa98xx/src/common.c
LOCAL_MODULE := climax_init
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libtfasrv libtfa libtfahal
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_EXECUTABLE)

############################## cli app
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= 	external/climax/app/climax/src/cli\
                        external/climax/app/climax/inc\
                        external/climax/srv/inc\
                        external/climax/tfa/inc\
                        external/climax/hal/inc\
                        external/climax/hal/src\
                        external/climax
LOCAL_SRC_FILES:= 	app/climax/src/climax.c \
			app/climax/src/cliCommands.c \
			app/climax/src/cli/cmdline.c
LOCAL_MODULE := climax
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libtfasrv libtfa libtfahal
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_EXECUTABLE)

############################### tfa9890
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= 	external/climax/srv/inc\
                        external/climax/tfa/inc\
                        external/climax/hal/inc\
                        external/climax/hal/src\
                        external/climax/hal/src/lxScribo\
                        external/climax/app/exTfa98xx/inc
LOCAL_SRC_FILES:= 	app/exTfa98xx/src/mono_static.c \
	tfa/src/Tfa98xx.c\
	hal/src/NXP_I2C_linux.c  \
	app/exTfa98xx/src/common.c \
	hal/src/lxScribo/lxScribo.c \
	hal/src/lxScribo/lxDummy.c  \
	hal/src/lxScribo/lxScriboSerial.c  \
	hal/src/lxScribo/lxScriboSocket.c\
	hal/src/lxScribo/lxI2c.c \
	hal/src/lxScribo/scribosrv/i2cserver.c \
	hal/src/lxScribo/scribosrv/cmd.c
LOCAL_MODULE := climax_static
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

