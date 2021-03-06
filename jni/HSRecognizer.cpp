//native connector include
#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

//cpp includes
#include <iostream>
#include <vector>
#include <bitset>
#include "HandObject.cpp"
#include "HandGestures.cpp"

#define SAMPLE_CNT 7
#define MATCH_TEMPLATES_CNT 7

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
JNIEXPORT void JNICALL Java_com_example_orangeglove_HandRecognizer_invokeNativeHSRecogCode(
		JNIEnv* env, jobject thisobject, jlong addrGray, jlong addrRgba,
		jlong addrHSVMat, jlong addrFore, jlong addrBack,
		jlong addrSc_LBoundMat, jlong addrSc_UBoundMat, jlong addrPattrnMat);

typedef bitset<8> byte;

//helper functions
void fillScalarFromBoundMats();
void fillScalarFromMat(Mat &myMat, Scalar scalarArray[], int chan);
void SResUp(Mat &srcMat, int cnt);
void SResDown(Mat &srcMat, int cnt);
void getAllTemplateMats(JNIEnv* env, jobject thisobj);
//init functions
void init_LowResMat();
//main opecv/calculating functions
void formForeImageSum();
void detectBoundAndDraw();
//locally used variables
vector<Mat> binImgs;
HandObject myHand;
HandGestures myGestures;
//mats
Mat& backMat = *(Mat*) new Mat();
Mat& foreMat = *(Mat*) new Mat();
Mat& mGray = *(Mat*) new Mat();
Mat& mRgba = *(Mat*) new Mat(); //main mat that the user sees
Mat& mRgba_lowRes = *(Mat*) new Mat();
Mat& mHSVMat = *(Mat*) new Mat(); //this is the mat used for calculating averages
Mat& mScalarLBoundMat = *(Mat*) new Mat();
Mat& mScalarUBoundMat = *(Mat*) new Mat();
Mat& pattrnMat = *(Mat*) new Mat();
Mat templateMats[MATCH_TEMPLATES_CNT];

//bounds and avg value vars
Scalar sc_lowerBounds[SAMPLE_CNT];
Scalar sc_upperBounds[SAMPLE_CNT];

static int NDKCallCnt = 0;

JNIEXPORT void JNICALL Java_com_example_orangeglove_HandRecognizer_invokeNativeHSRecogCode(
		JNIEnv* env, jobject thisobject, jlong addrGray, jlong addrRgba,
		jlong addrHSVMat, jlong addrFore, jlong addrBack,
		jlong addrSc_LBoundMat, jlong addrSc_UBoundMat, jlong addr_pattrnMat) {

	mGray = *(Mat*) addrGray;
	mRgba = *(Mat*) addrRgba;
	foreMat = *(Mat*) addrFore;
	backMat = *(Mat*) addrBack;
	mHSVMat = *(Mat*) addrHSVMat;
	mScalarLBoundMat = *(Mat*) addrSc_LBoundMat;
	mScalarUBoundMat = *(Mat*) addrSc_UBoundMat;
	pattrnMat = *(Mat*) addr_pattrnMat;
	string codepath = "HSRecognizer";

		__android_log_print(ANDROID_LOG_INFO, "HSRecognizer",
				"In NDK. after assigning vars. NDKCallCnt=%d", NDKCallCnt++);

//	long& addrlong = *(long*) addrAddrTemplMats;
//	env->GetLongArrayElements(addrAddrTemplMats, false);

	/*--------------------------------------get all template matrices------------------------------------------*/
	//get all template matrices
//	getAllTemplateMats(env, thisobject);

	jclass thisclass = env->GetObjectClass(thisobject);
	jclass matclass = env->FindClass("org/opencv/core/Mat");

	// Get methods and fields
	jmethodID getPtrMethod = env->GetMethodID(matclass, "getNativeObjAddr",
			"()J");
	jfieldID bufimgsfieldId = env->GetFieldID(thisclass, "matchTemplates",
			"[Lorg/opencv/core/Mat;");
	__android_log_print(ANDROID_LOG_INFO, "HSRecognizer",
					"got field IDs");
	jobjectArray bufimgsArray = (jobjectArray)env->GetObjectField(thisobject, bufimgsfieldId);

	__android_log_print(ANDROID_LOG_INFO, "HSRecognizer",
						"Assigning field IDs");
	for (int i = 0; i < 7; i++)
	        templateMats[i] = *(Mat*)env->CallLongMethod(env->GetObjectArrayElement(bufimgsArray, i), getPtrMethod);

//	Size sz(mRgba.cols,mRgba.rows);
	__android_log_print(ANDROID_LOG_INFO, "HSRecognizer",
						"Resizing mat to for mRgba");
//	resize(templateMats[3], mRgba, sz);
//	templateMats[3].copyTo(mRgba);

//	Imgproc.resize(matchTemplates[2], mRgba, mRgba.size());
	/*---------------------------------------------------------------------------------------------------------*/


	//we'll flip the image if front cam is being used. this is a dummy placeholder condition for now
	if (codepath == "HSRecognizer") {
		__android_log_print(ANDROID_LOG_VERBOSE, "HSRecognizer",
				"Hello from NDK src from inside the Hand Recognizer function");

//		flip(mRgba, mRgba, 1);

		//get a low res mat to work with. this improves speed
		init_LowResMat();
		//save bound matrices into scalar arrays
		fillScalarFromBoundMats();
		//working with the bounds form binary images and form the foreground
		formForeImageSum();
		//start main detection process
		detectBoundAndDraw();

	}

}

}

