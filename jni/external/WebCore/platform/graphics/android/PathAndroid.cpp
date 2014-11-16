/*
 * Copyright 2007, The Android Open Source Project
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
#include "Path.h"
#include "FloatRect.h"
#include "GraphicsContext.h"
#include "ImageBuffer.h"
#include "StrokeStyleApplier.h"
#include "TransformationMatrix.h"

#include "SkPaint.h"
#include "SkPath.h"
#include "SkRegion.h"

#include "android_graphics.h"

namespace WebCore {

Path::Path()
{
    m_path = new SkPath;
//    m_path->setFlags(SkPath::kWinding_FillType);
}

Path::Path(const Path& other)
{
    m_path = new SkPath(*other.m_path);
}

Path::~Path()
{
    delete m_path;
}

Path& Path::operator=(const Path& other)
{
    *m_path = *other.m_path;
    return *this;
}

bool Path::isEmpty() const
{
    return m_path->isEmpty();
}

bool Path::hasCurrentPoint() const
{
    // webkit wants to know if we have any points, including any moveTos.
    // Skia's empty() will return true if it has just a moveTo, so we need to
    // call getPoints(NULL), which returns the number of points,
    // including moveTo.
    return m_path->getPoints(0, 0) > 0;
}

bool Path::contains(const FloatPoint& point, WindRule rule) const
{
    SkRegion    rgn, clip;
    
    int x = (int)floorf(point.x());
    int y = (int)floorf(point.y());
    clip.setRect(x, y, x + 1, y + 1);
    
    SkPath::FillType ft = m_path->getFillType();    // save
    m_path->setFillType(rule == RULE_NONZERO ? SkPath::kWinding_FillType : SkPath::kEvenOdd_FillType);

    bool contains = rgn.setPath(*m_path, clip);
    
    m_path->setFillType(ft);    // restore
    return contains;
}

void Path::translate(const FloatSize& size)
{
    m_path->offset(SkFloatToScalar(size.width()), SkFloatToScalar(size.height()));
}

FloatRect Path::boundingRect() const
{
    const SkRect& r = m_path->getBounds();
    return FloatRect(   SkScalarToFloat(r.fLeft),
                        SkScalarToFloat(r.fTop),
                        SkScalarToFloat(r.width()),
                        SkScalarToFloat(r.height()));
}

void Path::moveTo(const FloatPoint& point)
{
    m_path->moveTo(SkFloatToScalar(point.x()), SkFloatToScalar(point.y()));
}

void Path::addLineTo(const FloatPoint& p)
{
    m_path->lineTo(SkFloatToScalar(p.x()), SkFloatToScalar(p.y()));
}

void Path::addQuadCurveTo(const FloatPoint& cp, const FloatPoint& ep)
{
    m_path->quadTo( SkFloatToScalar(cp.x()), SkFloatToScalar(cp.y()),
                    SkFloatToScalar(ep.x()), SkFloatToScalar(ep.y()));
}

void Path::addBezierCurveTo(const FloatPoint& p1, const FloatPoint& p2, const FloatPoint& ep)
{
    m_path->cubicTo(SkFloatToScalar(p1.x()), SkFloatToScalar(p1.y()),
                    SkFloatToScalar(p2.x()), SkFloatToScalar(p2.y()),
                    SkFloatToScalar(ep.x()), SkFloatToScalar(ep.y()));
}

void Path::addArcTo(const FloatPoint& p1, const FloatPoint& p2, float radius)
{
    m_path->arcTo(SkFloatToScalar(p1.x()), SkFloatToScalar(p1.y()),
                  SkFloatToScalar(p2.x()), SkFloatToScalar(p2.y()),
                  SkFloatToScalar(radius));
}

void Path::closeSubpath()
{
    m_path->close();
}

static const float gPI  = 3.14159265f;
static const float g2PI = 6.28318531f;
static const float g180OverPI = 57.29577951308f;

static float fast_mod(float angle, float max) {
    if (angle >= max || angle <= -max) {
        angle = fmodf(angle, max); 
    }
    return angle;
}

void Path::addArc(const FloatPoint& p, float r, float sa, float ea,
                  bool clockwise) {
    SkScalar    cx = SkFloatToScalar(p.x());
    SkScalar    cy = SkFloatToScalar(p.y());
    SkScalar    radius = SkFloatToScalar(r);

    SkRect  oval;
    oval.set(cx - radius, cy - radius, cx + radius, cy + radius);
    
    float sweep = ea - sa;
    bool prependOval = false;

    /*  Note if clockwise and the sign of the sweep disagree. This particular
        logic was deduced from http://canvex.lazyilluminati.com/misc/arc.html
    */
    if (clockwise && (sweep > 0 || sweep < -g2PI)) {
        sweep = fmodf(sweep, g2PI) - g2PI;
    } else if (!clockwise && (sweep < 0 || sweep > g2PI)) {
        sweep = fmodf(sweep, g2PI) + g2PI;
    }
    
    // If the abs(sweep) >= 2PI, then we need to add a circle before we call
    // arcTo, since it treats the sweep mod 2PI. We don't have a prepend call,
    // so we just remember this, and at the end create a new path with an oval
    // and our current path, and then swap then.
    //
    if (sweep >= g2PI || sweep <= -g2PI) {
        prependOval = true;
//        SkDebugf("addArc sa=%g ea=%g cw=%d sweep %g treat as circle\n", sa, ea, clockwise, sweep);

        // now reduce sweep to just the amount we need, so that the current
        // point is left where the caller expects it.
        sweep = fmodf(sweep, g2PI);
    }

    sa = fast_mod(sa, g2PI);
    SkScalar startDegrees = SkFloatToScalar(sa * g180OverPI);
    SkScalar sweepDegrees = SkFloatToScalar(sweep * g180OverPI);

