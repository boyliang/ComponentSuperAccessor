/*
 * Copyright 2006, The Android Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "webcoreglue"

#include <config.h>
#include <wtf/Platform.h>

#include "jni_utility.h"
#include "WebCoreResourceLoader.h"
#include "SkUtils.h"

#include "CString.h"
#include "ResourceError.h"
#include "ResourceHandle.h"
#include "ResourceHandleClient.h"
#include "ResourceHandleInternal.h"
#include "ResourceResponse.h"
#include "WebCoreJni.h"

#ifdef ANDROID_INSTRUMENT
#include "TimeCounter.h"
#endif

#include <utils/misc.h>
#include <JNIHelp.h>
#include <SkTypes.h>
#include <stdlib.h>

namespace android {
  
// ----------------------------------------------------------------------------

static struct resourceloader_t {
    jfieldID    mObject;
    jmethodID   mCancelMethodID;
    jmethodID   mDownloadFileMethodID;
    jmethodID   mWillLoadFromCacheMethodID;
} gResourceLoader;

// ----------------------------------------------------------------------------

#define GET_NATIVE_HANDLE(env, obj) ((WebCore::ResourceHandle*)env->GetIntField(obj, gResourceLoader.mObject))
#define SET_NATIVE_HANDLE(env, obj, handle) (env->SetIntField(obj, gResourceLoader.mObject, handle))

//-----------------------------------------------------------------------------
// ResourceLoadHandler

WebCoreResourceLoader::WebCoreResourceLoader(JNIEnv *env, jobject jLoadListener)
{
    mJLoader = env->NewGlobalRef(jLoadListener);
}

WebCoreResourceLoader::~WebCoreResourceLoader()
{
    JNIEnv* env = JSC::Bindings::getJNIEnv();
    SET_NATIVE_HANDLE(env, mJLoader, 0);
    env->DeleteGlobalRef(mJLoader);
    mJLoader = 0;
}

void WebCoreResourceLoader::cancel()
{
    JNIEnv* env = JSC::Bindings::getJNIEnv();
    env->CallVoidMethod(mJLoader, gResourceLoader.mCancelMethodID);
    checkException(env);
}

void WebCoreResourceLoader::downloadFile()
{
    JNIEnv* env = JSC::Bindings::getJNIEnv();
    env->CallVoidMethod(mJLoader, gResourceLoader.mDownloadFileMethodID);
    checkException(env);
}

/*
* This static method is called to check to see if a POST response is in
* the cache. This may be slow, but is only used during a navigation to
* a POST response.
*/
bool WebCoreResourceLoader::willLoadFromCache(const WebCore::KURL& url)
{
    JNIEnv* env = JSC::Bindings::getJNIEnv();
    WebCore::String urlStr = url.string();
    jstring jUrlStr = env->NewString(urlStr.characters(), urlStr.length());
    jclass resourceLoader = env->FindClass("android/webkit/LoadListener");
    bool val = env->CallStaticBooleanMethod(resourceLoader, 
            gResourceLoader.mWillLoadFromCacheMethodID, jUrlStr);
    checkException(env);
    env->DeleteLocalRef(jUrlStr);

    return val;
}

// ----------------------------------------------------------------------------
void WebCoreResourceLoader::SetResponseHeader(JNIEnv* env, jobject obj, jint nativeResponse, jstring key, jstring val)
{
#ifdef ANDROID_INSTRUMENT
    TimeCounterAuto counter(TimeCounter::ResourceTimeCounter);
#endif
    
    WebCore::ResourceResponse* response = (WebCore::ResourceResponse*)nativeResponse;
    LOG_ASSERT(response, "nativeSetResponseHeader must take a valid response pointer!");

    LOG_ASSERT(key, "How did a null value become a key?");
    if (val) {
        WebCore::String valStr = to_string(env, val);
        if (!valStr.isEmpty())
            response->setHTTPHeaderField(to_string(env, key), valStr);
    }
}

