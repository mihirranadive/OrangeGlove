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

#define TAG "HandObject"

using namespace std;
using namespace cv;

//class contains all Hand related Objects like contours around hand bounding rectangle/polygon or area
class HandObject {

public:
	//contour and bounding related variables
	bool isHandClosed; //true when hand is present/open/detected
	Scalar avgColor; // this is determined from the color profiler
	vector<vector<Point> > contours;
	vector<vector<int> > hullsI;
	vector<vector<Point> > hullsP;
	vector<vector<Vec4i> > Cdefects;
	vector<Point> fingerTips;

	Rect handBoundingRect;
	int indxBigContr;
	int cDef_cnt;
	//tracking and flow related variables
	int fingerCount, prevFingerCnt;

	//pattern and tracking vars
	vector<Point> drawnPatternpts;
	vector<int> fingerChangeTracker;
	int lastfingerCnt;


	/*--------------------------Constructors and initializations---------------------------*/

	void initializeElements() {
		cDef_cnt = 0;
		indxBigContr = fingerCount = prevFingerCnt = -1;
	}

	void initializeVectors() {
		if (contours.size() > 0) {
			hullsI = vector<vector<int> >(contours.size());
			hullsP = vector<vector<Point> >(contours.size());
			Cdefects = vector<vector<Vec4i> >(contours.size());
			__android_log_print(ANDROID_LOG_INFO, TAG,
					"Vectors have been initialized");
		} else {
			__android_log_print(ANDROID_LOG_ERROR, TAG,
					"Can't initialize vectors since contours are null/0 in size");
		}
	}

	/*--------------------------hand detect related functions----------------------------*/

