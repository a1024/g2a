//MainActivity.java - Grapher 2A main activity.
//Copyright (C) 2019-2020  Ayman Wagih Mohsen
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <https://www.gnu.org/licenses/>.

package com.example.grapher2;

import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;

import android.app.Activity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Rect;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity
{
	GL2JNIView mView;

	EditText editBox;
	//TextView resultBox;
	Boolean eBoxVisible=true;
	int softKbHeight=0, old_kb_diff=0;//
	boolean justResumed=true;
	boolean landscape, oldLandscape;
	boolean[] kb;

	ConstraintLayout main_cl;

	// Used to load the 'native-lib' library on application startup.
	//static
	//{
	//	System.loadLibrary("native-lib");
	//}

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		c=this;
		kb=new boolean[300];
		main_cl=findViewById(R.id.constraintLayout);
		main_cl.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener()
		{
			@Override public void onGlobalLayout()
			{
				Rect r = new Rect();
				main_cl.getWindowVisibleDisplayFrame(r);
				int h2=main_cl.getRootView().getHeight(), h1=r.bottom-r.top;
				int diff=h2-h1;
				if(diff<old_kb_diff&&!justResumed)//kb has just disappeared and not resume
				//	if(diff==0)//no soft kb
				{
					if(eBoxVisible)//redundant
					{
						hideEditBox();
						GL2JNILib.toggleInputBox();
						eBoxVisible=false;
					}
				}
				if(diff!=0)
					softKbHeight=diff;
				old_kb_diff=diff;
				justResumed=false;
			}
		});
		mView=findViewById(R.id.glSurfaceView);
		editBox=findViewById(R.id.editText);
		mView.editBox=editBox;
		editBox.addTextChangedListener(new TextWatcher()
		{
			@Override
			public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2)
			{
			}
			@Override
			public void onTextChanged(CharSequence charSequence, int start, int before, int count)
			{
				if(oldLandscape==landscape)
				{
					int quit=GL2JNILib.changeText(charSequence.toString(), start, before, count);
					if(quit==42)
						System.exit(0);
				}
				oldLandscape=landscape;
				//resultBox.setText(editBox.getText());
			}
			@Override
			public void afterTextChanged(Editable editable)
			{
				//editBox.setText("LOL_1");//CRASH
				//editBox.setText(mView.err);//CRASH
			}
		});
		editBox.bringToFront();
		showEditBox();

		//// Example of a call to a native method
		//TextView tv=findViewById(R.id.sample_text);
		//tv.setText(stringFromJNI());
	}
	@Override protected void onPause()
	{
		justResumed=true;//onGlobalLayout gets called before onResume when unlocking
		super.onPause();
		mView.onPause();
		//mView.setVisibility(View.GONE);
	}
	@Override protected void onResume()
	{
		//	justResumed=true;
		super.onResume();
		mView.onResume();
		//mView.setVisibility(View.VISIBLE);
		GL2JNILib.resume();
		if(GL2JNIView.err!=null&&!GL2JNIView.err.isEmpty())
			editBox.setText(GL2JNIView.err);//
	}
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);

		landscape=newConfig.orientation==Configuration.ORIENTATION_LANDSCAPE;
		GL2JNILib.switchOrientation(newConfig.orientation==Configuration.ORIENTATION_LANDSCAPE);
		//if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
		//	// landscape
		//} else if (newConfig.orientation == Configuration.ORIENTATION_PORTRAIT) {
		//	// portrait
		//}
	}
	static Context c;//TODO: fix memory leak
	public static void textToClipboard(String text)
	{
		ClipboardManager clipboard=(ClipboardManager)c.getSystemService(Context.CLIPBOARD_SERVICE);
		if(clipboard==null)
			return;
		ClipData clip=ClipData.newPlainText(null, text);
		clipboard.setPrimaryClip(clip);

		CharSequence msg="Copied to clipboard.";
		Toast t=Toast.makeText(c, msg, Toast.LENGTH_SHORT);
		t.show();
	}

	InputMethodManager imm;
	void showEditBox()
	{
		editBox.setVisibility(View.VISIBLE);//show edit box

		editBox.setFocusableInTouchMode(true);//request focus
		editBox.requestFocus();

		if(imm==null)
			imm=(InputMethodManager)this.getSystemService(Context.INPUT_METHOD_SERVICE);//show soft keyboard
		imm.showSoftInput(editBox, InputMethodManager.SHOW_IMPLICIT);
	}
	void hideEditBox()
	{
		editBox.setVisibility(View.GONE);//hide edit box
	}
	@Override public void onBackPressed()
	{
		//super.onBackPressed();//removes application from screen
		if(eBoxVisible)
		{
			hideEditBox();
			//interactionHappened=true;
		}
		else
			showEditBox();
		eBoxVisible=!eBoxVisible;
		GL2JNILib.toggleInputBox();
	}
	int keyDown_counter;
	@Override public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		++keyDown_counter;
		if(!kb[keyCode])
		{
			//GL2JNILib.inputKeyDown(keyCode);
			GL2JNILib.inputKeyDown(keyDown_counter);
			kb[keyCode]=true;
		}
		return super.onKeyDown(keyCode, event);
	}
	@Override public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		GL2JNILib.inputKeyUp(keyCode);
		kb[keyCode]=false;
		return super.onKeyUp(keyCode, event);
	}

	//A native method that is implemented by the 'native-lib' native library, which is packaged with this application.
	//public native String stringFromJNI();
}
