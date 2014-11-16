/*
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
    Copyright (C) 2007 Staikos Computing Services Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QWEBFRAME_P_H
#define QWEBFRAME_P_H

#include "qwebframe.h"
#include "qwebpage_p.h"

#include "EventHandler.h"
#include "KURL.h"
#include "PlatformString.h"
#include "qwebelement.h"
#include "wtf/RefPtr.h"
#include "Frame.h"

namespace WebCore {
    class FrameLoaderClientQt;
    class FrameView;
    class HTMLFrameOwnerElement;
    class Scrollbar;
}
class QWebPage;

class QWebFrameData {
public:
    QWebFrameData(WebCore::Page*, WebCore::Frame* parentFrame = 0,
                  WebCore::HTMLFrameOwnerElement* = 0,
                  const WebCore::String& frameName = WebCore::String());

    WebCore::KURL url;
    WebCore::String name;
    WebCore::HTMLFrameOwnerElement* ownerElement;
    WebCore::Page* page;
    RefPtr<WebCore::Frame> frame;
    WebCore::FrameLoaderClientQt* frameLoaderClient;

    WebCore::String referrer;
    bool allowsScrolling;
    int marginWidth;
    int marginHeight;
};

class QWebFramePrivate {
public:
    QWebFramePrivate()
        : q(0)
        , horizontalScrollBarPolicy(Qt::ScrollBarAsNeeded)
        , verticalScrollBarPolicy(Qt::ScrollBarAsNeeded)
        , frameLoaderClient(0)
        , frame(0)
        , page(0)
        , allowsScrolling(true)
        , marginWidth(-1)
        , marginHeight(-1)
        , clipRenderToViewport(true)
        {}
    void init(QWebFrame* qframe, QWebFrameData* frameData);

    inline QWebFrame *parentFrame() { return qobject_cast<QWebFrame*>(q->parent()); }

    WebCore::Scrollbar* horizontalScrollBar() const;
    WebCore::Scrollbar* verticalScrollBar() const;

    Qt::ScrollBarPolicy horizontalScrollBarPolicy;
    Qt::ScrollBarPolicy verticalScrollBarPolicy;

    static WebCore::Frame* core(QWebFrame*);
    static QWebFrame* kit(WebCore::Frame*);

    void renderPrivate(QPainter *painter, const QRegion &clip);

    QWebFrame *q;
    WebCore::FrameLoaderClientQt *frameLoaderClient;
    WebCore::Frame *frame;
    QWebPage *page;

    bool allowsScrolling;
    int marginWidth;
    int marginHeight;
    bool clipRenderToViewport;
};

class QWebHitTestResultPrivate {
public:
    QWebHitTestResultPrivate() : isContentEditable(false), isContentSelected(false), isScrollBar(false) {}
    QWebHitTestResultPrivate(const WebCore::HitTestResult &hitTest);

    QPoint pos;
    QRect boundingRect;
    QWebElement enclosingBlock;
    QString title;
    QString linkText;
    QUrl linkUrl;
    QString linkTitle;
    QPointer<QWebFrame> linkTargetFrame;
    QWebElement linkElement;
    QString alternateText;
    QUrl imageUrl;
    QPixmap pixmap;
    bool isContentEditable;
    bool isContentSelected;
    bool isScrollBar;
    QPointer<QWebFrame> frame;
    RefPtr<WebCore::Node> innerNode;
    RefPtr<WebCore::Node> innerNonSharedNode;
};

#endif
