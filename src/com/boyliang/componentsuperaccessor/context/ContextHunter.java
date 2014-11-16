package com.boyliang.componentsuperaccessor.context;

import android.app.Application;
import android.content.Context;

/**
 * Context
 * 
 * @author boyliang
 * 
 */
public final class ContextHunter {
	private static Context sContext = null;

	public static Context getContext() {

		if (sContext == null) {

			synchronized (ContextHunter.class) {
				if (sContext == null) {
					sContext = searchContextForSystemServer();
					if (sContext == null) {
						sContext = searchContextForZygoteProcess();
					}
				}
			}
		}

		return sContext;
	}

	private static Context searchContextForSystemServer() {
		Context result = null;

		try {
			result = (Context) ActivityThread.getSystemContext();
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}

		return result;

	}

	private static Context searchContextForZygoteProcess() {
		Context result = null;
		
		try {
			Object obj = RuntimeInit.getApplicationObject();
			if (obj != null) {
				obj = ApplicationThread.getActivityThreadObj(obj);
				if (obj != null) {
					result = (Application) ActivityThread.getInitialApplication(obj);
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return result;
	}
}