//    SkDebugf("addArc sa=%g ea=%g cw=%d sweep=%g ssweep=%g\n", sa, ea, clockwise, sweep, SkScalarToFloat(sweepDegrees));
    m_path->arcTo(oval, startDegrees, sweepDegrees, false);
    
    if (prependOval) {
        SkPath tmp;
        tmp.addOval(oval);
        tmp.addPath(*m_path);
        m_path->swap(tmp);
    }
}

void Path::addRect(const FloatRect& rect)
{
    m_path->addRect(rect);
}

void Path::addEllipse(const FloatRect& rect)
{
    m_path->addOval(rect);
}

void Path::clear()
{
    m_path->reset();
}

static FloatPoint* setfpts(FloatPoint dst[], const SkPoint src[], int count)
{
    for (int i = 0; i < count; i++)
    {
        dst[i].setX(SkScalarToFloat(src[i].fX));
        dst[i].setY(SkScalarToFloat(src[i].fY));
    }
    return dst;
}

void Path::apply(void* info, PathApplierFunction function) const
{
    SkPath::Iter    iter(*m_path, false);
    SkPoint         pts[4];
    
    PathElement     elem;
    FloatPoint      fpts[3];

    for (;;)
    {
        switch (iter.next(pts)) {
        case SkPath::kMove_Verb:
            elem.type = PathElementMoveToPoint;
            elem.points = setfpts(fpts, &pts[0], 1);
            break;
        case SkPath::kLine_Verb:
            elem.type = PathElementAddLineToPoint;
            elem.points = setfpts(fpts, &pts[1], 1);
            break;
        case SkPath::kQuad_Verb:
            elem.type = PathElementAddQuadCurveToPoint;
            elem.points = setfpts(fpts, &pts[1], 2);
            break;
        case SkPath::kCubic_Verb:
            elem.type = PathElementAddCurveToPoint;
            elem.points = setfpts(fpts, &pts[1], 3);
            break;
        case SkPath::kClose_Verb:
            elem.type = PathElementCloseSubpath;
            elem.points = setfpts(fpts, 0, 0);
            break;
        case SkPath::kDone_Verb:
            return;
        }
        function(info, &elem);
    }
}

void Path::transform(const TransformationMatrix& xform)
{
    m_path->transform(xform);
}
    