jint WebCoreResourceLoader::CreateResponse(JNIEnv* env, jobject obj, jstring url, jint statusCode,
                                                    jstring statusText, jstring mimeType, jlong expectedLength,
                                                    jstring encoding)
{
#ifdef ANDROID_INSTRUMENT
    TimeCounterAuto counter(TimeCounter::ResourceTimeCounter);
#endif
    LOG_ASSERT(url, "Must have a url in the response!");
    WebCore::KURL kurl(to_string(env, url));
    WebCore::String encodingStr;
    WebCore::String mimeTypeStr;
    if (mimeType) {
        mimeTypeStr = to_string(env, mimeType);
        LOGV("Response setMIMEType: %s", mimeTypeStr.latin1().data());
    }
    if (encoding) {
        encodingStr = to_string(env, encoding);
        LOGV("Response setTextEncodingName: %s", encodingStr.latin1().data());
    }
    WebCore::ResourceResponse* response = new WebCore::ResourceResponse(
            kurl, mimeTypeStr, (long long)expectedLength,
            encodingStr, WebCore::String());
    response->setHTTPStatusCode(statusCode);
    if (statusText) {
        WebCore::String status = to_string(env, statusText);
        response->setHTTPStatusText(status);
        LOGV("Response setStatusText: %s", status.latin1().data());
    }
    return (int)response;
}
   
void WebCoreResourceLoader::ReceivedResponse(JNIEnv* env, jobject obj, jint nativeResponse)
{
#ifdef ANDROID_INSTRUMENT
    TimeCounterAuto counter(TimeCounter::ResourceTimeCounter);
#endif
    WebCore::ResourceHandle* handle = GET_NATIVE_HANDLE(env, obj);
    LOG_ASSERT(handle, "nativeReceivedResponse must take a valid handle!");
    // ResourceLoader::didFail() can set handle to be NULL, we need to check
    if (!handle)
        return;

    WebCore::ResourceResponse* response = (WebCore::ResourceResponse*)nativeResponse;
    LOG_ASSERT(response, "nativeReceivedResponse must take a valid resource pointer!");
    handle->client()->didReceiveResponse(handle, *response);
    // As the client makes a copy of the response, delete it here.
    delete response;
}

void WebCoreResourceLoader::AddData(JNIEnv* env, jobject obj, jbyteArray dataArray, jint length)
{
#ifdef ANDROID_INSTRUMENT
    TimeCounterAuto counter(TimeCounter::ResourceTimeCounter);
#endif
    LOGV("webcore_resourceloader data(%d)", length);

    WebCore::ResourceHandle* handle = GET_NATIVE_HANDLE(env, obj);
    LOG_ASSERT(handle, "nativeAddData must take a valid handle!");
    // ResourceLoader::didFail() can set handle to be NULL, we need to check
    if (!handle)
        return;

    SkAutoMemoryUsageProbe  mup("android_webcore_resourceloader_nativeAddData");
    
    bool result = false;
    jbyte * data =  env->GetByteArrayElements(dataArray, NULL);

    LOG_ASSERT(handle->client(), "Why do we not have a client?");
    handle->client()->didReceiveData(handle, (const char *)data, length, length);
    env->ReleaseByteArrayElements(dataArray, data, JNI_ABORT);    
}

void WebCoreResourceLoader::Finished(JNIEnv* env, jobject obj)
{
#ifdef ANDROID_INSTRUMENT
    TimeCounterAuto counter(TimeCounter::ResourceTimeCounter);
#endif
    LOGV("webcore_resourceloader finished");
    WebCore::ResourceHandle* handle = GET_NATIVE_HANDLE(env, obj);
    LOG_ASSERT(handle, "nativeFinished must take a valid handle!");
    // ResourceLoader::didFail() can set handle to be NULL, we need to check
    if (!handle)
        return;

    LOG_ASSERT(handle->client(), "Why do we not have a client?");
    handle->client()->didFinishLoading(handle);
}

