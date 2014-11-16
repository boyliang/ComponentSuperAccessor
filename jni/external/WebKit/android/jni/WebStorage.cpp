/*
 * Copyright 2009, The Android Open Source Project
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

#include "config.h"

#if ENABLE(DATABASE)

#include <JNIHelp.h>

#include <WebCore/loader/appcache/ApplicationCacheStorage.h>
#include <WebCore/page/SecurityOrigin.h>
#include <WebCore/storage/DatabaseTracker.h>

#include "JavaSharedClient.h"
#include "jni_utility.h"
#include "KURL.h"
#include "WebCoreJni.h"

namespace android {

static jobject GetOrigins(JNIEnv* env, jobject obj)
{
    Vector<RefPtr<WebCore::SecurityOrigin> > coreOrigins;
    WebCore::DatabaseTracker::tracker().origins(coreOrigins);
    Vector<WebCore::KURL> manifestUrls;
    if (WebCore::cacheStorage().manifestURLs(&manifestUrls)) {
        int size = manifestUrls.size();
        for (int i = 0; i < size; ++i) {
            RefPtr<WebCore::SecurityOrigin> manifestOrigin = WebCore::SecurityOrigin::create(manifestUrls[i]);
            if (manifestOrigin.get() == 0)
                continue;
            coreOrigins.append(manifestOrigin);
        }
    }

    jclass setClass = env->FindClass("java/util/HashSet");
    jmethodID cid = env->GetMethodID(setClass, "<init>", "()V");
    jmethodID mid = env->GetMethodID(setClass, "add", "(Ljava/lang/Object;)Z");
    jobject set = env->NewObject(setClass, cid);

    for (unsigned i = 0; i < coreOrigins.size(); ++i) {
        WebCore::SecurityOrigin* origin = coreOrigins[i].get();
        WebCore::String url = origin->toString();
        jstring jUrl = env->NewString(url.characters(), url.length());
        env->CallBooleanMethod(set, mid, jUrl);
        env->DeleteLocalRef(jUrl);
    }

    return set;
}

static unsigned long long GetQuotaForOrigin(JNIEnv* env, jobject obj, jstring origin)
{
    WebCore::String originStr = to_string(env, origin);
    RefPtr<WebCore::SecurityOrigin> securityOrigin = WebCore::SecurityOrigin::createFromString(originStr);
    unsigned long long quota = WebCore::DatabaseTracker::tracker().quotaForOrigin(securityOrigin.get());
    return quota;
}

static unsigned long long GetUsageForOrigin(JNIEnv* env, jobject obj, jstring origin)
{
    WebCore::String originStr = to_string(env, origin);
    RefPtr<WebCore::SecurityOrigin> securityOrigin = WebCore::SecurityOrigin::createFromString(originStr);
    unsigned long long usage = WebCore::DatabaseTracker::tracker().usageForOrigin(securityOrigin.get());
    Vector<WebCore::KURL> manifestUrls;
    if (!WebCore::cacheStorage().manifestURLs(&manifestUrls))
        return usage;
    int size = manifestUrls.size();
    for (int i = 0; i < size; ++i) {
        RefPtr<WebCore::SecurityOrigin> manifestOrigin = WebCore::SecurityOrigin::create(manifestUrls[i]);
        if (manifestOrigin.get() == 0)
            continue;
        if (manifestOrigin->isSameSchemeHostPort(securityOrigin.get())) {
            int64_t size = 0;
            WebCore::cacheStorage().cacheGroupSize(manifestUrls[i].string(), &size);
            usage += size;
        }
    }
    return usage;
}

static void SetQuotaForOrigin(JNIEnv* env, jobject obj, jstring origin, unsigned long long quota)
{
    WebCore::String originStr = to_string(env, origin);
    RefPtr<WebCore::SecurityOrigin> securityOrigin = WebCore::SecurityOrigin::createFromString(originStr);
    WebCore::DatabaseTracker::tracker().setQuota(securityOrigin.get(), quota);
}

static void DeleteOrigin(JNIEnv* env, jobject obj, jstring origin)
{
    WebCore::String originStr = to_string(env, origin);
    RefPtr<WebCore::SecurityOrigin> securityOrigin = WebCore::SecurityOrigin::createFromString(originStr);
    WebCore::DatabaseTracker::tracker().deleteOrigin(securityOrigin.get());

    Vector<WebCore::KURL> manifestUrls;
    if (!WebCore::cacheStorage().manifestURLs(&manifestUrls))
        return;
    int size = manifestUrls.size();
    for (int i = 0; i < size; ++i) {
        RefPtr<WebCore::SecurityOrigin> manifestOrigin = WebCore::SecurityOrigin::create(manifestUrls[i]);
        if (manifestOrigin.get() == 0)
            continue;
        if (manifestOrigin->isSameSchemeHostPort(securityOrigin.get()))
            WebCore::cacheStorage().deleteCacheGroup(manifestUrls[i]);
    }
}

static void DeleteAllData(JNIEnv* env, jobject obj)
{
    WebCore::DatabaseTracker::tracker().deleteAllDatabases();
    WebCore::cacheStorage().empty();
}

/*
 * JNI registration
 */
static JNINativeMethod gWebStorageMethods[] = {
    { "nativeGetOrigins", "()Ljava/util/Set;",
        (void*) GetOrigins },
    { "nativeGetUsageForOrigin", "(Ljava/lang/String;)J",
        (void*) GetUsageForOrigin },
    { "nativeGetQuotaForOrigin", "(Ljava/lang/String;)J",
        (void*) GetQuotaForOrigin },
    { "nativeSetQuotaForOrigin", "(Ljava/lang/String;J)V",
        (void*) SetQuotaForOrigin },
    { "nativeDeleteOrigin", "(Ljava/lang/String;)V",
        (void*) DeleteOrigin },
    { "nativeDeleteAllData", "()V",
        (void*) DeleteAllData }
};

int register_webstorage(JNIEnv* env)
{
    jclass webStorage = env->FindClass("android/webkit/WebStorage");
    LOG_ASSERT(webStorage, "Unable to find class android.webkit.WebStorage");

    return jniRegisterNativeMethods(env, "android/webkit/WebStorage",
            gWebStorageMethods, NELEM(gWebStorageMethods));
}

}

#endif //ENABLE(DATABASE)

