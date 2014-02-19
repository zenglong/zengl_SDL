LOCAL_PATH := $(call my-dir)

ZENGL_PATH := ../../../linux

include $(CLEAR_VARS)
LOCAL_MODULE    := zengl
LOCAL_SRC_FILES := $(ZENGL_PATH)/zengl_main.c  $(ZENGL_PATH)/zengl_parser.c  $(ZENGL_PATH)/zengl_symbol.c  $(ZENGL_PATH)/zengl_locals.c  $(ZENGL_PATH)/zengl_assemble.c  $(ZENGL_PATH)/zengl_ld.c  $(ZENGL_PATH)/zenglrun_main.c  $(ZENGL_PATH)/zenglrun_func.c  $(ZENGL_PATH)/zenglApi.c  $(ZENGL_PATH)/zenglApi_BltModFuns.c $(ZENGL_PATH)/zenglDebug.c

LOCAL_CFLAGS    := -DZL_LANG_EN_WITH_CH -DZL_EXP_OS_IN_ARM_GCC
#LOCAL_CFLAGS += -DHAVE_SYS_UIO_H
#LOCAL_CFLAGS    += -g
#LOCAL_CFLAGS    += -ggdb
#LOCAL_CFLAGS    += -O1

#LOCAL_LDLIBS := -llog
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../SDL_image

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.cpp \
	testgles.c jni_glue.cpp mySDL_Lib.c

#LOCAL_CFLAGS    += -g
#LOCAL_CFLAGS    += -ggdb
#LOCAL_CFLAGS    += -O1

LOCAL_STATIC_LIBRARIES := zengl
LOCAL_SHARED_LIBRARIES := SDL SDL_image SDL_ttf

LOCAL_LDLIBS := -lGLESv1_CM -llog

include $(BUILD_SHARED_LIBRARY)