jstring WebCoreResourceLoader::RedirectedToUrl(JNIEnv* env, jobject obj,
        jstring baseUrl, jstring redirectTo, jint nativeResponse)
{
#ifdef ANDROID_INSTRUMENT
    TimeCounterAuto counter(TimeCounter::ResourceTimeCounter);
#endif
    LOGV("webcore_resourceloader redirectedToUrl");
    WebCore::ResourceHandle* handle = GET_NATIVE_HANDLE(env, obj);
    LOG_ASSERT(handle, "nativeRedirectedToUrl must take a valid handle!");
    // ResourceLoader::didFail() can set handle to be NULL, we need to check
    if (!handle)
        return NULL;

    LOG_ASSERT(handle->client(), "Why do we not have a client?");
    WebCore::ResourceRequest r = handle->request();
    WebCore::KURL url(WebCore::KURL(to_string(env, baseUrl)),
            to_string(env, redirectTo));
    r.setURL(url);
    if (r.httpMethod() == "POST") {
        r.setHTTPMethod("GET");
        r.clearHTTPReferrer();
        r.setHTTPBody(0);
        r.setHTTPContentType("");
    }
    WebCore::ResourceResponse* response = (WebCore::ResourceResponse*)nativeResponse;
    // If the url fails to resolve the relative path, return null.
    if (url.protocol().isEmpty()) {
        delete response;
        return NULL;
    }
    handle->client()->willSendRequest(handle, r, *response);
    delete response;
    WebCore::String s = url.string();
    return env->NewString((unsigned short*)s.characters(), s.length());
}

void WebCoreResourceLoader::Error(JNIEnv* env, jobject obj, jint id, jstring description,
        jstring failingUrl)
{
#ifdef ANDROID_INSTRUMENT
    TimeCounterAuto counter(TimeCounter::ResourceTimeCounter);
#endif
    LOGV("webcore_resourceloader error");
    WebCore::ResourceHandle* handle = GET_NATIVE_HANDLE(env, obj);
    LOG_ASSERT(handle, "nativeError must take a valid handle!");
    // ResourceLoader::didFail() can set handle to be NULL, we need to check
    if (!handle)
        return;

    handle->client()->didFail(handle, WebCore::ResourceError("", id,
                to_string(env, failingUrl), to_string(env, description)));
}

// ----------------------------------------------------------------------------

/*
 * JNI registration.
 */
static JNINativeMethod gResourceloaderMethods[] = {
    /* name, signature, funcPtr */
    { "nativeSetResponseHeader", "(ILjava/lang/String;Ljava/lang/String;)V",
        (void*) WebCoreResourceLoader::SetResponseHeader },
    { "nativeCreateResponse", "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;JLjava/lang/String;)I",
        (void*) WebCoreResourceLoader::CreateResponse },
    { "nativeReceivedResponse", "(I)V",
        (void*) WebCoreResourceLoader::ReceivedResponse },
    { "nativeAddData", "([BI)V",
        (void*) WebCoreResourceLoader::AddData },
    { "nativeFinished", "()V",
        (void*) WebCoreResourceLoader::Finished },
    { "nativeRedirectedToUrl", "(Ljava/lang/String;Ljava/lang/String;I)Ljava/lang/String;",
        (void*) WebCoreResourceLoader::RedirectedToUrl },
    { "nativeError", "(ILjava/lang/String;Ljava/lang/String;)V",
        (void*) WebCoreResourceLoader::Error }
};

int register_resource_loader(JNIEnv* env)
{
    jclass resourceLoader = env->FindClass("android/webkit/LoadListener");
    LOG_FATAL_IF(resourceLoader == NULL, 
        "Unable to find class android/webkit/LoadListener");
    
    gResourceLoader.mObject = 
        env->GetFieldID(resourceLoader, "mNativeLoader", "I");
    LOG_FATAL_IF(gResourceLoader.mObject == NULL, 
        "Unable to find android/webkit/LoadListener.mNativeLoader");
    
    gResourceLoader.mCancelMethodID = 
        env->GetMethodID(resourceLoader, "cancel", "()V");
    LOG_FATAL_IF(gResourceLoader.mCancelMethodID == NULL, 
        "Could not find method cancel on LoadListener");
    
    gResourceLoader.mDownloadFileMethodID = 
        env->GetMethodID(resourceLoader, "downloadFile", "()V");
    LOG_FATAL_IF(gResourceLoader.mDownloadFileMethodID == NULL, 
        "Could not find method downloadFile on LoadListener");

    gResourceLoader.mWillLoadFromCacheMethodID = 
        env->GetStaticMethodID(resourceLoader, "willLoadFromCache", "(Ljava/lang/String;)Z");
    LOG_FATAL_IF(gResourceLoader.mWillLoadFromCacheMethodID == NULL, 
        "Could not find static method willLoadFromCache on LoadListener");

    return jniRegisterNativeMethods(env, "android/webkit/LoadListener", 
                     gResourceloaderMethods, NELEM(gResourceloaderMethods));
}

} /* namespace android */
