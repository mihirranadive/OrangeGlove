//native connector include
#include <jni.h>
#include <android/log.h>

//cpp includes
#include <iostream>
#include <vector>
#include "MedianRectangles.cpp"
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
JNIEXPORT void JNICALL Java_com_example_orangeglove_ColorProfiler_invokeNativeColorProfiler(
		JNIEnv*, jobject, jlong addrGray, jlong addrRgba, jlong addrHSVMat,
		jlong addrFore, jlong addrBack, jlong addrSc_LBoundMat,
		jlong addrSc_UBoundMat);
//JNIEXPORT void JNICALL Java_com_example_orangeglove_MainActivity_invokeNativeCode();

//helper functions
float euclideanDist(Point p1, Point p2);
int getMedianVal(vector<int> values);
void colorMat_to_HSV_Format();
void colorMat_to_RGBA_Format();
void fillMat(Mat &myMat, Scalar scalarArray[], int chan);
//init functions
void initMedianRects();
void init_upAndlowBounds();
//main calculating functions
void getAvgColorFromMedianRect(MedianRectangle medRect, int mean[3]);
void ColorProfiler_HSV();
void calcLowAndUppBounds();
void fillBoundMats();
//variables
vector<MedianRectangle> profilerRects;

//mats
Mat& back = *(Mat*) new Mat();
Mat& fore = *(Mat*) new Mat();
Mat& mGr = *(Mat*) new Mat();
Mat& mRgb = *(Mat*) new Mat(); //main mat that the user sees
Mat& mHSV = *(Mat*) new Mat(); //this is the mat used for calculating averages
Mat& mSc_LBoundMat = *(Mat*) new Mat();
Mat& mSc_UBoundMat = *(Mat*) new Mat();

//bounds and average value vars
int lower_lim[SAMPLE_CNT][3];
int upper_lim[SAMPLE_CNT][3];

Scalar lowerBounds[SAMPLE_CNT];
Scalar upperBounds[SAMPLE_CNT];
int avgColorVals[SAMPLE_CNT][3];

JNIEXPORT void JNICALL Java_com_example_orangeglove_ColorProfiler_invokeNativeColorProfiler(
		JNIEnv*, jobject, jlong addrGray, jlong addrRgba, jlong addrHSVMat,
		jlong addrFore, jlong addrBack, jlong addrSc_LBoundMat,
		jlong addrSc_UBoundMat) {
//JNIEXPORT void JNICALL Java_com_example_orangeglove_MainActivity_invokeNativeCode(){

	__android_log_print(ANDROID_LOG_VERBOSE, "HSRecognizer",
			"Hello from NDK src");

	//convert input addresses to workable objects
	mGr = *(Mat*) addrGray;
	mRgb = *(Mat*) addrRgba;
	fore = *(Mat*) addrFore;
	back = *(Mat*) addrBack;
	mHSV = *(Mat*) addrHSVMat;
	mSc_LBoundMat = *(Mat*) addrSc_LBoundMat;
	mSc_UBoundMat = *(Mat*) addrSc_UBoundMat;

//	lowerBounds[0] = *(Scalar*) sc_lowerbound[0];
//	upperBounds[0] = *(Scalar*) sc_upperbound[0];

	string prog_path = "ColorProfiler";
	//placeholder condition will be modified for type of cam used
	if (prog_path.compare("ColorProfiler") == 0) {
		//path towards color profiler
		__android_log_print(ANDROID_LOG_VERBOSE, "ColorProfiler",
				"Path chosen => Color Profiler");
		//if using the back camera flip the image
		//if()
		flip(mRgb, mRgb, 1);

		//initMedianRects();
		//ColorProfiler_HSV();
		fillBoundMats();


	}

}

}

/*-----------------------------------------INIT FUNCTIONS----------------------------------------- */

