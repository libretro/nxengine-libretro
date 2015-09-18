LOCAL_PATH := $(call my-dir)
RELEASE_BUILD=1

include $(CLEAR_VARS)

NX_DIR     = ../nxengine-1.0.0.4
EXTRACTDIR = $(NX_DIR)/extract-auto

LOCAL_MODULE    := retro

ifeq ($(RELEASE_BUILD), 1)
LOCAL_CXXFLAGS += -DRELEASE_BUILD
endif

ifeq ($(TARGET_ARCH),arm)
LOCAL_CXXFLAGS += -DANDROID_ARM
LOCAL_ARM_MODE := arm
endif

ifeq ($(TARGET_ARCH),x86)
LOCAL_CXXFLAGS +=  -DANDROID_X86
endif

ifeq ($(TARGET_ARCH),mips)
LOCAL_CXXFLAGS += -DANDROID_MIPS
endif

ifeq ($(DEBUGLOG), 1)
CFLAGS += -DDEBUG_LOG=1
endif

CORE_DIR := ../nxengine
EXTRACTDIR   := $(CORE_DIR)/extract-auto

include ../Makefile.common

OBJECTS    := 	$(SOURCES_C) $(SOURCES_CXX)

LOCAL_SRC_FILES := $(OBJECTS)

LOCAL_CXXFLAGS += -DINLINE=inline -DHAVE_STDINT_H -DHAVE_INTTYPES_H -D__LIBRETRO__ -DFRONTEND_SUPPORTS_RGB565
LOCAL_C_INCLUDES  = $(CORE_DIR) $(CORE_DIR)/graphics $(CORE_DIR)/libretro $(CORE_DIR)/sdl/include $(CORE_DIR)/libretro/libretro-common/include

include $(BUILD_SHARED_LIBRARY)