/*---------------------------------------------Init Functions---------------------------------------------------------*/
void init_LowResMat() {
	//this doesnt work right if used to display since JavaCamera view cannot resize the Camera frame in the activity
	//so if image that is pyrdowned is displayed it has to be pyrUp-ed equally to display with low_resolution
	pyrDown(mRgba, mRgba_lowRes);
	pyrDown(mRgba_lowRes, mRgba_lowRes);
	pyrUp(mRgba_lowRes, mRgba_lowRes);
	pyrUp(mRgba_lowRes, mRgba_lowRes);
}

/*---------------------------------------------Main Opencv/Calculation Functions--------------------------------------*/

//forms no of <SAMPLE_CNT> of binary images and sums them up into one image
//assumes global vars and worksa with low resolution matrix for better performance
void formForeImageSum() {
	//form bin-imgs per sample or per median rectangle sample
	for (int i = 0; i < SAMPLE_CNT; i++) {
		Mat tempForeMat = Mat();
//		binImgs.push_back(Mat(mRgba_lowRes.rows, mRgba_lowRes.cols, CV_8U));
		//keep dest Mat type clean
		blur(mRgba_lowRes, mRgba_lowRes, Size(3, 3));
		inRange(mRgba_lowRes, sc_lowerBounds[i], sc_upperBounds[i],
				tempForeMat);
		binImgs.push_back(tempForeMat);
	}

	//sum up all binary images
	Mat foreImg = Mat();
	binImgs[0].copyTo(foreImg);	//this will correctly set up the size and type of the foreImg mat
	__android_log_print(ANDROID_LOG_VERBOSE, "HSRecognizer",
			"In formForeImageSum. Now adding all Bin-images.");
	for (int i = 1; i < SAMPLE_CNT; i++) {
		foreImg += binImgs[i];
	}
	__android_log_print(ANDROID_LOG_VERBOSE, "HSRecognizer",
			"In formForeImageSum. Now applying medianblur.");
	medianBlur(foreImg, foreImg, 5);

	//dont know why but the resultant binary image is inverted in colors
	//the hand object is black and background is white
	bitwise_not(foreImg, foreImg);

	//copy to jav-addr mat
	foreImg.copyTo(foreMat);

	//clear vectors since they will keep adding unless destroyed
	binImgs.clear();
	if (binImgs.empty())
		__android_log_print(ANDROID_LOG_VERBOSE, "HSRecognizer",
				"BinImgs vector is now empty");
}

