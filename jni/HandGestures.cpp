//native connector include
#include <jni.h>
#include <android/log.h>

#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <cmath>

//opencv inculdes
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>

#define TAG "HandGestures"

using namespace std;
using namespace cv;



class HandGestures{

	Mat drawnPattern; //keeps track of the pattern thats drawn.. has a blank background
	vector<Point> drawPtSet; //Set of points to draw the circles for



	/*--------------------------------------init functions----------------------------------------------------------*/

	void initElements(){
		__android_log_print(ANDROID_LOG_VERBOSE, TAG, "initializing HandGestures Elements");
		drawnPattern = Mat();
	}


	/*--------------------------------------Pattern Draw and update functions---------------------------------------*/


	/*--------------------------------------helper functions-------------------------------------------------------*/


};
