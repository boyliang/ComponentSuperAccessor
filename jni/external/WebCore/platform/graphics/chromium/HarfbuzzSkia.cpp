/*
 * Copyright (c) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "Font.h"
#include "FontPlatformData.h"
#include "wtf/OwnArrayPtr.h"

#include "SkFontHost.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkRect.h"

extern "C" {
#include "harfbuzz-shaper.h"
#include "harfbuzz-unicode.h"
}

// This file implements the callbacks which Harfbuzz requires by using Skia
// calls. See the Harfbuzz source for references about what these callbacks do.

namespace WebCore {

static HB_Fixed SkiaScalarToHarfbuzzFixed(SkScalar value)
{
    // HB_Fixed is a 26.6 fixed point format.
    return value * 64;
}

static HB_Bool stringToGlyphs(HB_Font hbFont, const HB_UChar16* characters, hb_uint32 length, HB_Glyph* glyphs, hb_uint32* glyphsSize, HB_Bool isRTL)
{
    FontPlatformData* font = reinterpret_cast<FontPlatformData*>(hbFont->userData);
    SkPaint paint;

    font->setupPaint(&paint);
    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
    int numGlyphs = paint.textToGlyphs(characters, length * sizeof(uint16_t), reinterpret_cast<uint16_t*>(glyphs));

    // HB_Glyph is 32-bit, but Skia outputs only 16-bit numbers. So our
    // |glyphs| array needs to be converted.
    // Additionally, if the CSS white-space property is inhibiting line
    // breaking, we might find end-of-line charactors rendered via the complex
    // text path. Fonts don't provide glyphs for these code points so, if we
    // find one, we simulate the space glyph being interposed in this case.
    // Because the input is variable-length per code point, we walk the input
    // in step with the output.
    // FIXME: it seems that this logic is duplicated in CoreTextController and UniscribeController
    ssize_t indexOfNextCodePoint = 0;
    uint16_t spaceGlyphNumber = 0;
    for (int i = numGlyphs - 1; i >= 0; --i) {
        const uint32_t currentCodePoint = utf16_to_code_point(characters, length, &indexOfNextCodePoint);

        uint16_t value;
        // We use a memcpy to avoid breaking strict aliasing rules.
        memcpy(&value, reinterpret_cast<char*>(glyphs) + sizeof(uint16_t) * i, sizeof(uint16_t));

        if (!value && Font::treatAsSpace(currentCodePoint)) {
            static const uint16_t spaceUTF16 = ' ';
            if (!spaceGlyphNumber)
                paint.textToGlyphs(&spaceUTF16, sizeof(spaceUTF16), &spaceGlyphNumber);
            value = spaceGlyphNumber;
        }

        glyphs[i] = value;
    }

    *glyphsSize = numGlyphs;
    return 1;
}

static void glyphsToAdvances(HB_Font hbFont, const HB_Glyph* glyphs, hb_uint32 numGlyphs, HB_Fixed* advances, int flags)
{
    FontPlatformData* font = reinterpret_cast<FontPlatformData*>(hbFont->userData);
    SkPaint paint;

    font->setupPaint(&paint);
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    OwnArrayPtr<uint16_t> glyphs16(new uint16_t[numGlyphs]);
    if (!glyphs16.get())
        return;
    for (unsigned i = 0; i < numGlyphs; ++i)
        glyphs16[i] = glyphs[i];
    paint.getTextWidths(glyphs16.get(), numGlyphs * sizeof(uint16_t), reinterpret_cast<SkScalar*>(advances));

    // The |advances| values which Skia outputs are SkScalars, which are floats
    // in Chromium. However, Harfbuzz wants them in 26.6 fixed point format.
    // These two formats are both 32-bits long.
    for (unsigned i = 0; i < numGlyphs; ++i) {
        float value;
        // We use a memcpy to avoid breaking strict aliasing rules.
        memcpy(&value, reinterpret_cast<char*>(advances) + sizeof(float) * i, sizeof(float));
        advances[i] = SkiaScalarToHarfbuzzFixed(value);
    }
}

static HB_Bool canRender(HB_Font hbFont, const HB_UChar16* characters, hb_uint32 length)
{
    FontPlatformData* font = reinterpret_cast<FontPlatformData*>(hbFont->userData);
    SkPaint paint;

    font->setupPaint(&paint);
    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);

    OwnArrayPtr<uint16_t> glyphs16(new uint16_t[length]);
    if (!glyphs16.get())
        return 0;
    int numGlyphs = paint.textToGlyphs(characters, length * sizeof(uint16_t), glyphs16.get());

    bool canRender = true;
    for (int i = 0; i < numGlyphs; ++i) {
        if (!glyphs16[i]) {
            canRender = false;
            break;
        }
    }

    return canRender;
}

static HB_Error getOutlinePoint(HB_Font hbFont, HB_Glyph glyph, int flags, hb_uint32 point, HB_Fixed* xPos, HB_Fixed* yPos, hb_uint32* resultingNumPoints)
{
    FontPlatformData* font = reinterpret_cast<FontPlatformData*>(hbFont->userData);
    SkPaint paint;

    if (flags & HB_ShaperFlag_UseDesignMetrics)
        return HB_Err_Invalid_Argument;  // This is requesting pre-hinted positions. We can't support this.

    font->setupPaint(&paint);
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    uint16_t glyph16 = glyph;
    SkPath path;
    paint.getTextPath(&glyph16, sizeof(glyph16), 0, 0, &path);
    int numPoints = path.getPoints(NULL, 0);
    if (point >= numPoints)
        return HB_Err_Invalid_SubTable;
    SkPoint* points = reinterpret_cast<SkPoint*>(fastMalloc(sizeof(SkPoint) * (point + 1)));
    if (!points)
        return HB_Err_Invalid_SubTable;
    // Skia does let us get a single point from the path.
    path.getPoints(points, point + 1);
    *xPos = SkiaScalarToHarfbuzzFixed(points[point].fX);
    *yPos = SkiaScalarToHarfbuzzFixed(points[point].fY);
    *resultingNumPoints = numPoints;
    fastFree(points);

    return HB_Err_Ok;
}

static void getGlyphMetrics(HB_Font hbFont, HB_Glyph glyph, HB_GlyphMetrics* metrics)
{
    FontPlatformData* font = reinterpret_cast<FontPlatformData*>(hbFont->userData);
    SkPaint paint;

    font->setupPaint(&paint);
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    uint16_t glyph16 = glyph;
    SkScalar width;
    SkRect bounds;
    paint.getTextWidths(&glyph16, sizeof(glyph16), &width, &bounds);

    metrics->x = SkiaScalarToHarfbuzzFixed(width);
    // We can't actually get the |y| correct because Skia doesn't export
    // the vertical advance. However, nor we do ever render vertical text at
    // the moment so it's unimportant.
    metrics->y = 0;
    metrics->width = SkiaScalarToHarfbuzzFixed(bounds.width());
    metrics->height = SkiaScalarToHarfbuzzFixed(bounds.height());
    metrics->xOffset = SkiaScalarToHarfbuzzFixed(bounds.fLeft);
    metrics->yOffset = SkiaScalarToHarfbuzzFixed(bounds.fTop);
}

static HB_Fixed getFontMetric(HB_Font hbFont, HB_FontMetric metric)
{
    FontPlatformData* font = reinterpret_cast<FontPlatformData*>(hbFont->userData);
    SkPaint paint;

    font->setupPaint(&paint);
    SkPaint::FontMetrics skiaMetrics;
    paint.getFontMetrics(&skiaMetrics);

    switch (metric) {
    case HB_FontAscent:
        return SkiaScalarToHarfbuzzFixed(-skiaMetrics.fAscent);
    // We don't support getting the rest of the metrics and Harfbuzz doesn't seem to need them.
    default:
        return 0;
    }
}

HB_FontClass harfbuzzSkiaClass = {
    stringToGlyphs,
    glyphsToAdvances,
    canRender,
    getOutlinePoint,
    getGlyphMetrics,
    getFontMetric,
};

HB_Error harfbuzzSkiaGetTable(void* voidface, const HB_Tag tag, HB_Byte* buffer, HB_UInt* len)
{
    FontPlatformData* font = reinterpret_cast<FontPlatformData*>(voidface);

    const size_t tableSize = SkFontHost::GetTableSize(font->uniqueID(), tag);
    if (!tableSize)
        return HB_Err_Invalid_Argument;
    // If Harfbuzz specified a NULL buffer then it's asking for the size of the table.
    if (!buffer) {
        *len = tableSize;
        return HB_Err_Ok;
    }

    if (*len < tableSize)
        return HB_Err_Invalid_Argument;
    SkFontHost::GetTableData(font->uniqueID(), tag, 0, tableSize, buffer);
    return HB_Err_Ok;
}

}  // namespace WebCore
