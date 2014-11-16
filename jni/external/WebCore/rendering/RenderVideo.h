/*
 * Copyright (C) 2007, 2009 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
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

#ifndef RenderVideo_h
#define RenderVideo_h

#if ENABLE(VIDEO)

#include "RenderMedia.h"

namespace WebCore {
    
class HTMLMediaElement;
#if USE(ACCELERATED_COMPOSITING)
class GraphicsLayer;
#endif

class RenderVideo : public RenderMedia {
public:
    RenderVideo(HTMLMediaElement*);
    virtual ~RenderVideo();

    void videoSizeChanged();
    IntRect videoBox() const;
    
#if USE(ACCELERATED_COMPOSITING)
    bool supportsAcceleratedRendering() const;
    void acceleratedRenderingStateChanged();
    GraphicsLayer* videoGraphicsLayer() const;
#endif

private:
    virtual void updateFromElement();

    virtual void intrinsicSizeChanged() { videoSizeChanged(); }

    virtual const char* renderName() const { return "RenderVideo"; }

    virtual bool requiresLayer() const { return true; }
    virtual bool isVideo() const { return true; }

    virtual void paintReplaced(PaintInfo&, int tx, int ty);

    virtual void layout();

    virtual int calcReplacedWidth(bool includeMaxWidth = true) const;
    virtual int calcReplacedHeight() const;

    virtual void calcPrefWidths();
    
    int calcAspectRatioWidth() const;
    int calcAspectRatioHeight() const;

    bool isWidthSpecified() const;
    bool isHeightSpecified() const;
    
    void updatePlayer();
};

inline RenderVideo* toRenderVideo(RenderObject* object)
{
    ASSERT(!object || object->isVideo());
    return static_cast<RenderVideo*>(object);
}

// This will catch anyone doing an unnecessary cast.
void toRenderVideo(const RenderVideo*);

} // namespace WebCore

#endif
#endif // RenderVideo_h
