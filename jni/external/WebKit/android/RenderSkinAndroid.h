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

#ifndef RenderSkinAndroid_h
#define RenderSkinAndroid_h

#include "PlatformString.h"

namespace android {
    class AssetManager;
}

class SkBitmap;

namespace WebCore {
class Node;
class PlatformGraphicsContext;

/*  RenderSkinAndroid is the base class for all RenderSkins.  Form elements each have a
 *  subclass for drawing themselves.
 */
class RenderSkinAndroid
{
public:
    RenderSkinAndroid();
    virtual ~RenderSkinAndroid() {}
    
    enum State {
        kDisabled,
        kNormal,
        kFocused,
        kPressed,
    
        kNumStates
    };

    /**
     * Initialize the Android skinning system. The AssetManager may be used to find resources used
     * in rendering.
     */
    static void Init(android::AssetManager*, String drawableDirectory);
    
    /* DecodeBitmap determines which file to use, with the given fileName of the form 
     * "images/bitmap.png", and uses the asset manager to select the exact one.  It
     * returns true if it successfully decoded the bitmap, false otherwise.
     */
    static bool DecodeBitmap(android::AssetManager* am, const char* fileName, SkBitmap* bitmap);

    /*  draw() tells the skin to draw itself, and returns true if the skin needs
     *  a redraw to animations, false otherwise
     */
    virtual bool draw(PlatformGraphicsContext*) { return false; }
    
    /*  notifyState() checks to see if the element is checked, focused, and enabled
     *  it must be implemented in the subclass
     */
    virtual void notifyState(Node* element) { }
    
    /*  setDim() tells the skin its width and height
     */
    virtual void setDim(int width, int height) { m_width = width; m_height = height; }

protected:
    int                     m_height;
    int                     m_width;

};

} // WebCore

#endif