//initialize and draw median rectangles on screen from which we will take color samples from
void initMedianRects() {
	int sqLen = 20; //defines the length of each side of the square
	//place median rectangles strategically on screen
	//we will be currently taking samples from 7 rectangles

	//middle finger upper point of palm
	profilerRects.push_back(
			MedianRectangle(Point(mRgb.cols / 2 + 20, mRgb.rows / 4 - 20),
					Point(mRgb.cols / 2 + 20 + sqLen,
							mRgb.rows / 4 - 20 + sqLen)));
	//little finger lower and far side of palm
	profilerRects.push_back(
			MedianRectangle(Point(mRgb.cols / 2 + 70, mRgb.rows / 3),
					Point(mRgb.cols / 2 + 70 + sqLen, mRgb.rows / 3 + sqLen)));
	//center but below fingers point of palm. this can be quite dark
	profilerRects.push_back(
			MedianRectangle(Point(mRgb.cols / 2 + 25, mRgb.rows / 2 + 10),
					Point(mRgb.cols / 2 + 25 + sqLen,
							mRgb.rows / 2 + 10 + sqLen)));
	//just below fingers and towards the little finger side
	profilerRects.push_back(
			MedianRectangle(Point(mRgb.cols / 2 + 100, mRgb.rows / 2 + 10),
					Point(mRgb.cols / 2 + 100 + sqLen,
							mRgb.rows / 2 + 10 + sqLen)));
	//lower part towards the thumb
	profilerRects.push_back(
			MedianRectangle(Point(mRgb.cols / 1.75 - 20, mRgb.rows / 1.5),
					Point(mRgb.cols / 1.75 - 20 + sqLen,
							mRgb.rows / 1.5 + sqLen)));
	//lower part towards the little finger
	profilerRects.push_back(
			MedianRectangle(Point(mRgb.cols / 1.75 + 40, mRgb.rows / 1.5),
					Point(mRgb.cols / 1.75 + 40 + sqLen,
							mRgb.rows / 1.5 + sqLen)));
	//under-side of the thumb. an outer lower part
	profilerRects.push_back(
			MedianRectangle(Point(mRgb.cols / 2 - 35, mRgb.rows / 2 + 45),
					Point(mRgb.cols / 2 - 35 + sqLen,
							mRgb.rows / 2 + 45 + sqLen)));

	//draw all 7 rectangles
	for (int i = 0; i < SAMPLE_CNT; i++) {
		MedianRectangle curr = profilerRects[i];
		if (curr.up_corner.x > 0 && curr.low_corner.x > 0
				&& curr.up_corner.y > 0 && curr.low_corner.y > 0)
				//these conditions ensure that while drawing the Rect the width and height are in correct bounds.
				//if not it can cause the App to crash
//				&& curr.width + curr.up_corner.x < mRgb.cols && curr.width + curr.low_corner.x < mRgb.cols
//				&& curr.height + curr.up_corner.y < mRgb.rows && curr.height + curr.low_corner.y < mRgb.rows)
						{
			profilerRects[i].drawRectangle(mRgb);
		}
	}
}

//sets lower and upper bounds for our HSV ranges to define a proper bound for each mean calculated
void initupAndlowBounds() {
	for (int i = 0; i < SAMPLE_CNT; i++) {
		lower_lim[i][0] = 6;
		lower_lim[i][1] = 27;
		lower_lim[i][2] = 75;

		upper_lim[i][0] = 12;
		upper_lim[i][1] = 35;
		upper_lim[i][2] = 75;
	}
}

/*-----------------------------------------MAIN CALCULATING/OPENCV FUNCTIONS----------------------------------------- */

//main fucntion calling all others to create color profile
//function assumes the mHSV mat exists and will calculate mean H,S and V values.
void ColorProfiler_HSV() {
	//convert image to HSV format
	colorMat_to_HSV_Format();

	//get average color values for each median rectangle
	//loop through the median rectangles get each of their median values
	for (int i = 0; i < SAMPLE_CNT; i++) {
		getAvgColorFromMedianRect(profilerRects[i], avgColorVals[i]);
	}

	//calculate bounds for the inrange function
	calcLowAndUppBounds();
	__android_log_print(ANDROID_LOG_VERBOSE, "ColorProfiler",
			"lower and upper bounds have been calculated");
}

//fills in the respected matrices with calculated lower and upper bounds
// mats filled in are of the type 7,1, CV_8UC3 and a scalar component which is the average of the
// scalar values calculated per median rect
void fillBoundMats() {
	Scalar scArr[7];
	for (int i = 0; i < 7; i++)
		scArr[i] = Scalar(0 + i, 100 + i, 200 + i);

	if(mSc_LBoundMat.rows == mSc_UBoundMat.rows && mSc_LBoundMat.cols == mSc_UBoundMat.cols){
//		fillMat(mSc_LBoundMat, lowerBounds, mSc_LBoundMat.channels());
//		fillMat(mSc_UBoundMat, upperBounds, mSc_LBoundMat.channels());
		fillMat(mSc_LBoundMat, scArr, mSc_LBoundMat.channels());
		fillMat(mSc_UBoundMat, scArr, mSc_LBoundMat.channels());
	}
	else{
		__android_log_print(ANDROID_LOG_ERROR, "ColorProfiler", "Low/Upper bound Matrice Sizes are different!");
	}
	__android_log_print(ANDROID_LOG_VERBOSE, "ColorProfiler", "filled out both bound matrices manually");
}