//main function calling other hand detection functions
void detectBoundAndDraw() {
	/*------------------Detect Functions----------------------*/
	myHand.initializeElements();
	myHand.findHandContours(foreMat);
	myHand.initializeVectors();
	myHand.findConvexHulls();
	myHand.findBoundingPolygon();
	myHand.findConvexityDefects();
	myHand.getfingerCount(foreMat);

	myHand.writePatternToMat(mRgba);
	if (myHand.isPalmClosed()) {
		//draw pattern into mat
		myHand.writePatternToMat(pattrnMat);
		myHand.clearPatternObjects();//resets drawn pattern
		//analyze
		myGestures.initiateALLFromRecognizedData(pattrnMat,
				myHand.drawnPatternpts, myHand.fingerChangeTracker,
				myHand.lastfingerCnt);
		myGestures.printAllValues();
		myGestures.compareAndgetHistogramValues(templateMats);
		myGestures.clearALL();

	}

	/*------------------Drawing functions---------------------*/

	myHand.drawBoundingRect(mRgba);
//	myHand.drawMatContours(mRgba, myHand.contours, myHand.indxBigContr, Scalar(0,0,255));
	//draw Polygon
	myHand.drawMatContours(mRgba, myHand.hullsP, myHand.indxBigContr,
			Scalar(0, 0, 255));
	//draw CDefects
//	myHand.drawConvexityDefects(mRgba);
	myHand.drawFingerTipPoints(mRgba);

}

/*---------------------------------------------Hepler Functions including file readers--------------------------------------*/

//ASSet manager readers for reading or getting string from the assets folder where we store out pattern matching images
void getImageFilePaths() {

	/*---------------------------------------------------------------------------------------------*/
	/*
	 AAssetManager* mgr = AAssetManager_fromJava(env, asstmgr);
	 assert(NULL != mgr);
	 __android_log_print(ANDROID_LOG_INFO, TAG, "created AssetMGR in NDK");
	 const char* js = "ptImgs/img1.png";
	 AAsset* asset = AAssetManager_open(mgr, (const char *) js,
	 AASSET_MODE_UNKNOWN);
	 if (asset == NULL) {
	 __android_log_print(ANDROID_LOG_ERROR, TAG,
	 "CANNOT Open Asset/file. filename=%s", js);
	 return;
	 }

	 long len = AAsset_getLength(asset);
	 __android_log_print(ANDROID_LOG_INFO, TAG, "Len=%ld", len);
	 char* buf_char = (char*) malloc(sizeof(char) * (len));

	 AAsset_read(asset, buf_char, len);

	 //	byte buffByte[] = (byte) malloc(sizeof(byte) * len);
	 byte buffByte[sizeof(byte) * len];
	 int nRead;

	 FileStorage fstore;

	 while ((nRead = (AAsset_read(asset, buffByte, len))) != -1) {
	 buffer.write(buffByte, 0, nRead);
	 }


	 __android_log_print(ANDROID_LOG_INFO, TAG, "buffer==%s", buf_char);

	 //	__android_log_print(ANDROID_LOG_INFO, TAG, "byte =%d", buffByte);

	 //	byte* BYTEArr = AAsset_getBuffer(asset);
	 __android_log_print(ANDROID_LOG_ERROR, TAG, "BuufByteSize=%d, ARRSize=%ld ",
	 buffByte->size(), sizeof(byte) * len);

	 Mat outMat = Mat(500, 302, CV_8UC3, buffByte);
	 Mat decodedMat = Mat(500, 302, CV_8UC3);
	 decodedMat = imdecode(outMat, IMREAD_COLOR);

	 //	imdecode(BYTEArr, IMREAD_COLOR);

	 //	//keep readign the file
	 //	while((AAsset_read(asset, buf, len)) !=0 && ((AAsset_read(asset, buf, len)) !=NULL)){
	 //
	 //	}
	 //	outMat.copyTo(mRgba);
	 if (NDKCallCnt <= 1) {
	 __android_log_print(ANDROID_LOG_INFO, TAG, "Rows=%d, Cols=%d",
	 decodedMat.rows, decodedMat.cols);
	 int chan = decodedMat.channels();
	 for (int i = 0; i < decodedMat.rows; i++) {
	 for (int j = 0; j < outMat.cols; j++) {
	 int d1 = decodedMat.data[chan * decodedMat.cols * i + j * chan
	 + 0];
	 int d2 = decodedMat.data[chan * decodedMat.cols * i + j * chan
	 + 1];
	 int d3 = decodedMat.data[chan * decodedMat.cols * i + j * chan
	 + 2];
	 if (d1 != 0 || d2 != 0 || d3 != 0)
	 __android_log_print(ANDROID_LOG_INFO, TAG, "%d,%d,%d", d1,
	 d2, d3);
	 }
	 }
	 }
	 AAsset_close(asset);
	 __android_log_print(ANDROID_LOG_INFO, TAG, "Done loading assset stuff..");
	 */
	/*---------------------------------------------------------------------------------------------*/

}

