#ifndef __START_ROUTINE__H__
#define __START_ROUTINE__H__

#include <cutils/log.h>
#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <android_runtime/AndroidRuntime.h>
#include "start_routine.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TTT"

using namespace android;

static const char JSTRING[] = "Ljava/lang/String;";
static const char JCLASS_LOADER[] = "Ljava/lang/ClassLoader;";
static const char JCLASS[] = "Ljava/lang/Class;";

//#define ALOGI(fmt...) if(0)

#define check_value(x) 														\
	ALOGI("%s: %p", #x, x);									\
	if(x == NULL){ 															\
		ALOGE("%s was NULL", #x);											\
	   	exit(0);															\
	}																		\

static char *find_apk_path()
{
	const char *target_apk_fmt = "/data/app/com.android.testplugin-%d.apk";
	char *result = NULL;
	char target_apk[128];

	for (int i = 0; i < 10; i++) {
		snprintf(target_apk, sizeof(target_apk), target_apk_fmt, i);
		if (!access(target_apk, R_OK)) {
			result = strdup(target_apk);
			break;
		}
	}

	return result;
}

static jobject get_system_classloader(JNIEnv* jnienv)
{
	char buf[128];
	jclass class_loader_claxx = jnienv->FindClass("java/lang/ClassLoader");
	snprintf(buf, sizeof(buf), "()%s", JCLASS_LOADER);
	jmethodID getSystemClassLoader_method = jnienv->GetStaticMethodID(class_loader_claxx, "getSystemClassLoader", buf);
	return jnienv->CallStaticObjectMethod(class_loader_claxx, getSystemClassLoader_method);
}

static jstring tojstring(JNIEnv* jnienv, const char* c_str)
{
	return jnienv->NewStringUTF(c_str);
}

static int load_dex(JNIEnv* jnienv, const char* apk_path, const char* dexout_dir, const char* classname, const char* method)
{
	char buf[256];
	jclass dexloader_claxx = jnienv->FindClass("dalvik/system/DexClassLoader");
	snprintf(buf, sizeof(buf), "(%s%s%s%s)V", JSTRING, JSTRING, JSTRING, JCLASS_LOADER);
	jmethodID dexloader_init_method = jnienv->GetMethodID(dexloader_claxx, "<init>", buf);
	jobject system_class_loader = get_system_classloader(jnienv);
	jobject dex_loader_obj = jnienv->NewObject(dexloader_claxx, dexloader_init_method, tojstring(jnienv, apk_path), tojstring(jnienv, dexout_dir), NULL, system_class_loader);

	snprintf(buf, sizeof(buf), "(%s)%s", JSTRING, JCLASS);
	jmethodID loadClass_method = jnienv->GetMethodID(dexloader_claxx, "loadClass", buf);

	jstring class_name_str = tojstring(jnienv, classname);
	jclass entry_class = static_cast<jclass>(jnienv->CallObjectMethod(dex_loader_obj, loadClass_method, class_name_str));
	jmethodID invoke_method = jnienv->GetStaticMethodID(entry_class, method, "()I");
	jint result = jnienv->CallStaticIntMethod(entry_class, invoke_method);
	return result;
}

void* start_routine(void *args)
{
	ALOGI(">>>>>>>>>>>>>I am in, I am a bad boy!!!!<<<<<<<<<<<<<<");

	JNIEnv *jnienv = NULL;
	JavaVM *javavm = NULL;
	bool should_detach = false;
	char *apk_path = NULL;

	javavm = AndroidRuntime::getJavaVM();
	if(javavm != NULL){

		javavm->GetEnv((void **)&jnienv, JNI_VERSION_1_4);

		if(jnienv == NULL){
			should_detach = true;
			javavm->AttachCurrentThread(&jnienv, NULL);
		}

		if(jnienv != NULL){

			apk_path = find_apk_path();
			if(!apk_path){
				ALOGE("Could found't apk_path");
				goto bails;
			}

			const char *dexout_dir = "/data/data/com.tencent.mm/dexout";

			int state = 0;

			if (access(dexout_dir, R_OK | W_OK | X_OK) != 0) {
				int state = mkdir(dexout_dir, S_IRWXU);
				if (state) {
					ALOGE("Could create dexout dir");
					goto bails;
				}
			}

			state = load_dex(jnienv, apk_path, dexout_dir, "com.android.testplugin.Entry", "invoke");
			if(state != 0){
				ALOGE("Load dex error!!");
				goto bails;
			}
		}
	}

bails:
	if(apk_path){
		free(apk_path);
		apk_path = NULL;
	}

	if(javavm && jnienv && should_detach){
		javavm->DetachCurrentThread();
		jnienv = NULL;
		javavm = NULL;
	}
	return NULL;
}

#endif
