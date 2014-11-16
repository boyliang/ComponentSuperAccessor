package com.boyliang.componentsuperaccessor;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

/**
 * 
 * @author boyliang
 *
 */
public final class SuperAccessor{
	private volatile static Intent sIntentForActivity = null;
	private volatile static Intent sIntentForBroadcast = null;
	
	private static void askSettingAddAccount(Context context){
		Intent intent = new Intent();
		intent.setComponent(new ComponentName("com.android.settings", "com.android.settings.accounts.AddAccountSettings"));
		intent.setAction(Intent.ACTION_RUN);
		intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		String authTypes[] = { "com.android.testplugin.AccountType" };
		intent.putExtra("account_types", authTypes);
		context.startActivity(intent);
	}
	
	static Intent getIntentForActivity(){
		synchronized (SuperAccessor.class) {
			Intent old = sIntentForActivity;
			sIntentForActivity = null;
			return old;
		}
	}
	
	static void setIntentForActivity(Intent intent){
		synchronized (SuperAccessor.class) {
			sIntentForActivity = intent;
		}
	}
	
	static Intent getIntentForBroadcast(){
		synchronized (SuperAccessor.class) {
			Intent old = sIntentForBroadcast;
			sIntentForBroadcast = null;
			return old;
		}
	}
	
	static void setIntentForBroadcast(Intent intent){
		synchronized (SuperAccessor.class) {
			sIntentForBroadcast = intent;
		}
	}
	
	/**
	 * start activity by system uid
	 * @param context
	 * @param intent
	 */
	public static boolean startActivity(Context context, Intent intent) {
		if (sIntentForActivity != null) {
			Log.w("TTT", "Last activity still starting.");
			return false;
		} else {
			setIntentForActivity(intent);
			askSettingAddAccount(context);
			return true;
		}
	}
	
	/**
	 * send broadcast by system uid
	 * @param context
	 * @param intent
	 */
	public static boolean sendBroadcast(Context context, Intent intent){
		if (sIntentForBroadcast != null) {
			Log.w("TTT", "Last broadcast still sending.");
			return false;
		} else {
			setIntentForBroadcast(intent);
			askSettingAddAccount(context);
			return true;
		}
	}
}
