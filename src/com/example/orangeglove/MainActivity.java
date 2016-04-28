package com.example.orangeglove;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.*;
import android.content.Intent;

public class MainActivity extends Activity {

	public static final String EXTRA_MESSAGE = "com.example.orangeglove.MESSAGE";
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
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
	
	
	public void redirecttoHSRecognizer(View view){
		Log.i("redirecttoHSRecognizer", "Redirecting to HandRecognizer activity");
		Intent HS_intent = new Intent(this, HandRecognizer.class);
		//EditText editText = (EditText) findViewById(R.id.edit_message);
	    //String message = editText.getText().toString();
	    HS_intent.putExtra(EXTRA_MESSAGE, "Opening HSRecognizer");
	    startActivity(HS_intent);
	}
	
	public void redirecttoColorProfiler(View view){
		Log.i("redirecttoColorProfiler", "Redirecting to ColorProfiler activity");
		Intent CP_intent = new Intent(this, ColorProfiler.class);
		//EditText editText = (EditText) findViewById(R.id.edit_message);
	    //String message = editText.getText().toString();
		//invokeNativeCode();
	    CP_intent.putExtra(EXTRA_MESSAGE, "Opening Color Profiler");
	    startActivity(CP_intent);
	}
	
	//public native void invokeNativeCode();

}
