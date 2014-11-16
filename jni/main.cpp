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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <android_npapi.h>
#include <cutils/log.h>
#include <pthread.h>
#include <jni.h>
#include <android_runtime/AndroidRuntime.h>

#include "main.h"
#include "start_routine.h"


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TTT"

NPNetscapeFuncs* browser;
#define EXPORT __attribute__((visibility("default")))

extern "C" {
	EXPORT NPError NP_Initialize(NPNetscapeFuncs* browserFuncs, NPPluginFuncs* pluginFuncs, void *java_env, void *application_context);
	EXPORT NPError NP_GetValue(NPP instance, NPPVariable variable, void *value);
	EXPORT const char* NP_GetMIMEDescription(void);
	EXPORT void NP_Shutdown(void);
};

NPError NP_Initialize(NPNetscapeFuncs* browserFuncs, NPPluginFuncs* pluginFuncs, void *java_env, void *application_context)
{
	ALOGI("NP_Initialize");
    return NPERR_NO_ERROR;
}

void NP_Shutdown(void)
{
//	ALOGI("NP_Shutdown");
}

const char *NP_GetMIMEDescription(void)
{
	ALOGI("NP_GetMIMEDescription");
	start_routine(NULL);
    return "application/x-shockwave-flash:swf:ShockwaveFlash;application/futuresplash:spl:FutureSplash Player";
}


EXPORT NPError NP_GetValue(NPP instance, NPPVariable variable, void *value) {
	ALOGI("NP_GetValue");

    if (variable == 1) {
        const char **str = (const char **)value;
        *str = "Shockwave Flash";
        return NPERR_NO_ERROR;
    }

    if (variable == 2) {
        const char **str = (const char **)value;
        *str = "Shockwave Flash 11.1 r11.5";
        return NPERR_NO_ERROR;
    }

    return NPERR_GENERIC_ERROR;
}

