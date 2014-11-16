package com.boyliang.componentsuperaccessor;

import android.content.Context;
import android.util.Log;

import com.boyliang.componentsuperaccessor.context.ContextHunter;

/**
 * 
 * @author boyliang
 *
 */
public final class Entry {
	private static Context sContext;
	
	final static class MyUIThread implements Runnable{
		
		public void run() {
			int i = 0;
			
			while(true){
				Log.i("TTT", "i=" + i);
				try {
					Thread.sleep(1000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				i++;
			}
		}
	}
	
	public static int invoke(){
		sContext = ContextHunter.getContext();
		
		//TODO here to do what you want
		Log.i("TTT", "context is " + sContext.getClass());
		new Thread(new MyUIThread()).start();
		return 0;
	}
}
