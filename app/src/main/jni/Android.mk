LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := slidesolver
LOCAL_SRC_FILES := slidesolver.c

include $(BUILD_SHARED_LIBRARY)
