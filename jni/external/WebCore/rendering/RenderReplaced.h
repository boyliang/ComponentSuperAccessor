/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2009 Apple Inc. All rights reserved.
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

#ifndef RenderReplaced_h
#define RenderReplaced_h

#include "RenderBox.h"

namespace WebCore {

class RenderReplaced : public RenderBox {
public:
    RenderReplaced(Node*);
    RenderReplaced(Node*, const IntSize& intrinsicSize);
    virtual ~RenderReplaced();

protected:
    virtual void layout();

    virtual IntSize intrinsicSize() const;

    virtual void setSelectionState(SelectionState);

    bool isSelected() const;

    virtual void styleDidChange(StyleDifference, const RenderStyle* oldStyle);

    void setIntrinsicSize(const IntSize&);
    virtual void intrinsicSizeChanged();

    bool shouldPaint(PaintInfo&, int& tx, int& ty);
    void adjustOverflowForBoxShadowAndReflect();
    IntRect localSelectionRect(bool checkWhetherSelected = true) const;

private:
    virtual const char* renderName() const { return "RenderReplaced"; }

    virtual bool canHaveChildren() const { return false; }

    virtual int lineHeight(bool firstLine, bool isRootLineBox = false) const;
    virtual int baselinePosition(bool firstLine, bool isRootLineBox = false) const;

    virtual void calcPrefWidths();

    virtual int minimumReplacedHeight() const { return 0; }

    virtual void paint(PaintInfo&, int tx, int ty);
    virtual void paintReplaced(PaintInfo&, int /*tx*/, int /*ty*/) { }

    virtual int overflowHeight(bool includeInterior = true) const;
    virtual int overflowWidth(bool includeInterior = true) const;
    virtual int overflowLeft(bool includeInterior = true) const;
    virtual int overflowTop(bool includeInterior = true) const;
    virtual IntRect overflowRect(bool includeInterior = true) const;

    virtual IntRect clippedOverflowRectForRepaint(RenderBoxModelObject* repaintContainer);

    virtual unsigned caretMaxRenderedOffset() const;
    virtual VisiblePosition positionForPoint(const IntPoint&);
    
    virtual bool canBeSelectionLeaf() const { return true; }

    virtual IntRect selectionRectForRepaint(RenderBoxModelObject* repaintContainer, bool clipToVisibleContent = true);

    IntSize m_intrinsicSize;
};

}

#endif
