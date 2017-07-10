LOCAL_PATH := $(call my-dir)
PROJECT_ROOT:= $(call my-dir)/../../../../..

include $(CLEAR_VARS)
LOCAL_MODULE    := librgba_data
LOCAL_SHARED_LIBRARIES := tango_client_api tango_support_api
LOCAL_CFLAGS    := -Werror -std=c++11
LOCAL_SRC_FILES := native-lib.cpp \
                   RGBA_Data.cpp \
                   lodepng.cpp
LOCAL_LDLIBS    := -llog -L$(SYSROOT)/usr/lib
include $(BUILD_SHARED_LIBRARY)

$(call import-add-path, $(PROJECT_ROOT))
$(call import-module,tango_client_api)
$(call import-module,tango_support_api)