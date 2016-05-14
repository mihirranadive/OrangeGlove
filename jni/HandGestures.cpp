//native connector include
#include <jni.h>
#include <android/log.h>
#include<android/asset_manager.h>
#include<android/asset_manager_jni.h>

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
#define MATCH_TEMPLATES_COUNT 7

using namespace std;
using namespace cv;

class HandGestures {

public:
	Mat drawnPattern; //keeps track of the pattern thats drawn.. has a blank background
	vector<Point> drawnPtSet; //Set of points to draw the circles for
	vector<int> fingerChanges;
	vector<double> histVals;
	int lastfingerCount;
//	Mat templateMats[MATCH_TEMPLATES_COUNT];
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
		checkFingerCntValidity(); //check if last val is valid
	}

	void clearALL() {
		drawnPattern.release();
		drawnPtSet.clear();
		fingerChanges.clear();
		histVals.clear();
	}
	/*--------------------------------------Draw and update functions-----------------------------------------------*/
	void printAllValues() {
		__android_log_print(ANDROID_LOG_INFO, TAG, "Printing All Values");
		__android_log_print(ANDROID_LOG_INFO, TAG, "lastFingerCount=%d",
				lastfingerCount);
		__android_log_print(ANDROID_LOG_INFO, TAG,
				"drawnPattern.size::Rows=%d, Cols=%d", drawnPattern.rows,
				drawnPattern.cols);
		size_t sz = drawnPtSet.size();
		__android_log_print(ANDROID_LOG_INFO, TAG, "drawnptSet size=%d", sz);
		for (int i = 0; i < fingerChanges.size(); i++) {
			__android_log_print(ANDROID_LOG_INFO, TAG, "%d", fingerChanges[i]);
		}
	}

	/*--------------------------------------Pattern matching and/or recognition functions---------------------------*/
	//these inclcude reading from ready images to match against
	//compares drawnpattern with all template images and stores values for each one
	void compareAndgetHistogramValues(Mat templateMats[]) {
		Mat outHist;
		MatND hist1;
		float h_ranges[] = { 0, 180 };
		float s_ranges[] = { 0, 256 };
		int channels[] = { 0, 1 };
		const float* ranges[] = { h_ranges, s_ranges };

		int Rval = 30, sbins = 32;
		int histSize[] = { Rval, sbins };
		//loop through each template image and get hist values
//		for (int i = 0; i < MATCH_TEMPLATES_COUNT; i++) {
		calcHist(templateMats, 7, channels, Mat(), hist1, 2, histSize, ranges,
				true, false);
		double maxVal = 0;
		minMaxLoc(hist1, 0, &maxVal, 0, 0);
//		}
	}

	/*--------------------------------------helper functions--------------------------------------------------------*/

	string integerToString(int num) {
		stringstream ss;
		ss << num;
		return ss.str();
	}

//checks validity of data
	bool checkFingerCntValidity() {
		int lstFng = fingerChanges[fingerChanges.size() - 1];
		if (lstFng != lastfingerCount) {
			__android_log_print(ANDROID_LOG_ERROR, TAG,
					"Last finger counts dont match. lstFnginVec=%d and lastFingerCnt=%d",
					lstFng, lastfingerCount);
			return true;
		}
		return false;
	}

};
