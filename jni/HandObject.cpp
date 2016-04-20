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

using namespace std;
using namespace cv;

//class contains all Hand related Objects like contours around hand bounding rectangle/polygon or area
class HandObject {

public:

	//contour and bounding related variables
	bool isHand; //true when hand is present/open/detected
	Scalar avgColor; // this is determined from the color profiler
	vector<vector<Point> > contours;
	vector<vector<int> > hullI;
	vector<vector<Point> > hullP;
	vector<vector<Vec4i> > Cdefects;
	vector<Point> fingerTips;

	Rect boundingRect;
	int indexofBiggestContour;
	//tracking and flow related variables
	int fingerCount, prevFingerCnt;

	/*--------------------------hand detect related functions----------------------------*/
	int getfingerCount() {
		Mat t1;

		return 1;
	}

	//remove unnecessary convexity defects. a defect is bad or unnecessary if it makes an obtuse angle with two adjoining defects.
	//this will result in adjacent defects with only acute angles which will give us points at the tip of the fingers and
	// at the finger joining points
	void removeBadCDefects() {

	}

	//tracking or flow tracking functions
	void drawFingerTipPoints() {
		Mat src;
		flip(src, src, 1);
	}

	//sets index of biggest contour.
	void setIndexofBiggestContour() {
		indexofBiggestContour = -1;
		int sizeOfBiggestContour = 0;
		for (int i = 0; i < contours.size(); i++) {
			if (contours[i].size() > sizeOfBiggestContour) {
				sizeOfBiggestContour = contours[i].size();
				indexofBiggestContour = i;
			}
		}
	}
};
