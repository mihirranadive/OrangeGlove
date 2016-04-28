package com.example.orangeglove;

import java.io.*;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Scalar;
import org.opencv.core.Size;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SurfaceView;
import android.view.View;
//import android.widget.RelativeLayout;
//import android.widget.TextView;

public class ColorProfiler extends Activity implements CvCameraViewListener2 {

	public final static String EXTRA_MESSAGE = "com.example.orangeglove.ColorProfiler";
	public final static String TAG = "ColorProfiler";

	// opencv vars
	private Mat currMat, prevMat;
	private Mat mRgba, mGray, BGRMat, HSVMat, sc_LBoundMat, sc_UBoundMat;
	private Mat fore, back;

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

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.color_profiler, menu);
		Log.i(TAG, "called onCreateOptionsMenu");
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

	@Override
	public void onResume() {
		super.onResume();
		// if (!OpenCVLoader.initDebug()) {
		// Log.d(TAG,
		// "Internal OpenCV library not found. Using OpenCV Manager for initialization");
		OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this,
				mLoaderCallback);
		// } else {
		// Log.d(TAG,
		// "OpenCV library found inside App package. Using it from within the App!");
		// mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
		// }
	}

	/*----------------------------------------ANDROID INIT FUNCTIONS------------------------------*/
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_color_profiler);

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

	/*-------------------------------------ACTIVITY INTERACTIONS----------------------------------*/

	public void DoneButtonClicked(View view) {
		// done button was clicked.
		// save profile to file and go back to main page
		// Saving profile to file
		writeBoundMattoFiles();
		// navigating back to main activity
		Intent CP_intent = new Intent(this, MainActivity.class);
		// EditText editText = (EditText) findViewById(R.id.edit_message);
		// String message = editText.getText().toString();
		CP_intent.putExtra(EXTRA_MESSAGE, "Opening Main");
		startActivity(CP_intent);
	}

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
		mRgba = new Mat(height, width, CvType.CV_64FC4);
		mGray = new Mat(height, width, CvType.CV_64FC4);
		currMat = new Mat(height, width, CvType.CV_8UC3);
		back = new Mat(height, width, CvType.CV_64FC4);
		fore = new Mat(height, width, CvType.CV_8UC1);
		BGRMat = new Mat(height, width, CvType.CV_64FC4);
		sc_LBoundMat = new Mat(7, 1, CvType.CV_8UC3, new Scalar(0, 0, 0));
		sc_UBoundMat = new Mat(7, 1, CvType.CV_8UC3, new Scalar(0, 0, 0));
		HSVMat = new Mat(height, width, CvType.CV_8UC3);
		Log.d("OnCameraViewStopped",
				"ALL declarations of Mats and OpenCV Objects done");
	}

	public void onCameraViewStopped() {
		Log.d("OnCameraViewStopped",
				"Calling all release of Mat objects and Contours");
		// release all declared Mats
		mRgba.release();
		mGray.release();
		currMat.release();
		back.release();
		fore.release();
		BGRMat.release();
		sc_LBoundMat.release();
		sc_UBoundMat.release();
		HSVMat.release();
		Log.d("OnCameraViewStopped", "ALL releases done");
	}

	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		mRgba = inputFrame.rgba();
		mGray = inputFrame.gray();
		
		Log.d("OnCameraFrame", "Invoking NDK");
		invokeNativeColorProfiler(mGray.getNativeObjAddr(),
				mRgba.getNativeObjAddr(), HSVMat.getNativeObjAddr(),
				fore.getNativeObjAddr(), back.getNativeObjAddr(),
				sc_LBoundMat.getNativeObjAddr(),
				sc_UBoundMat.getNativeObjAddr());
		Log.d("OnCameraFrame", "Back from NDK");
		// Scalar sc = new Scalar(0,255,9);

		System.out.println("LowerBScalarMat==>" + sc_LBoundMat.dump());
		System.out.println("\n\n\n ");
		System.out.println("UpperBScalarMat==>" + sc_UBoundMat.dump());

		Log.i("OnCameraFrame",
				"Now to write the bound vars per MedRect to File. For Now this is done only when the done button is clicked");
		return mRgba;
	}

	/*---------------------------HELPER Functions and other types---------------------------------*/

	// writes data from lower bound and upper bound Mats into files
	void writeBoundMattoFiles() {
		Log.d("writeBoundMattoFile", "Just in writer function");
//		File file_LB = null;
//		File file_UB = null;
		FileOutputStream fos_LB = null;
		FileOutputStream fos_UB = null;
		
		try {
			Log.d("writeBoundMattoFile", "Just in try block");
//			file_LB = new File("/DataFiles/lowerBoundMat.txt");
//			file_UB = new File("/DataFiles/upperBoundMat.txt");
			
			//writes the file internally
			//in DDMS its present in data>data>mypackage>files
			fos_LB = openFileOutput("lowerBound.txt", MODE_PRIVATE);
			fos_UB = openFileOutput("upperBound.txt", MODE_PRIVATE);

			fos_LB.write(sc_LBoundMat.dump().getBytes());
			fos_UB.write(sc_UBoundMat.dump().getBytes());

			fos_LB.flush();
			fos_LB.close();
			fos_UB.flush();
			fos_UB.close();
		} catch (FileNotFoundException e) {
			Log.e("writeBoundMattoFile", "FileNotFoundException");
			e.printStackTrace();
		} catch (IOException e) {
			Log.e("writeBoundMattoFile", "IOException while writing file");
			e.printStackTrace();
		}

		Log.d("writeBoundMattoFile", "Exiting write function");
	}

	// declaration for NDK calling function. 
	public native void invokeNativeColorProfiler(long matAddrGr,
			long matAddrRgba, long matAddrHSVMat, long mataddreFore,
			long mataddrBack, long matSc_LBoundMataddr, long matSc_UBoundMataddr);

	static {
		System.loadLibrary("orangeglove");
		// System.loadLibrary("opencv_java3");
	}

}
