package com.boyliang.componentsuperaccessor;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public final class FakeAccountService extends Service {

	@Override
	public IBinder onBind(Intent intent) {
		return new FakeAccountAuthenticator(this).getIBinder();
	}
	
	@Override
	public void onCreate() {
		super.onCreate();
		Log.i("TTT", "FakeAccountService.onCreate");
	}

}