//fills in the respected lower and upper bounds Scalars with matrices that were used to store the color profile
// mats filled in are of the type 7,1, CV_8UC3 and a scalar component which is the average of the
// scalar values calculated per median rect in the colorprofiler.cpp file
void fillScalarFromBoundMats() {
	if (mScalarLBoundMat.rows == mScalarUBoundMat.rows
			&& mScalarLBoundMat.cols == mScalarUBoundMat.cols) {
		fillScalarFromMat(mScalarLBoundMat, sc_lowerBounds,
				mScalarLBoundMat.channels());
		fillScalarFromMat(mScalarUBoundMat, sc_upperBounds,
				mScalarUBoundMat.channels());
	} else {
		__android_log_print(ANDROID_LOG_ERROR, "HSRecognizer",
				"Low/Upper bound Matrice Sizes are different!");
	}
	__android_log_print(ANDROID_LOG_VERBOSE, "HSRecognizer",
			"filled out both bound Scalars manually");

	//print Scalar arrays
	__android_log_print(ANDROID_LOG_VERBOSE, "HSRecognizer", "Scal_LB==>");
	for (int i = 0; i < SAMPLE_CNT; i++) {
		__android_log_print(ANDROID_LOG_VERBOSE, "HSRecognizer", "%f, %f, %f",
				sc_lowerBounds[i].val[0], sc_lowerBounds[i].val[1],
				sc_lowerBounds[i].val[2]);
	}
	__android_log_print(ANDROID_LOG_VERBOSE, "HSRecognizer", "Scal_UB==>");
	for (int i = 0; i < SAMPLE_CNT; i++) {
		__android_log_print(ANDROID_LOG_VERBOSE, "HSRecognizer", "%f, %f, %f",
				sc_upperBounds[i].val[0], sc_upperBounds[i].val[1],
				sc_upperBounds[i].val[2]);
	}

}

//manually populates the scalar arrays from the matrices filled in from the file read in java code
//files and the respective mats contain color profile created for the user.
void fillScalarFromMat(Mat &myMat, Scalar scalarArray[], int chan) {
	for (int i = 0; i < myMat.rows; i++) {
		for (int j = 0; j < myMat.cols; j++) {
			//RGB channels
			scalarArray[i].val[0] = myMat.data[chan * myMat.cols * i + j * chan
					+ 0]; //R
			scalarArray[i].val[1] = myMat.data[chan * myMat.cols * i + j * chan
					+ 1]; //G
			scalarArray[i].val[2] = myMat.data[chan * myMat.cols * i + j * chan
					+ 2]; //B
		}
	}
}

void SResDown(Mat &srcMat, int cnt) {
	Mat tempMat1 = Mat();
	Mat tempMat2 = Mat();
	srcMat.copyTo(tempMat1);
	for (int i = 0; i < cnt; i++) {
		pyrDown(tempMat1, tempMat2);
		tempMat2.copyTo(tempMat1);
	}
	tempMat1.copyTo(srcMat);
}

void SResUp(Mat &srcMat, int cnt) {
	Mat tempMat1 = Mat();
	Mat tempMat2 = Mat();
	srcMat.copyTo(tempMat1);
	for (int i = 0; i < cnt; i++) {
		pyrUp(tempMat1, tempMat2);
		tempMat2.copyTo(tempMat1);
	}
	tempMat1.copyTo(srcMat);
}

void getAllTemplateMats(JNIEnv* env, jobject& thisobject) {

}
