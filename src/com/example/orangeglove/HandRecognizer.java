package com.example.orangeglove;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.core.Mat;

import android.app.Activity;
//import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SurfaceView;

public class HandRecognizer extends Activity implements CvCameraViewListener2 {

	public final static String EXTRA_MESSAGE = "com.example.orangeglove.ColorProfiler";
	public final static String TAG = "HandRecognizer";

	/*---------------------------------------OPENCV INIT FUNCTIONS--------------------------*/

	protected CameraBridgeViewBase mOpenCvCameraView;
	protected BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
		@Override
		public void onManagerConnected(int status) {
			switch (status) {
			case LoaderCallbackInterface.SUCCESS: {
				Log.i(TAG, "OpenCV loaded successfully");

				// Load native library after(!) OpenCV initialization
				// System.loadLibrary("mixed_sample");
				mOpenCvCameraView.enableView();
			}
				break;
			default: {
				super.onManagerConnected(status);
			}
				break;
			}
		}
	};

	/*----------------------------------------ANDROID INIT FUNCTIONS------------------------------*/
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_hand_recognizer);

		// receive the intent from the parent/MainActivity
		// this will give us the key-value pairs passed from that Activity
		/*
		 * Intent intent = getIntent(); String message =
		 * intent.getStringExtra(MainActivity.EXTRA_MESSAGE); TextView textView
		 * = new TextView(this); textView.setTextSize(40);
		 * textView.setText(message);
		 * 
		 * RelativeLayout layout = (RelativeLayout) findViewById(R.id.content);
		 * layout.addView(textView);
		 */

		mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.CProfilerCameraView);
		mOpenCvCameraView.setVisibility(SurfaceView.VISIBLE);
		mOpenCvCameraView.setCvCameraViewListener(this);

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.hand_recognizer, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	/*-------------------------------------ACTIVITY INTERACTIONS----------------------------------*/

	//public void myfun_FOO()
	//{}
	
	/*---------------------------ALL OPENCV and CAMERA interactions---------------------------------*/

	@Override
	public void onPause() {
		super.onPause();
		if (mOpenCvCameraView != null)
			mOpenCvCameraView.disableView();
	}

	public void onDestroy() {
		super.onDestroy();
		if (mOpenCvCameraView != null)
			mOpenCvCameraView.disableView();
	}

	public void onCameraViewStarted(int width, int height) {
		// declare all matrices and OpenCV Objects here
		// this is because OpenCV functions are in C/C++ and Java's garbage
		// collector cannot recognize what they are and
		// hence it can't deallocate them.

		Log.d("OnCameraViewStopped",
				"ALL declarations of Mats and OpenCV Objects done");
	}

	public void onCameraViewStopped() {
		Log.d("OnCameraViewStopped",
				"Calling all release of Mat objects and Contours");
		// release all declared Mats

		Log.d("OnCameraViewStopped", "ALL releases done");
	}

	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		return inputFrame.rgba();
	}

	/*---------------------------HELPER Functions and other types---------------------------------*/

	// declaration for NDK calling function.
	// pass the string parameter to define which path you are going in your
	// program. it can be either ColorProfiler or HSRecognizer
	public native void invokeNativeHSRecogCode(long matAddrGr,
			long matAddrRgba, long mataddreFore, long mataddrBack,
			String progPath);

	static {
		System.loadLibrary("orangeglove");
		// System.loadLibrary("opencv_java3");
	}
	
}
