/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef RenderInline_h
#define RenderInline_h

#include "RenderBoxModelObject.h"
#include "RenderLineBoxList.h"

namespace WebCore {

class Position;

class RenderInline : public RenderBoxModelObject {
public:
    RenderInline(Node*);

    virtual void destroy();

    virtual void addChild(RenderObject* newChild, RenderObject* beforeChild = 0);

    virtual int marginLeft() const;
    virtual int marginRight() const;
    
    virtual void absoluteRects(Vector<IntRect>&, int tx, int ty);
    virtual void absoluteQuads(Vector<FloatQuad>&);

    IntRect linesBoundingBox() const;

    InlineFlowBox* createAndAppendInlineFlowBox();

    void dirtyLineBoxes(bool fullLayout);

    RenderLineBoxList* lineBoxes() { return &m_lineBoxes; }
    const RenderLineBoxList* lineBoxes() const { return &m_lineBoxes; }

    InlineFlowBox* firstLineBox() const { return m_lineBoxes.firstLineBox(); }
    InlineFlowBox* lastLineBox() const { return m_lineBoxes.lastLineBox(); }

    RenderBoxModelObject* continuation() const { return m_continuation; }

    virtual void updateDragState(bool dragOn);
    
    IntSize relativePositionedInlineOffset(const RenderBox* child) const;

    virtual void addFocusRingRects(GraphicsContext*, int tx, int ty);
    void paintOutline(GraphicsContext*, int tx, int ty);

    int verticalPositionFromCache(bool firstLine) const;
    void invalidateVerticalPosition() { m_verticalPosition = PositionUndefined; }

private:
    virtual RenderObjectChildList* virtualChildren() { return children(); }
    virtual const RenderObjectChildList* virtualChildren() const { return children(); }
    const RenderObjectChildList* children() const { return &m_children; }
    RenderObjectChildList* children() { return &m_children; }

    virtual const char* renderName() const;

    virtual bool isRenderInline() const { return true; }

    void addChildToContinuation(RenderObject* newChild, RenderObject* beforeChild);
    virtual void addChildIgnoringContinuation(RenderObject* newChild, RenderObject* beforeChild = 0);

    void splitInlines(RenderBlock* fromBlock, RenderBlock* toBlock, RenderBlock* middleBlock,
                      RenderObject* beforeChild, RenderBoxModelObject* oldCont);
    void splitFlow(RenderObject* beforeChild, RenderBlock* newBlockBox,
                   RenderObject* newChild, RenderBoxModelObject* oldCont);

    virtual void layout() { ASSERT_NOT_REACHED(); } // Do nothing for layout()

    virtual void paint(PaintInfo&, int tx, int ty);

    virtual bool nodeAtPoint(const HitTestRequest&, HitTestResult&, int x, int y, int tx, int ty, HitTestAction);

    virtual bool requiresLayer() const { return isRelPositioned() || isTransparent() || hasMask(); }

    virtual int offsetLeft() const;
    virtual int offsetTop() const;
    virtual int offsetWidth() const { return linesBoundingBox().width(); }
    virtual int offsetHeight() const { return linesBoundingBox().height(); }

    // Just ignore top/bottom margins on RenderInlines.
    virtual int marginTop() const { return 0; }
    virtual int marginBottom() const { return 0; }
    virtual IntRect clippedOverflowRectForRepaint(RenderBoxModelObject* repaintContainer);
    virtual IntRect rectWithOutlineForRepaint(RenderBoxModelObject* repaintContainer, int outlineWidth);
    virtual void computeRectForRepaint(RenderBoxModelObject* repaintContainer, IntRect& rect, bool fixed);

    virtual VisiblePosition positionForPoint(const IntPoint&);

    virtual IntRect borderBoundingBox() const
    {
        IntRect boundingBox = linesBoundingBox();
        return IntRect(0, 0, boundingBox.width(), boundingBox.height());
    }

    virtual InlineFlowBox* createInlineFlowBox(); // Subclassed by SVG and Ruby

    virtual void dirtyLinesFromChangedChild(RenderObject* child) { m_lineBoxes.dirtyLinesFromChangedChild(this, child); }

    virtual int lineHeight(bool firstLine, bool isRootLineBox = false) const;

    RenderInline* inlineContinuation() const;
    void setContinuation(RenderBoxModelObject* c) { m_continuation = c; }
    
    virtual void childBecameNonInline(RenderObject* child);

    virtual void updateHitTestResult(HitTestResult&, const IntPoint&);

    virtual void imageChanged(WrappedImagePtr, const IntRect* = 0);

#if ENABLE(DASHBOARD_SUPPORT)
    virtual void addDashboardRegions(Vector<DashboardRegionValue>&);
#endif
    
    virtual void styleDidChange(StyleDifference, const RenderStyle* oldStyle);
    virtual void updateBoxModelInfoFromStyle();
    
    static RenderInline* cloneInline(RenderInline* src);

    void paintOutlineForLine(GraphicsContext*, int tx, int ty, const IntRect& prevLine, const IntRect& thisLine, const IntRect& nextLine);
    RenderBoxModelObject* continuationBefore(RenderObject* beforeChild);

    RenderObjectChildList m_children;
    RenderLineBoxList m_lineBoxes;   // All of the line boxes created for this inline flow.  For example, <i>Hello<br>world.</i> will have two <i> line boxes.

    RenderBoxModelObject* m_continuation; // Can be either a block or an inline. <b><i><p>Hello</p></i></b>. In this example the <i> will have a block as its continuation but the
                                          // <b> will just have an inline as its continuation.
    mutable int m_lineHeight;
    mutable int m_verticalPosition;
};

inline RenderInline* toRenderInline(RenderObject* object)
{ 
    ASSERT(!object || object->isRenderInline());
    return static_cast<RenderInline*>(object);
}

inline const RenderInline* toRenderInline(const RenderObject* object)
{ 
    ASSERT(!object || object->isRenderInline());
    return static_cast<const RenderInline*>(object);
}

// This will catch anyone doing an unnecessary cast.
void toRenderInline(const RenderInline*);

} // namespace WebCore

#endif // RenderInline_h
