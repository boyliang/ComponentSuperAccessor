/*
 * Copyright (C) 2006, 2007, 2008 Apple Computer, Inc. All rights reserved.
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2007, 2009 Holger Hans Peter Freyther
 * Copyright (C) 2008 Collabora Ltd.
 *
 * All rights reserved.
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
#include "ScrollView.h"

#include "FloatRect.h"
#include "GraphicsContext.h"
#include "HostWindow.h"
#include "IntRect.h"
#include "PlatformMouseEvent.h"
#include "PlatformWheelEvent.h"
#include "ScrollbarGtk.h"
#include "ScrollbarTheme.h"

#include <gtk/gtk.h>

using namespace std;

namespace WebCore {

void ScrollView::platformInit()
{
    m_horizontalAdjustment = 0;
    m_verticalAdjustment = 0;
}

void ScrollView::platformDestroy()
{
    m_horizontalAdjustment = 0;
    m_verticalAdjustment = 0;
}

PassRefPtr<Scrollbar> ScrollView::createScrollbar(ScrollbarOrientation orientation)
{
    if (orientation == HorizontalScrollbar && m_horizontalAdjustment)
        return ScrollbarGtk::createScrollbar(this, orientation, m_horizontalAdjustment);
    else if (orientation == VerticalScrollbar && m_verticalAdjustment)
        return ScrollbarGtk::createScrollbar(this, orientation, m_verticalAdjustment);
    else
        return Scrollbar::createNativeScrollbar(this, orientation, RegularScrollbar);
}

/*
 * The following is assumed:
 *   (hadj && vadj) || (!hadj && !vadj)
 */
void ScrollView::setGtkAdjustments(GtkAdjustment* hadj, GtkAdjustment* vadj)
{
    ASSERT(!hadj == !vadj);

    m_horizontalAdjustment = hadj;
    m_verticalAdjustment = vadj;

    // Reset the adjustments to a sane default
    if (m_horizontalAdjustment) {
        m_horizontalAdjustment->lower = 0;
        m_horizontalAdjustment->upper = 0;
        m_horizontalAdjustment->value = 0;
        gtk_adjustment_changed(m_horizontalAdjustment);
        gtk_adjustment_value_changed(m_horizontalAdjustment);

        m_verticalAdjustment->lower = 0;
        m_verticalAdjustment->upper = 0;
        m_verticalAdjustment->value = 0;
        gtk_adjustment_changed(m_verticalAdjustment);
        gtk_adjustment_value_changed(m_verticalAdjustment);
    }

    /* reconsider having a scrollbar */
    setHasVerticalScrollbar(false);
    setHasHorizontalScrollbar(false);
    updateScrollbars(m_scrollOffset);
}

void ScrollView::platformAddChild(Widget* child)
{
    if (!GTK_IS_SOCKET(child->platformWidget()))
        gtk_container_add(GTK_CONTAINER(hostWindow()->platformWindow()), child->platformWidget());
}

void ScrollView::platformRemoveChild(Widget* child)
{
    GtkWidget* parent;

    // HostWindow can be NULL here. If that's the case
    // let's grab the child's parent instead.
    if (hostWindow())
        parent = GTK_WIDGET(hostWindow()->platformWindow());
    else
        parent = GTK_WIDGET(child->platformWidget()->parent);

    if (GTK_IS_CONTAINER(parent) && parent == child->platformWidget()->parent)
        gtk_container_remove(GTK_CONTAINER(parent), child->platformWidget());
}

IntRect ScrollView::visibleContentRect(bool includeScrollbars) const
{
    if (!m_horizontalAdjustment)
        return IntRect(IntPoint(m_scrollOffset.width(), m_scrollOffset.height()),
                       IntSize(max(0, width() - (verticalScrollbar() && !includeScrollbars ? verticalScrollbar()->width() : 0)),
                               max(0, height() - (horizontalScrollbar() && !includeScrollbars ? horizontalScrollbar()->height() : 0))));

    // Main frame.
    GtkWidget* measuredWidget = hostWindow()->platformWindow();
    GtkWidget* parent = gtk_widget_get_parent(measuredWidget);

    // We may not be in a widget that displays scrollbars, but we may
    // have other kinds of decoration that make us smaller.
    if (parent && includeScrollbars)
        measuredWidget = parent;

    return IntRect(IntPoint(m_scrollOffset.width(), m_scrollOffset.height()),
                   IntSize(measuredWidget->allocation.width,
                           measuredWidget->allocation.height));
}

}