	//given a binary image matrix, finds contours for that matrix
	//also calls and sets index of biggest contour
	void findHandContours(Mat const &binaryMat) {
		Mat ctrMat;
		binaryMat.copyTo(ctrMat);
		__android_log_print(ANDROID_LOG_WARN, "HandObject", "Finding contours");
		findContours(ctrMat, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
//		__android_log_print(ANDROID_LOG_ERROR, "HandObject", "Done finding contours. contours size=>%d", contours.size());
		setIndexofBiggestContour();
		__android_log_print(ANDROID_LOG_INFO, TAG,
				"Exiting FindHandContours. Biggest contour Index=%d",
				indxBigContr);
	}

	//finds approxPolydp around the HullsP
	//assumes hullsP is filled when finding convex hulls along with indxBigContr
	void findBoundingPolygon() {
		if (indxBigContr >= 0 && !hullsP.empty()) {
			int epsilon = 18; // degree of acceptability for the Ramer-Douglas-Peucker algo
			RotatedRect rotRect = minAreaRect(Mat(contours[indxBigContr]));
			handBoundingRect = rotRect.boundingRect();
			approxPolyDP(Mat(contours[indxBigContr]), hullsP[indxBigContr],
					epsilon, true);
			return;
		}
		__android_log_print(ANDROID_LOG_ERROR, TAG,
				"in findPoly. indxBigContr is -ve or hullsP is empty");
	}

	//finds convex hulls of I and P based on the filled out contours
	void findConvexHulls() {
		if (indxBigContr >= 0) {
			convexHull(Mat(contours[indxBigContr]), hullsI[indxBigContr], false,
					false);
			//return points is true since we need the convex hull pts for Cdefects
			convexHull(Mat(contours[indxBigContr]), hullsP[indxBigContr], false,
					true);
		} else {
			__android_log_print(ANDROID_LOG_ERROR, TAG,
					"In findConvexHulls. Can't build on contours since biggest contour index <0");
		}
	}

	//find convexity Defexts in the largest contour
	void findConvexityDefects() {
		if (indxBigContr >= 0 && contours[indxBigContr].size() > 5) {
			__android_log_print(ANDROID_LOG_VERBOSE, TAG,
					"Starting findConvexityDefects");
			convexityDefects(contours[indxBigContr], hullsI[indxBigContr],
					Cdefects[indxBigContr]);
			removeBadCDefects();
			removeJointFingerPts();
			__android_log_print(ANDROID_LOG_VERBOSE, TAG,
					"Exiting findConvexityDefects");
		} else
			__android_log_print(ANDROID_LOG_ERROR, TAG,
					"In findCDefects, indx is -ve or contour size is bad");
	}

	//gives finger count based on Cdefects present
	//adds or removes finger tips as required
	int getfingerCount(Mat &srcMat) {
		fingerTips.clear();
		prevFingerCnt = fingerCount;
		vector<Point> resPoints;
		vector<Vec4i>::iterator defIterator = Cdefects[indxBigContr].begin();
		int i = 0;	//point/defect number indicator
		while (defIterator != Cdefects[indxBigContr].end()) {
			Vec4i& currCd = (*defIterator);
			int start = currCd[0];
			int end = currCd[1];
			int faridx = currCd[2];
			Point startPt(contours[indxBigContr][start]);
			Point endPt(contours[indxBigContr][end]);
			Point farPt(contours[indxBigContr][faridx]);
			if (i == 0) {
				fingerTips.push_back(startPt);
//				i++;
			}
			fingerTips.push_back(endPt);
			i++;
			defIterator++;
		}
		if (fingerTips.size() == 0) {
			//check for one finger only since Convexity defects are not detected for one finger
			singleFingerCheck(srcMat); //this assigns finger count appropriately if one finger
		}

		//remove fingers tips too close to each other
		for (int i = 0; i < fingerTips.size(); i++) {
			for (int j = i; j < fingerTips.size(); j++) {
				if (p2pDist(fingerTips[i], fingerTips[j]) < 10 && i != j) {
				} else {
					resPoints.push_back(fingerTips[i]);
					break; //move on to next fingertip
				}
			}
		}
		fingerTips.swap(resPoints);

		fingerCount = fingerTips.size();

		if(fingerCount>0){ //since at 0 vector is reset anyway
			fingerChangeTracker.push_back(fingerCount);
			lastfingerCnt = fingerCount;
		}
		__android_log_print(ANDROID_LOG_INFO, TAG,
				"Exiting getFingerCount. fingercnt=%d", fingerCount);
		return fingerCount;
	}

	//checks for single finger since Cdefects cannot detect for one finger
	void singleFingerCheck(Mat &srcMat) {
		__android_log_print(ANDROID_LOG_VERBOSE, TAG, "In singleFingerCheck");
		int ht_tol = handBoundingRect.height / 6;
		Point ptHigh;
		ptHigh.y = srcMat.rows;
		//check and cycle through Cdefects if any
		vector<Point>::iterator defIterator = contours[indxBigContr].begin();
		int i = 0;	//point/defect number indicator

		//go through defects and set highest point at the highest defect point
		while (defIterator != contours[indxBigContr].end()) {
			Point currCD = (*defIterator);
			if (currCD.y < ptHigh.y) {
				ptHigh = currCD;
				__android_log_print(ANDROID_LOG_INFO, TAG, "Highest Pont.y=%d",
						ptHigh.y);
			}
			defIterator++;
		}

		//go through the polygon points to get highest point
		defIterator = hullsP[indxBigContr].begin();
		int cnt_def = 0;
		while (defIterator != hullsP[indxBigContr].end()) {
			Point currCD = (*defIterator);
			if (currCD.y < ptHigh.y + ht_tol && currCD.y != ptHigh.y
					&& currCD.x != ptHigh.x) {
				cnt_def++;
			}
			defIterator++;
		}
		//if polygon has no point within tolerable limits then
		if (cnt_def == 0) {
			fingerTips.push_back(ptHigh);
			fingerCount = fingerTips.size();
			__android_log_print(ANDROID_LOG_INFO, TAG,
					"SingleFingerCheck. FingrCnt=%d", fingerCount);
		}

		__android_log_print(ANDROID_LOG_VERBOSE, TAG,
				"Exiting singleFingerCheck");
	}

	bool isPalmClosed() {
		if (fingerCount == 0 || fingerTips.size() == 0) {
			__android_log_print(ANDROID_LOG_INFO, TAG,
					"In isPalmClosed and returning TRUE");
			return true;
		}
		return false;
	}

	//remove unnecessary convexity defects. a defect is bad or unnecessary if it makes an obtuse angle with two adjoining defects.
	//this will result in adjacent defects with only acute angles which will give us points at the tip of the fingers and
	// at the finger joining points
	void removeBadCDefects() {
		cDef_cnt = Cdefects[indxBigContr].size();
		__android_log_print(ANDROID_LOG_INFO, TAG,
				"In RemoveBadCDefects. Cdefect Count=%d", cDef_cnt);
		int minDistThresh = handBoundingRect.height / 5;
//		double maxDistThresh = handBoundingRect.height / 1.5; NOT helpful. destroys farthest away points (thumb & pinky)
		int tol_angleThresh = 100; //degrees by which is a tolerable angle
		int tol_angleMinThresh = 10;
		vector<Vec4i> resDefects;
		int startidx, endidx, faridx;
		vector<Vec4i>::iterator defIterator = Cdefects[indxBigContr].begin();

		while (defIterator != Cdefects[indxBigContr].end()) {
			Vec4i& currCd = (*defIterator);
			int start = currCd[0];
			int end = currCd[1];
			int faridx = currCd[2];
			Point startPt(contours[indxBigContr][start]);
			Point endPt(contours[indxBigContr][end]);
			Point farPt(contours[indxBigContr][faridx]);

			//based on thresholds see if they hold for the start, end and far points
			double distStart_Far = p2pDist(startPt, farPt);
			double distFar_End = p2pDist(farPt, endPt);
			double distStrt_End = p2pDist(startPt, endPt);
			double angle = getAngle(startPt, farPt, endPt);
			if (distFar_End > minDistThresh && distStart_Far > minDistThresh
					&& (angle < tol_angleThresh && angle > tol_angleMinThresh)) {
				//defect is within thresholds
				if (startPt.y
						< (handBoundingRect.height + handBoundingRect.y
								- handBoundingRect.height / 4)
						&& endPt.y
								< (handBoundingRect.height + handBoundingRect.y
										- handBoundingRect.height / 4)) {
					//defect is within bounding rectangle plus  some threshold
					resDefects.push_back(currCd);
				}
			}
			__android_log_print(ANDROID_LOG_VERBOSE, TAG,
					"Threshs'. minDist=%d, angThresh=%d", minDistThresh,
					tol_angleThresh);
			__android_log_print(ANDROID_LOG_VERBOSE, TAG,
					"Angle=%f, DistS-F=%f, DistF-E=%f", angle, distStart_Far,
					distFar_End);

			defIterator++; //iterate
		}

		//copy into Cdefects
		Cdefects[indxBigContr].swap(resDefects);
		cDef_cnt = Cdefects[indxBigContr].size();
		__android_log_print(ANDROID_LOG_INFO, TAG,
				"Exiting removeBadCdefects. Cdefect Count=%d", cDef_cnt);
	}

	//removes Cdefects when fingers join together or/and the Cdefects are way too close.
	//usage maybe redundant in the future depending on usage/requirement
	void removeJointFingerPts() {
		cDef_cnt = Cdefects[indxBigContr].size();
		__android_log_print(ANDROID_LOG_VERBOSE, TAG,
				"Inside removeJointFingerPts. Current Cdefect_cnt=%d",
				cDef_cnt);
		vector<Vec4i> resContours = Cdefects[indxBigContr];
//		float avgX, avgY;
		float distThresh = handBoundingRect.width / 6;//threshold for the distance we'll allow
		int startidx, endidx, faridx;
		int startidx2, endidx2, faridx2;
		for (int i = 0; i < resContours.size(); i++) {
			for (int j = i; j < resContours.size(); j++) {
				startidx = resContours[i][0];
				Point ptStart(contours[indxBigContr][startidx]);
				endidx = resContours[i][1];
				Point ptEnd(contours[indxBigContr][endidx]);
				startidx2 = resContours[j][0];
				Point ptStart2(contours[indxBigContr][startidx2]);
				endidx2 = resContours[j][1];
				Point ptEnd2(contours[indxBigContr][endidx2]);
				if (p2pDist(ptStart, ptEnd2) < distThresh) {
					contours[indxBigContr][startidx] = ptEnd2;
					break;
				}
				if (p2pDist(ptEnd, ptStart2) < distThresh) {
					contours[indxBigContr][startidx2] = ptEnd;
				}
			}
		}

		cDef_cnt = Cdefects[indxBigContr].size();
		__android_log_print(ANDROID_LOG_VERBOSE, TAG,
				"Exiting removeJointFingerPts. Cdefect_cnt=%d", cDef_cnt);
	}

	//-------------------------------------tracking or flow tracking functions----------------------------------------

	//sets index of biggest contour.
	void setIndexofBiggestContour() {
		indxBigContr = -1;
		int sizeOfBiggestContour = 0;
		if (contours.size() <= 0) {
			__android_log_print(ANDROID_LOG_ERROR, "HandObject",
					"Contours is null in setIndexofBiggestContour");
			return;
		}
		for (int i = 0; i < contours.size(); i++) {
			if (contours[i].size() > sizeOfBiggestContour) {
				sizeOfBiggestContour = contours[i].size();
				indxBigContr = i;
			}
		}
	}

	void writePatternToMat(Mat &srcMat) {
		//write stored pattern to Mat/image
		for (int i = 0; i < drawnPatternpts.size(); i++) {
			circle(srcMat, drawnPatternpts[i], 10, Scalar(255, 0, 0), -1, 8);
		}
	}


	void clearPatternObjects(){
		fingerChangeTracker.clear();
		drawnPatternpts.clear();
		lastfingerCnt = -1;
	}

	/*------------------------------------------Drawing functions----------------------------------------*/

	//draws filled dots/squares around cdefects at the finger tips
	void drawFingerTipPoints(Mat &srcMat) {
		int myfont = FONT_HERSHEY_PLAIN;
		for (int i = 0; i < fingerTips.size(); i++) {
			stringstream ss;
			Point pt_this = fingerTips[i];
			ss << (i + 1);
			string str = ss.str();
			putText(srcMat, str, pt_this - Point(0, 20), myfont, 1.2f,
					Scalar(10, 200, 0), 2, 8);
			circle(srcMat, pt_this, 10, Scalar(0, 255, 0), -1, 8);
			//store circle point in patternMat
			drawnPatternpts.push_back(pt_this);
		}

		__android_log_print(ANDROID_LOG_INFO, TAG,
				"Exiting drawFingerTipPoints. FingerCnt=%d", fingerCount);
	}

	//draws a bouding rectangle around the largest contour, given  a srcMat to draw on
	void drawBoundingRect(Mat &srcMat) {
		if (indxBigContr >= 0 && handBoundingRect.area() > 50) {
			__android_log_print(ANDROID_LOG_VERBOSE, TAG,
					"In drawBoundingRect. Now drawing Bounding Rectangle.");
//			checkRectBounds(srcMat);
			rectangle(srcMat, handBoundingRect, Scalar(255, 0, 0), 2, 8, 0);
			__android_log_print(ANDROID_LOG_VERBOSE, TAG,
					"In drawBoundingRect. Done drawing rectangle.");
		} else {
			__android_log_print(ANDROID_LOG_ERROR, TAG,
					"In drawBoundingRect. Can't build on contours since biggest contour index <0. index=%d",
					indxBigContr);
		}
	}

	//general drawmat function for drawing ay set of points on the srcMat
	void drawMatContours(Mat & srcMat, vector<vector<Point> > &srcContours,
			int index) {
		__android_log_print(ANDROID_LOG_VERBOSE, TAG,
				"In drawMatContours. Now Drawing Contours.");
		Scalar sc = Scalar(0, 0, 255);
		if (!srcMat.empty() && !srcContours.empty() && index >= 0)
			drawContours(srcMat, srcContours, index, sc, 1, 8);
		__android_log_print(ANDROID_LOG_VERBOSE, TAG,
				"Exiting drawMatContours. Finished Drawing Contours.");
	}

	//general drawmat function for drawing ay set of points on the srcMat
	void drawMatContours(Mat & srcMat, vector<vector<Point> > &srcContours,
			int index, Scalar sc) {
		__android_log_print(ANDROID_LOG_VERBOSE, TAG,
				"In drawMatContours with Scalar. Now Drawing Contours.");
		if (!srcMat.empty() && !srcContours.empty() && index >= 0)
			drawContours(srcMat, srcContours, index, sc, 1, 8);
		__android_log_print(ANDROID_LOG_VERBOSE, TAG,
				"Exiting drawMatContours with Scalar. Finished Drawing Contours.");
	}

	//draw convexity defects
	//assumes that Cdefects have been filtered out and index of biggest contour is filled
	void drawConvexityDefects(Mat &srcMat) {
		if (indxBigContr >= 0) {
			int startidx, endidx, faridx;
			vector<Vec4i>::iterator defIterator =
					Cdefects[indxBigContr].begin();

			while (defIterator != Cdefects[indxBigContr].end()) {
				Vec4i& currCd = (*defIterator);
				int start = currCd[0];
				int end = currCd[1];
				int faridx = currCd[2];
				Point startPt(contours[indxBigContr][start]);
				Point endPt(contours[indxBigContr][end]);
				Point farPt(contours[indxBigContr][faridx]);

				int thickness = -1;
				int rad = 10;
				circle(srcMat, startPt, rad, Scalar(0, 255, 0), thickness, 8);
				circle(srcMat, endPt, rad, Scalar(0, 255, 0), thickness, 8);
				circle(srcMat, farPt, rad, Scalar(0, 255, 0), thickness, 8);
				defIterator++;
			}
		} else {
			__android_log_print(ANDROID_LOG_ERROR, TAG,
					"In drawCdefects. Index is <0, indx=%d", indxBigContr);
		}
	}

	/*------------------------------------------destructor & helper functions----------------------------------------*/

	//return euclidean distance between two points
	double p2pDist(Point p1, Point p2) {
//		__android_log_print(ANDROID_LOG_VERBOSE, "ColorProfiler",
//				"Calculating euclidean distance");
		return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
	}

	//returns true if the HandboundRectangle fits well within limits of the screen.
	//returns false if its too large
//	bool checkRectBounds(Mat &srcMat){
//		double area = handBoundingRect.area();
//		MatSize matSize = srcMat.size;
//		if(handBoundingRect.width < srcMat.cols/2 || handBoundingRect.height < srcMat.rows/2 || area < (size_t)srcMat.size){
//			__android_log_print(ANDROID_LOG_VERBOSE, TAG, "In CheckRectBounds. Rect is within limits");
//			return true;
//		}
//		__android_log_print(ANDROID_LOG_ERROR, TAG, "Rectangle is out of bounds");
//		return false;
//	}

	//returns the angle in degrees, for three given points
	//start and end are end points. Far is the point where the lines join
	double getAngle(Point start, Point far, Point end) {
		double l1, l2, l3; //sides of our triangle
		double angle;
		double PI = 3.142;
		l1 = p2pDist(start, far);
		l2 = p2pDist(far, end);
		l3 = p2pDist(start, end); //this is the side opposite our angle theta

		angle = (pow(l1, 2) + pow(l2, 2)) - pow(l3, 2);
		angle = angle / (2 * l1 * l2);
		angle = acos(angle) * 180 / PI; //get cos^-1 and convert to degrees from radians
		return angle;
	}

	void clearAllVectors() {
		contours.clear();
		hullsI.clear();
		hullsP.clear();
		Cdefects.clear();
		fingerTips.clear();
	}

	~HandObject() {
		clearAllVectors();
	}
}
;
