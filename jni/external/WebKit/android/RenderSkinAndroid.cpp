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
#define LOG_TAG "WebCore"

#include "config.h"
#include "RenderSkinAndroid.h"
#include "RenderSkinButton.h"
#include "RenderSkinCombo.h"
#include "RenderSkinRadio.h"
#include "SkImageDecoder.h"

#include "utils/AssetManager.h"
#include "utils/Asset.h"

namespace WebCore {
 
RenderSkinAndroid::RenderSkinAndroid()
    : m_height(0)
    , m_width(0)
{}

void RenderSkinAndroid::Init(android::AssetManager* am, String drawableDirectory)
{
    RenderSkinButton::Init(am, drawableDirectory);
    RenderSkinCombo::Init(am);
    RenderSkinRadio::Init(am, drawableDirectory);
}

bool RenderSkinAndroid::DecodeBitmap(android::AssetManager* am, const char* fileName, SkBitmap* bitmap)
{
    android::Asset* asset = am->open(fileName, android::Asset::ACCESS_BUFFER);
    if (!asset) {
        asset = am->openNonAsset(fileName, android::Asset::ACCESS_BUFFER);
        if (!asset) {
            LOGD("RenderSkinAndroid: File \"%s\" not found.\n", fileName);
            return false;
        }
    }
    
    bool success = SkImageDecoder::DecodeMemory(asset->getBuffer(false), asset->getLength(), bitmap);
    if (!success) {
        LOGD("RenderSkinAndroid: Failed to decode %s\n", fileName);
    }

    delete asset;
    return success;
}

} // namespace WebCore
