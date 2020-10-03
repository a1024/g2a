//GL2JNILib.java - Native code wrapper
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

//import android.content.ClipData;
//import android.content.ClipboardManager;
//import android.content.Context;

public class GL2JNILib
{
	static
	{
		System.loadLibrary("native-lib");
	}
	public static native String init(int width, int height, int hard_reset);
	public static native void step(int cursor);
	public static native boolean touch(float x, float y, int msg, int idx);
	public static native int changeText(String text, int start, int before, int count);
	public static native void toggleInputBox();
	public static native void inputKeyDown(int keyCode);
	public static native void inputKeyUp(int keyCode);
	public static native void resume();
	public static native void switchOrientation(boolean landscape);
}
