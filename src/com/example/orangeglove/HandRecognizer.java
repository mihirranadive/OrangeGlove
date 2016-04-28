package com.example.orangeglove;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.core.Core;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Scalar;
import org.opencv.imgproc.Imgproc;

import android.app.Activity;
//import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SurfaceView;

public class HandRecognizer extends Activity implements CvCameraViewListener2 {

	public final static String EXTRA_MESSAGE = "com.example.orangeglove.HandRecognizer";
	public final static String TAG = "HandRecognizer";

	public final static int SAMPLE_CNT = 7;
	public static int framecnt = 0;
	
	// opencv vars
	private Mat currMat, prevMat;
	private Mat mRgba, mGray, BGRMat, HSVMat, sc_LBoundMat, sc_UBoundMat;
	private Mat pattrnMat;
	private Mat fore, back;
	private Scalar scal_LB[] = new Scalar[SAMPLE_CNT];
	private Scalar scal_UB[] = new Scalar[SAMPLE_CNT];

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
		Log.i(TAG, "in OnCreate Method");
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

		mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.HSRecognizerCameraView);
		mOpenCvCameraView.setVisibility(SurfaceView.VISIBLE);
		mOpenCvCameraView.setCvCameraViewListener(this);
		
		Log.i(TAG, "Exiting OnCreate Method");
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

	/*-------------------------------------ACTIVITY INTERACTIONS----------------------------------*/

	// public void myfun_FOO()
	// {}

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
		pattrnMat = new Mat(height, width, CvType.CV_8UC3);
		sc_LBoundMat = new Mat(7, 1, CvType.CV_8UC4, new Scalar(0, 0, 0));
		sc_UBoundMat = new Mat(7, 1, CvType.CV_8UC4, new Scalar(0, 0, 0));
		HSVMat = new Mat(height, width, CvType.CV_8UC3);
		for(int i=0;i<SAMPLE_CNT;i++){
			scal_LB[i] = new Scalar(0,0,0);
			scal_UB[i] = new Scalar(0,0,0);
		}
		
		Log.d("OnCameraFrame", "Getting Bound Vars from file into Mats");
		getBoundsFromFile();
		
		Log.d("OnCameraViewStarted",
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
//		prevMat.release();
		Log.d("OnCameraViewStopped", "ALL releases done");
	}

	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		mRgba = inputFrame.rgba();
		mGray = inputFrame.gray();
//		mGray.copyTo(fore);
//		Imgproc.pyrDown(mRgba, BGRMat);
		
		
		

		Imgproc.cvtColor(mRgba, mRgba, Imgproc.COLOR_RGBA2RGB);
		Log.d("OnCamerFrame", "Invoking NDK");
		invokeNativeHSRecogCode(mGray.getNativeObjAddr(), mRgba.getNativeObjAddr(), HSVMat.getNativeObjAddr(),
				fore.getNativeObjAddr(), back.getNativeObjAddr(), sc_LBoundMat.getNativeObjAddr(),
				sc_UBoundMat.getNativeObjAddr(), pattrnMat.getNativeObjAddr());
		Log.i("OnCamerFrame", "Back From NDK");	
	
		
		return mRgba;
	}

	/*---------------------------HELPER Functions and other types---------------------------------*/

	//reads the file into lower and upper bound matrices
	public void getBoundsFromFile(){
		Log.d("ReadBoundFiletoMat", "Just in Reader function");
		FileInputStream fistream_LB = null;
		FileInputStream fistream_UB = null;
		try {
			fistream_LB = openFileInput("lowerBound.txt");
			fistream_UB = openFileInput("upperBound.txt");
			
			int c;
			String fstr_LB = "";
			while( (c = fistream_LB.read()) != -1){
			   fstr_LB = fstr_LB + Character.toString((char)c);
			}
			
			int c2;
			String fstr_UB = "";
			while( (c2 = fistream_UB.read()) != -1){
			   fstr_UB = fstr_UB + Character.toString((char)c2);
			} 
			
			fillMatFromString(fstr_LB, "LBMat");
			fillMatFromString(fstr_UB, "UBMat");
			
			fistream_LB.close();
			fistream_UB.close();
		} catch (FileNotFoundException e) {
			Log.e("ReadBoundFiletoMat", "FileNotFoundException!!!");
			e.printStackTrace();
		} catch (IOException e) {
			Log.e("ReadBoundFiletoMat", "IOException!!!");
			e.printStackTrace();
		}
		Log.d("ReadBoundFiletoMat", "Exiting Reader function. Done reading data from both files");
	}
	
	//fill matrices with string components gathered from bounding files
	//each line/row of strArr is a row in the matrix
	//the Mat is of the type Mat(7,1,CV_8uC3, Scalar(0,0,0)) -- scalar values are updated below from the boundalue files
	public void fillMatFromString(String fileString, String Matname){		
		//filter the string which contains the whole mat
		
		//remove bad chars and trim
		fileString = fileString.replace('[', ' ').replace(']', ' ');
		fileString = fileString.replace("\n", "").replace("\r", "");
		fileString = fileString.replace(" ", "");
		fileString = fileString.trim();
		
//		System.out.println("String temp==>" + fileString);
		String str_lines[] = fileString.split(";");			

		for(int i=0;i<SAMPLE_CNT;i++){
			String str_int[] = str_lines[i].split(",");
						
			if(Matname.equals("LBMat") || Matname.contains("LB")){				
				//translate string vals per row to scalar values
//				Log.d("fillMatFromString", "filling in Scalar LB");
				for(int j=0;j<3;j++){
//					System.out.println("curr num==>>" + str_int[j]);
					scal_LB[i].val[j] = Double.parseDouble(str_int[j]);
				}
				scal_LB[i].val[3] = 0;
				//fill LB mat with scalar values
				int chan = sc_LBoundMat.channels();
				//scalarArray[i].val[0] = myMat.data[chan * myMat.cols * i + j * chan + 0]; //R
//				Log.d("fillMatFromString", "filling in Mat using Put LB");
				sc_LBoundMat.put(i, 0, scal_LB[i].val);
			}
			else if(Matname.equals("UBMat") || Matname.contains("UB")){
				//translate string vals per row to scalar values
//				Log.d("fillMatFromString", "filling in Scalar UB");
				for(int j=0;j<3;j++){
//					System.out.println("curr num==>>" + str_int[j]);
					scal_UB[i].val[j] = Double.parseDouble(str_int[j]);
				}
				scal_UB[i].val[3] = 0;
				//fill LB mat with scalar values
				int chan = sc_LBoundMat.channels();
				//scalarArray[i].val[0] = myMat.data[chan * myMat.cols * i + j * chan + 0]; //R
//				Log.d("fillMatFromString", "filling in Mat using Put UB");				
				sc_UBoundMat.put(i, 0, scal_UB[i].val);			
			}
		}
		
		if(Matname.equals("LBMat") || Matname.contains("LB"))
			System.out.println("LBMat=>>" + sc_LBoundMat.dump());
		else if(Matname.equals("UBMat") || Matname.contains("UB"))
			System.out.println("UBMat=>>" + sc_UBoundMat.dump());
		
	}
	
	// declaration for NDK calling function.
	// pass the string parameter to define which path you are going in your
	// program. it can be either ColorProfiler or HSRecognizer
	public native void invokeNativeHSRecogCode(long matAddrGr,
			long matAddrRgba, long matAddrHSVMat, long mataddreFore,
			long mataddrBack, long matSc_LBoundMataddr, long matSc_UBoundMataddr, long pattrnMataddr);


	static {
		System.loadLibrary("orangeglove");
		// System.loadLibrary("opencv_java3");
	}

}
