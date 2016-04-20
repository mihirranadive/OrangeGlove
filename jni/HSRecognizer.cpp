//native connector include
#include <jni.h>
#include <android/log.h>

//cpp includes
#include <iostream>
#include <vector>
#include "HandObject.cpp"

#define SAMPLE_CNT 7

//opencv inculdes
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
//namespaces
using namespace std;
using namespace cv;

extern "C" {

//------------------ALL function declarations need to be here to be in the native code scope---------------------------
JNIEXPORT void JNICALL Java_com_example_orangeglove_HandRecognizer_invokeNativeHSRecognizer(
		JNIEnv*, jobject, jlong addrGray, jlong addrRgba, jlong addrHSVMat,  jlong addrFore,
		jlong addrBack);

//helper functions


//init functions

//main calculating functions



//variables
Mat& backMat = *(Mat*) new Mat();
Mat& foreMat = *(Mat*) new Mat();
Mat& grMat = *(Mat*) new Mat();
Mat& rgbMat = *(Mat*) new Mat();
Mat& HSVMat = *(Mat*) new Mat();


JNIEXPORT void JNICALL Java_com_example_orangeglove_HandRecognizer_invokeNativeHSRecognizer(
		JNIEnv*, jobject, jlong addrGray, jlong addrRgba, jlong addrHSVMat,  jlong addrFore,
		jlong addrBack){

}

}
