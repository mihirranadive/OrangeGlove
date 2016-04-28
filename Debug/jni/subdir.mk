################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../jni/ColorProfiler.cpp \
../jni/HSRecognizer.cpp \
../jni/HandGestures.cpp \
../jni/HandObject.cpp \
../jni/MedianRectangles.cpp 

OBJS += \
./jni/ColorProfiler.o \
./jni/HSRecognizer.o \
./jni/HandGestures.o \
./jni/HandObject.o \
./jni/MedianRectangles.o 

CPP_DEPS += \
./jni/ColorProfiler.d \
./jni/HSRecognizer.d \
./jni/HandGestures.d \
./jni/HandObject.d \
./jni/MedianRectangles.d 


# Each subdirectory must supply rules for building sources it contributes
jni/%.o: ../jni/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/Users/mihir_root/DevKits/android-ndk-r10e/platforms/android-9/arch-arm/usr/include -I/Users/mihir_root/DevKits/OpenCV-android-sdk/sdk/native/jni/include -I/Library/Developer/CommandLineTools/usr/lib/clang/7.0.2/include -I/Users/mihir_root/DevKits/android-ndk-r10e/sources/cxx-stl/gnu-libstdc++/4.9/include -I/Users/mihir_root/DevKits/android-ndk-r10e/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a/include -I/Users/mihir_root/DevKits/android-ndk-r10e/sources/cxx-stl/gnu-libstdc++/4.9/libs/x86_64/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


