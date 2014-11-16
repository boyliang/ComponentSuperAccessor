package com.boyliang.componentsuperaccessor;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class FakePlugin extends Service {

	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}

	@Override
	public void onCreate() {
		super.onCreate();
		Log.i("TTT", "TestPlugin.onCreate");
	}

}
