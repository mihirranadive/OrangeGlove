LOCAL_PATH := $(call my-dir)
 
include $(CLEAR_VARS)
OPENCV_INSTALL_MODULES:=on
include /Users/mihir_root/DevKits/OpenCV-android-sdk/sdk/native/jni/OpenCV.mk
 
# Here we give our module name and source file(s)
LOCAL_MODULE    := orangeglove
LOCAL_SRC_FILES := HSRecognizer.cpp ColorProfiler.cpp
LOCAL_LDLIBS +=  -llog -ldl
 
include $(BUILD_SHARED_LIBRARY)


# Add prebuilt library
#include $(CLEAR_VARS)

#LOCAL_MODULE := opencv_java3
#LOCAL_SRC_FILES := libopencv_java3.so

#include $(PREBUILT_SHARED_LIBRARY)