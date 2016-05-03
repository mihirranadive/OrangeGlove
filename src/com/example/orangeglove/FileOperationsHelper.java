package com.example.orangeglove;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.content.res.AssetManager;
import android.os.Environment;
import android.util.Log;

//class is meant to have all file operations related functions and objects just to keep them separate
public class FileOperationsHelper {

	String filePath, fileName, dirPath; // dir path to a collection of files
	File fileObj, outFile;
	AssetManager assetMgr;
	OutputStream outStream;
	InputStream inStream;
	
	
	public void setPaths(String fpath){
		filePath = fpath; //include filename+extension
		//set other values from string operations
		
	}
	
	
	//reads default assets directory and sets values to it
	public void setPathsFromEnv(AssetManager astmgr){
		assetMgr = astmgr;
		
	}

	
}
