package com.example.orangeglove;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.core.Mat;

import android.content.Context;
import android.util.AttributeSet;

//puropse of this class is to make the image full screen when in portrait mode

//for now its left out since solution is not complete
//refer link 
// http://stackoverflow.com/questions/28562627/how-to-set-opencvs-camera-to-display-preview-in-both-portrait-orientation-and-f

public class ScreenSizer extends CameraBridgeViewBase{

	public ScreenSizer(Context context, AttributeSet attrs) {
		super(context, attrs);
		// TODO Auto-generated constructor stub
	}

	@Override
	protected boolean connectCamera(int width, int height) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	protected void disconnectCamera() {
		// TODO Auto-generated method stub
		
	}
	

}
