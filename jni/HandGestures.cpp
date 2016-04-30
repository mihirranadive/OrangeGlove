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

class HandGestures {

public:
	Mat drawnPattern; //keeps track of the pattern thats drawn.. has a blank background
	vector<Point> drawnPtSet; //Set of points to draw the circles for
	vector<int> fingerChanges;
	int lastfingerCount;
	string imgFilePath; //main path to image file which we will match/compare our drawn pattern against

	/*--------------------------------------init functions----------------------------------------------------------*/

	void initElements() {
		__android_log_print(ANDROID_LOG_VERBOSE, TAG,
				"initializing HandGestures Elements");
		drawnPattern = Mat();
	}

	void initiateALLFromRecognizedData(Mat &src, vector<Point> ptset,
			vector<int> cntSet, int lstfngrcnt) {
		src.copyTo(drawnPattern);
		drawnPtSet = ptset;
		fingerChanges = cntSet;
		lastfingerCount = lstfngrcnt;
	}

	/*--------------------------------------Pattern Draw and update functions---------------------------------------*/

	/*--------------------------------------Pattern matching and/or recognition functions---------------------------*/
	//these inclcude reading from ready images to match against

	/*--------------------------------------helper functions--------------------------------------------------------*/

	string integerToString(int num) {
		stringstream ss;
		ss << num;
		return ss.str();
	}

	//checks validity of data
	void checkFingerCntValidity() {
		int lstFng = fingerChanges[fingerChanges.size() - 1];
		if (lstFng != lastfingerCount) {
			__android_log_print(ANDROID_LOG_ERROR, TAG,
					"Last finger counts dont match. lstFnginVec=%d and lastFingerCnt=%d",
					lstFng, lastfingerCount);
		}
	}

};
