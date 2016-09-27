MY_PATH := $(call my-dir)

# add RPC dirs if RPC is available
ifneq ($(TARGET_NO_RPC),true)
include $(MY_PATH)/libloc_api-rpc-50001/Android.mk
endif #TARGET_NO_RPC

include $(MY_PATH)/libloc_api_50001/Android.mk
