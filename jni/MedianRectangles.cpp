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

class MedianRectangle {
public:
	Point up_corner, low_corner;
	float width, height;
	float Rectarea;
	Scalar borderColor;
	int thickness;

	MedianRectangle() {
		up_corner = Point(0, 0);
		Rectarea = 20.0;
		borderColor = Scalar(0, 255, 0);
		thickness = 1;
		width = height = 1;
	}

	MedianRectangle(Point upperCorner, Point lowercorner) {
		up_corner = upperCorner;
		low_corner = lowercorner;
		Rectarea = getRectArea(up_corner, low_corner);
		borderColor = Scalar(0,255,0);
		thickness = 1;
		setWidthAndHeight();
	}

	MedianRectangle(Point upperCorner, Point lowercorner, Mat src) {
			up_corner = upperCorner;
			low_corner = lowercorner;
			Rectarea = getRectArea(up_corner, low_corner);
			borderColor = Scalar(0,255,0);
			thickness = 1;
			setWidthAndHeight();
			Rectarea = getRectArea(up_corner, low_corner);
		}

	MedianRectangle(Point upperCorner, Point lowercorner, Scalar color) {
		up_corner = upperCorner;
		low_corner = lowercorner;
		borderColor = color;
		thickness = 1;
		setWidthAndHeight();
		Rectarea = getRectArea(up_corner, low_corner);
	}

	MedianRectangle(Point upperCorner, Point lowerCorner, Scalar color,
			int borderthickness) {
		up_corner = upperCorner;
		low_corner = lowerCorner;
		Rectarea = getRectArea(up_corner, low_corner);
		borderColor = color;
		thickness = borderthickness;
		setWidthAndHeight();
	}

	float getRectArea(Point up, Point low) {
		return fabs(width*height); //width * height
	}

	void setWidthAndHeight(){
		width = fabs(up_corner.x - low_corner.x);
		height = fabs(up_corner.y - low_corner.y);
	}

	void drawRectangle(Mat &src) {
		rectangle(src, up_corner, low_corner, borderColor, thickness);
	}


};