#if ENABLE(SVG)
String Path::debugString() const
{
    String result;

    SkPath::Iter iter(*m_path, false);
    SkPoint pts[4];

    int numPoints = m_path->getPoints(0, 0);
    SkPath::Verb verb;

    do {
        verb = iter.next(pts);
        switch (verb) {
        case SkPath::kMove_Verb:
            result += String::format("M%.2f,%.2f ", pts[0].fX, pts[0].fY);
            numPoints -= 1;
            break;
        case SkPath::kLine_Verb:
          if (!iter.isCloseLine()) {
                result += String::format("L%.2f,%.2f ", pts[1].fX, pts[1].fY); 
                numPoints -= 1;
            }
            break;
        case SkPath::kQuad_Verb:
            result += String::format("Q%.2f,%.2f,%.2f,%.2f ",
                pts[1].fX, pts[1].fY,
                pts[2].fX, pts[2].fY);
            numPoints -= 2;
            break;
        case SkPath::kCubic_Verb:
            result += String::format("C%.2f,%.2f,%.2f,%.2f,%.2f,%.2f ",
                pts[1].fX, pts[1].fY,
                pts[2].fX, pts[2].fY,
                pts[3].fX, pts[3].fY);
            numPoints -= 3;
            break;
        case SkPath::kClose_Verb:
            result += "Z ";
            break;
        case SkPath::kDone_Verb:
            break;
        }
    } while (verb != SkPath::kDone_Verb);

    // If you have a path that ends with an M, Skia will not iterate the
    // trailing M. That's nice of it, but Apple's paths output the trailing M
    // and we want out layout dumps to look like theirs
    if (numPoints) {
        ASSERT(numPoints==1);
        m_path->getLastPt(pts);
        result += String::format("M%.2f,%.2f ", pts[0].fX, pts[0].fY);
    }

    return result.stripWhiteSpace();
}
#endif

///////////////////////////////////////////////////////////////////////////////

// Computes the bounding box for the stroke and style currently selected into
// the given bounding box. This also takes into account the stroke width.
static FloatRect boundingBoxForCurrentStroke(GraphicsContext* context)
{
    const SkPath* path = context->getCurrPath();
    if (NULL == path) {
        return FloatRect();
    }

    SkPaint paint;
    context->setupStrokePaint(&paint);
    SkPath fillPath;
    paint.getFillPath(*path, &fillPath);
    const SkRect& r = fillPath.getBounds();
    return FloatRect(SkScalarToFloat(r.fLeft), SkScalarToFloat(r.fTop),
                     SkScalarToFloat(r.width()), SkScalarToFloat(r.height()));
}
    
static GraphicsContext* scratchContext()
{
    static ImageBuffer* scratch = 0;
    // TODO(benm): Confirm with reed that it's correct to use the (default) DeviceRGB ColorSpace parameter in the call to create below.
    if (!scratch)
        scratch = ImageBuffer::create(IntSize(1, 1)).release();
    // We don't bother checking for failure creating the ImageBuffer, since our
    // ImageBuffer initializer won't fail.
    return scratch->context();
}    

FloatRect Path::strokeBoundingRect(StrokeStyleApplier* applier)
{   
    GraphicsContext* scratch = scratchContext();
    scratch->save();
    scratch->beginPath();
    scratch->addPath(*this);
    
    if (applier)
        applier->strokeStyle(scratch);
    
    FloatRect r = boundingBoxForCurrentStroke(scratch);
    scratch->restore();
    return r;
}

#if ENABLE(SVG)
bool Path::strokeContains(StrokeStyleApplier* applier, const FloatPoint& point) const
{
#if 0
    ASSERT(applier);
    GraphicsContext* scratch = scratchContext();
    scratch->save();

    applier->strokeStyle(scratch);

    SkPaint paint;
    scratch->platformContext()->setupPaintForStroking(&paint, 0, 0);
    SkPath strokePath;
    paint.getFillPath(*platformPath(), &strokePath);
    bool contains = SkPathContainsPoint(&strokePath, point,
                                        SkPath::kWinding_FillType);

    scratch->restore();
    return contains;
#else
    // FIXME: 
    return false;
#endif
}
#endif

}