//get average HSV values from the median rect. does this for one median rect only
void getAvgColorFromMedianRect(MedianRectangle medRect, int mean[3]) {

	//get position of the rect inside the source Mat and store in a mat.
	Mat medRectMat = mHSV(
			Rect(medRect.up_corner.x, medRect.up_corner.y, medRect.width,
					medRect.height));

	vector<int> H_min, S_min, V_min; //HSV values to be stored to find the median value from
	int chans = medRectMat.channels();
	//going over the Mat to get values
	for (int i = 3; i < medRectMat.rows - 3; i++) {
		for (int j = 3; j < medRectMat.cols - 3; j++) {
			//push each row
			// refer this link for why the stepping through below
			//http://stackoverflow.com/questions/7899108/opencv-get-pixel-channel-value-from-mat-image
			H_min.push_back(
					medRectMat.data[chans * medRectMat.cols * i + j * chans + 0]); //H extracted
			S_min.push_back(
					medRectMat.data[chans * medRectMat.cols * i + j * chans + 1]); //S extracted
			V_min.push_back(
					medRectMat.data[chans * medRectMat.cols * i + j * chans + 2]); //V extracted
		}
	}
	//get medians
	mean[0] = getMedianVal(H_min);
	mean[1] = getMedianVal(S_min);
	mean[2] = getMedianVal(V_min);

}

//calculates lower and upper bounds for the inRange function based on median and average values found before
//average color/values from the HSV mat must've been found before using this function
void calcLowAndUppBounds() {
	__android_log_print(ANDROID_LOG_VERBOSE, "ColorProfiler",
			"setting initial low and up bound per median rectangle");
	initupAndlowBounds();
	__android_log_print(ANDROID_LOG_VERBOSE, "ColorProfiler",
			"setting final low and up bound per median rectangle");
	for (int i = 0; i < SAMPLE_CNT; i++) {
		lowerBounds[i] = Scalar(avgColorVals[i][0] - lower_lim[i][0],
				avgColorVals[i][1] - lower_lim[i][1],
				avgColorVals[i][2] - lower_lim[i][2]);
		upperBounds[i] = Scalar(avgColorVals[i][0] + upper_lim[i][0],
				avgColorVals[i][1] + upper_lim[i][1],
				avgColorVals[i][2] + upper_lim[i][2]);
	}
}

/*-----------------------------------------HELPER FUNCTIONS----------------------------------------- */

//return euclidean distance between two points
float euclideanDist(Point p1, Point p2) {
	__android_log_print(ANDROID_LOG_VERBOSE, "ColorProfiler",
			"Calculating euclidean distance");
	return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

//returns the median value of vector of integers
int getMedianVal(vector<int> values) {
	__android_log_print(ANDROID_LOG_VERBOSE, "getMedianVal",
			"In function getMedianVal");
	int medianVal = -1;
	size_t size = values.size();
	sort(values.begin(), values.end());
	if (size % 2 == 0) {
		medianVal = values[size / 2 - 1];
	} else {
		medianVal = values[size / 2];
	}
	return medianVal;
}

//converts our main mRgb mat to HSV color format into a separate mHSV mat.
//due to the aplha component this needs a step more conversion
void colorMat_to_HSV_Format() {
	__android_log_print(ANDROID_LOG_VERBOSE, "ColorProfiler",
			"Converting mRgba to HSV format");
	Mat tempMat = Mat();
	mRgb.copyTo(tempMat);
	//somehow cvtcolor won't work if input and output Mats are the same.
	cvtColor(mRgb, tempMat, CV_RGBA2RGB);
	cvtColor(tempMat, mHSV, CV_RGB2HSV);
}

//converts our converted HSV mat to RGBA color format
//this is not required only added for good measure, just in case.
void colorMat_to_RGBA_Format() {
	Mat tempMat = Mat();
	mRgb.copyTo(tempMat);
	//somehow cvtcolor won't work if input and output Mats are the same.
	cvtColor(mHSV, tempMat, CV_HSV2RGB);
	cvtColor(tempMat, mRgb, CV_RGB2RGBA);
}

//manually populates a matrix with data values from the passed scalar array
//updates only a 3 channel matrix
void fillMat(Mat &myMat, Scalar scalarArray[], int chan){
	for(int i=0;i<myMat.rows;i++){
		for(int j=0;j<myMat.cols;j++){
			//medRectMat.data[chans * medRectMat.cols * i + j * chans + 0]); //H extracted
			//RGB channels
			myMat.data[chan * myMat.cols * i + j * chan + 0] = scalarArray[i].val[0]; //R
			myMat.data[chan * myMat.cols * i + j * chan + 1] = scalarArray[i].val[1]; //G
			myMat.data[chan * myMat.cols * i + j * chan + 2] = scalarArray[i].val[2]; //B
		}
	}
}
