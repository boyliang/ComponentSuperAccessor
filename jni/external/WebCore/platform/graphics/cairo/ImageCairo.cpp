/*
 * Copyright (C) 2004, 2005, 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
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

#include "config.h"
#include "BitmapImage.h"

#if PLATFORM(CAIRO)

#include "Color.h"
#include "FloatRect.h"
#include "GraphicsContext.h"
#include "ImageBuffer.h"
#include "ImageObserver.h"
#include "TransformationMatrix.h"
#include <cairo.h>
#include <math.h>
#include <wtf/OwnPtr.h>

namespace WebCore {

bool FrameData::clear(bool clearMetadata)
{
    if (clearMetadata)
        m_haveMetadata = false;

    if (m_frame) {
        cairo_surface_destroy(m_frame);
        m_frame = 0;
        return true;
    }
    return false;
}

BitmapImage::BitmapImage(cairo_surface_t* surface, ImageObserver* observer)
    : Image(observer)
    , m_currentFrame(0)
    , m_frames(0)
    , m_frameTimer(0)
    , m_repetitionCount(cAnimationNone)
    , m_repetitionCountStatus(Unknown)
    , m_repetitionsComplete(0)
    , m_isSolidColor(false)
    , m_checkedForSolidColor(false)
    , m_animationFinished(true)
    , m_allDataReceived(true)
    , m_haveSize(true)
    , m_sizeAvailable(true)
    , m_decodedSize(0)
    , m_haveFrameCount(true)
    , m_frameCount(1)
{
    initPlatformData();

    // TODO: check to be sure this is an image surface

    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);
    m_decodedSize = width * height * 4;
    m_size = IntSize(width, height);

    m_frames.grow(1);
    m_frames[0].m_frame = surface;
    m_frames[0].m_hasAlpha = cairo_surface_get_content(surface) != CAIRO_CONTENT_COLOR;
    m_frames[0].m_haveMetadata = true;
    checkForSolidColor();
}

void BitmapImage::draw(GraphicsContext* context, const FloatRect& dst, const FloatRect& src, CompositeOperator op)
{
    FloatRect srcRect(src);
    FloatRect dstRect(dst);

    if (dstRect.width() == 0.0f || dstRect.height() == 0.0f ||
        srcRect.width() == 0.0f || srcRect.height() == 0.0f)
        return;

    startAnimation();

    cairo_surface_t* image = frameAtIndex(m_currentFrame);
    if (!image) // If it's too early we won't have an image yet.
        return;

    if (mayFillWithSolidColor()) {
        fillWithSolidColor(context, dstRect, solidColor(), op);
        return;
    }

    IntSize selfSize = size();

    cairo_t* cr = context->platformContext();
    context->save();

    // Set the compositing operation.
    if (op == CompositeSourceOver && !frameHasAlphaAtIndex(m_currentFrame))
        context->setCompositeOperation(CompositeCopy);
    else
        context->setCompositeOperation(op);

    // If we're drawing a sub portion of the image or scaling then create
    // a pattern transformation on the image and draw the transformed pattern.
    // Test using example site at http://www.meyerweb.com/eric/css/edge/complexspiral/demo.html
    cairo_pattern_t* pattern = cairo_pattern_create_for_surface(image);

    // To avoid the unwanted gradient effect (#14017) we use
    // CAIRO_FILTER_NEAREST now, but the real fix will be to have
    // CAIRO_EXTEND_PAD implemented for surfaces in Cairo allowing us to still
    // use bilinear filtering
    cairo_pattern_set_filter(pattern, CAIRO_FILTER_NEAREST);

    float scaleX = srcRect.width() / dstRect.width();
    float scaleY = srcRect.height() / dstRect.height();
    cairo_matrix_t matrix = { scaleX, 0, 0, scaleY, srcRect.x(), srcRect.y() };
    cairo_pattern_set_matrix(pattern, &matrix);

    // Draw the image.
    cairo_translate(cr, dstRect.x(), dstRect.y());
    cairo_set_source(cr, pattern);
    cairo_pattern_destroy(pattern);
    cairo_rectangle(cr, 0, 0, dstRect.width(), dstRect.height());
    cairo_clip(cr);
    cairo_paint_with_alpha(cr, context->getAlpha());

    context->restore();

    if (imageObserver())
        imageObserver()->didDraw(this);
}

void Image::drawPattern(GraphicsContext* context, const FloatRect& tileRect, const TransformationMatrix& patternTransform,
                        const FloatPoint& phase, CompositeOperator op, const FloatRect& destRect)
{
    cairo_surface_t* image = nativeImageForCurrentFrame();
    if (!image) // If it's too early we won't have an image yet.
        return;

    // Avoid NaN
    if (!isfinite(phase.x()) || !isfinite(phase.y()))
       return;

    cairo_t* cr = context->platformContext();
    context->save();

    IntRect imageSize = enclosingIntRect(tileRect);
    OwnPtr<ImageBuffer> imageSurface = ImageBuffer::create(imageSize.size());

    if (!imageSurface)
        return;

    if (tileRect.size() != size()) {
        cairo_t* clippedImageContext = imageSurface->context()->platformContext();
        cairo_set_source_surface(clippedImageContext, image, -tileRect.x(), -tileRect.y());
        cairo_paint(clippedImageContext);
        image = imageSurface->image()->nativeImageForCurrentFrame();
    }

    cairo_pattern_t* pattern = cairo_pattern_create_for_surface(image);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);

    // Workaround to avoid the unwanted gradient effect (#14017)
    cairo_pattern_set_filter(pattern, CAIRO_FILTER_NEAREST);

    cairo_matrix_t pattern_matrix = cairo_matrix_t(patternTransform);
    cairo_matrix_t phase_matrix = {1, 0, 0, 1, phase.x() + tileRect.x() * patternTransform.a(), phase.y() + tileRect.y() * patternTransform.d()};
    cairo_matrix_t combined;
    cairo_matrix_multiply(&combined, &pattern_matrix, &phase_matrix);
    cairo_matrix_invert(&combined);
    cairo_pattern_set_matrix(pattern, &combined);

    context->setCompositeOperation(op);
    cairo_set_source(cr, pattern);
    cairo_pattern_destroy(pattern);
    cairo_rectangle(cr, destRect.x(), destRect.y(), destRect.width(), destRect.height());
    cairo_fill(cr);

    context->restore();

    if (imageObserver())
        imageObserver()->didDraw(this);
}

void BitmapImage::checkForSolidColor()
{
    m_isSolidColor = false;
    m_checkedForSolidColor = true;

    if (frameCount() > 1)
        return;

    cairo_surface_t* frameSurface = frameAtIndex(0);
    if (!frameSurface)
        return;

    ASSERT(cairo_surface_get_type(frameSurface) == CAIRO_SURFACE_TYPE_IMAGE);

    int width = cairo_image_surface_get_width(frameSurface);
    int height = cairo_image_surface_get_height(frameSurface);

    if (width != 1 || height != 1)
        return;

    unsigned* pixelColor = reinterpret_cast<unsigned*>(cairo_image_surface_get_data(frameSurface));
    m_solidColor = colorFromPremultipliedARGB(*pixelColor);

    m_isSolidColor = true;
}

}

#endif // PLATFORM(CAIRO)
