LOCAL_PATH := $(call my-dir)

ROOT_DIR   := $(LOCAL_PATH)/..
CORE_DIR   := $(ROOT_DIR)/nxengine
EXTRACTDIR := $(CORE_DIR)/extract-auto

include $(ROOT_DIR)/Makefile.common

COREFLAGS := -D__LIBRETRO__ -DFRONTEND_SUPPORTS_RGB565 $(INCFLAGS)

ifeq ($(NDK_DEBUG), 1)
  COREFLAGS += -DDEBUG_LOG=1
else
  COREFLAGS += -DRELEASE_BUILD
endif

include $(CLEAR_VARS)
LOCAL_MODULE    := retro
LOCAL_SRC_FILES := $(SOURCES_C) $(SOURCES_CXX)
LOCAL_CXXFLAGS  := $(COREFLAGS)
LOCAL_CFLAGS    := $(COREFLAGS)
LOCAL_LDFLAGS   := -Wl,-version-script=$(CORE_DIR)/libretro/link.T
include $(BUILD_SHARED_LIBRARY)
