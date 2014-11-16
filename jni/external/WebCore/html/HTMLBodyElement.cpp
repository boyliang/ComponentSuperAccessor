/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Simon Hausmann (hausmann@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
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
 */

#include "config.h"
#include "HTMLBodyElement.h"

#include "CSSStyleSelector.h"
#include "CSSStyleSheet.h"
#include "CSSValueKeywords.h"
#include "EventNames.h"
#include "Frame.h"
#include "FrameView.h"
#include "HTMLFrameElementBase.h"
#include "HTMLNames.h"
#include "MappedAttribute.h"
#include "ScriptEventListener.h"

#ifdef ANDROID_META_SUPPORT
#include "Settings.h"
#include "WebViewCore.h"
#endif

namespace WebCore {

using namespace HTMLNames;

HTMLBodyElement::HTMLBodyElement(const QualifiedName& tagName, Document* document)
    : HTMLElement(tagName, document)
{
    ASSERT(hasTagName(bodyTag));
}

HTMLBodyElement::~HTMLBodyElement()
{
    if (m_linkDecl) {
        m_linkDecl->setNode(0);
        m_linkDecl->setParent(0);
    }
}

void HTMLBodyElement::createLinkDecl()
{
    m_linkDecl = CSSMutableStyleDeclaration::create();
    m_linkDecl->setParent(document()->elementSheet());
    m_linkDecl->setNode(this);
    m_linkDecl->setStrictParsing(!document()->inCompatMode());
}

bool HTMLBodyElement::mapToEntry(const QualifiedName& attrName, MappedAttributeEntry& result) const
{
    if (attrName == backgroundAttr) {
        result = (MappedAttributeEntry)(eLastEntry + document()->docID());
        return false;
    } 
    
    if (attrName == bgcolorAttr ||
        attrName == textAttr ||
        attrName == marginwidthAttr ||
        attrName == leftmarginAttr ||
        attrName == marginheightAttr ||
        attrName == topmarginAttr ||
        attrName == bgpropertiesAttr) {
        result = eUniversal;
        return false;
    }

    return HTMLElement::mapToEntry(attrName, result);
}

void HTMLBodyElement::parseMappedAttribute(MappedAttribute *attr)
{
    if (attr->name() == backgroundAttr) {
        String url = deprecatedParseURL(attr->value());
        if (!url.isEmpty())
            addCSSImageProperty(attr, CSSPropertyBackgroundImage, document()->completeURL(url).string());
    } else if (attr->name() == marginwidthAttr || attr->name() == leftmarginAttr) {
        addCSSLength(attr, CSSPropertyMarginRight, attr->value());
        addCSSLength(attr, CSSPropertyMarginLeft, attr->value());
    } else if (attr->name() == marginheightAttr || attr->name() == topmarginAttr) {
        addCSSLength(attr, CSSPropertyMarginBottom, attr->value());
        addCSSLength(attr, CSSPropertyMarginTop, attr->value());
    } else if (attr->name() == bgcolorAttr) {
        addCSSColor(attr, CSSPropertyBackgroundColor, attr->value());
    } else if (attr->name() == textAttr) {
        addCSSColor(attr, CSSPropertyColor, attr->value());
    } else if (attr->name() == bgpropertiesAttr) {
        if (equalIgnoringCase(attr->value(), "fixed"))
            addCSSProperty(attr, CSSPropertyBackgroundAttachment, CSSValueFixed);
    } else if (attr->name() == vlinkAttr ||
               attr->name() == alinkAttr ||
               attr->name() == linkAttr) {
        if (attr->isNull()) {
            if (attr->name() == linkAttr)
                document()->resetLinkColor();
            else if (attr->name() == vlinkAttr)
                document()->resetVisitedLinkColor();
            else
                document()->resetActiveLinkColor();
        } else {
            if (!m_linkDecl)
                createLinkDecl();
            m_linkDecl->setProperty(CSSPropertyColor, attr->value(), false, false);
            RefPtr<CSSValue> val = m_linkDecl->getPropertyCSSValue(CSSPropertyColor);
            if (val && val->isPrimitiveValue()) {
                Color col = document()->styleSelector()->getColorFromPrimitiveValue(static_cast<CSSPrimitiveValue*>(val.get()));
                if (attr->name() == linkAttr)
                    document()->setLinkColor(col);
                else if (attr->name() == vlinkAttr)
                    document()->setVisitedLinkColor(col);
                else
                    document()->setActiveLinkColor(col);
            }
        }
        
        if (attached())
            document()->recalcStyle(Force);
    } else if (attr->name() == onloadAttr)
        document()->setWindowAttributeEventListener(eventNames().loadEvent, createAttributeEventListener(document()->frame(), attr));
    else if (attr->name() == onbeforeunloadAttr)
        document()->setWindowAttributeEventListener(eventNames().beforeunloadEvent, createAttributeEventListener(document()->frame(), attr));
    else if (attr->name() == onunloadAttr)
        document()->setWindowAttributeEventListener(eventNames().unloadEvent, createAttributeEventListener(document()->frame(), attr));
    else if (attr->name() == onblurAttr)
        document()->setWindowAttributeEventListener(eventNames().blurEvent, createAttributeEventListener(document()->frame(), attr));
    else if (attr->name() == onfocusAttr)
        document()->setWindowAttributeEventListener(eventNames().focusEvent, createAttributeEventListener(document()->frame(), attr));
    else if (attr->name() == onhashchangeAttr)
        document()->setWindowAttributeEventListener(eventNames().hashchangeEvent, createAttributeEventListener(document()->frame(), attr));
    else if (attr->name() == onresizeAttr)
        document()->setWindowAttributeEventListener(eventNames().resizeEvent, createAttributeEventListener(document()->frame(), attr));
    else if (attr->name() == onscrollAttr)
        document()->setWindowAttributeEventListener(eventNames().scrollEvent, createAttributeEventListener(document()->frame(), attr));
    else if (attr->name() == onstorageAttr)
        document()->setWindowAttributeEventListener(eventNames().storageEvent, createAttributeEventListener(document()->frame(), attr));
    else if (attr->name() == ononlineAttr)
        document()->setWindowAttributeEventListener(eventNames().onlineEvent, createAttributeEventListener(document()->frame(), attr));
    else if (attr->name() == onofflineAttr)
        document()->setWindowAttributeEventListener(eventNames().offlineEvent, createAttributeEventListener(document()->frame(), attr));
    else
        HTMLElement::parseMappedAttribute(attr);
}

void HTMLBodyElement::insertedIntoDocument()
{
    HTMLElement::insertedIntoDocument();

    // FIXME: Perhaps this code should be in attach() instead of here.
    Element* ownerElement = document()->ownerElement();
    if (ownerElement && (ownerElement->hasTagName(frameTag) || ownerElement->hasTagName(iframeTag))) {
        HTMLFrameElementBase* ownerFrameElement = static_cast<HTMLFrameElementBase*>(ownerElement);
        int marginWidth = ownerFrameElement->getMarginWidth();
        if (marginWidth != -1)
            setAttribute(marginwidthAttr, String::number(marginWidth));
        int marginHeight = ownerFrameElement->getMarginHeight();
        if (marginHeight != -1)
            setAttribute(marginheightAttr, String::number(marginHeight));
    }

#ifdef ANDROID_META_SUPPORT
    Settings * settings = document()->settings();
    if (settings) {
        String host = document()->baseURI().host().lower();
        if (settings->viewportWidth() == -1 && (host.startsWith("m.") || host.startsWith("mobile.")
                || host.contains(".m.") || host.contains(".mobile."))) {
            // fit mobile sites directly in the screen
            settings->setMetadataSettings("width", "device-width");
            // update the meta data if it is the top document
            if (!ownerElement) {
                FrameView* view = document()->view();
                if (view)
                    android::WebViewCore::getWebViewCore(view)->updateViewport();
            }
        }
    }
#endif

    // FIXME: This call to scheduleRelayout should not be needed here.
    // But without it we hang during WebKit tests; need to fix that and remove this.
    if (FrameView* view = document()->view())
        view->scheduleRelayout();
}

bool HTMLBodyElement::isURLAttribute(Attribute *attr) const
{
    return attr->name() == backgroundAttr;
}

String HTMLBodyElement::aLink() const
{
    return getAttribute(alinkAttr);
}

void HTMLBodyElement::setALink(const String& value)
{
    setAttribute(alinkAttr, value);
}

String HTMLBodyElement::bgColor() const
{
    return getAttribute(bgcolorAttr);
}

void HTMLBodyElement::setBgColor(const String& value)
{
    setAttribute(bgcolorAttr, value);
}

String HTMLBodyElement::link() const
{
    return getAttribute(linkAttr);
}

void HTMLBodyElement::setLink(const String& value)
{
    setAttribute(linkAttr, value);
}

String HTMLBodyElement::text() const
{
    return getAttribute(textAttr);
}

void HTMLBodyElement::setText(const String& value)
{
    setAttribute(textAttr, value);
}

String HTMLBodyElement::vLink() const
{
    return getAttribute(vlinkAttr);
}

void HTMLBodyElement::setVLink(const String& value)
{
    setAttribute(vlinkAttr, value);
}

static int adjustForZoom(int value, FrameView* frameView)
{
    float zoomFactor = frameView->frame()->zoomFactor();
    if (zoomFactor == 1)
        return value;
    // Needed because of truncation (rather than rounding) when scaling up.
    if (zoomFactor > 1)
        value++;
    return static_cast<int>(value / zoomFactor);
}

int HTMLBodyElement::scrollLeft() const
{
    // Update the document's layout.
    Document* doc = document();
    doc->updateLayoutIgnorePendingStylesheets();
    FrameView* view = doc->view();
    return view ? adjustForZoom(view->scrollX(), view) : 0;
}

void HTMLBodyElement::setScrollLeft(int scrollLeft)
{
    FrameView* sview = ownerDocument()->view();
    if (sview) {
        // Update the document's layout
        document()->updateLayoutIgnorePendingStylesheets();
        sview->setScrollPosition(IntPoint(static_cast<int>(scrollLeft * sview->frame()->zoomFactor()), sview->scrollY()));
    }    
}

int HTMLBodyElement::scrollTop() const
{
    // Update the document's layout.
    Document* doc = document();
    doc->updateLayoutIgnorePendingStylesheets();
    FrameView* view = doc->view();
    return view ? adjustForZoom(view->scrollY(), view) : 0;
}

void HTMLBodyElement::setScrollTop(int scrollTop)
{
    FrameView* sview = ownerDocument()->view();
    if (sview) {
        // Update the document's layout
        document()->updateLayoutIgnorePendingStylesheets();
        sview->setScrollPosition(IntPoint(sview->scrollX(), static_cast<int>(scrollTop * sview->frame()->zoomFactor())));
    }        
}

int HTMLBodyElement::scrollHeight() const
{
    // Update the document's layout.
    Document* doc = document();
    doc->updateLayoutIgnorePendingStylesheets();
    FrameView* view = doc->view();
    return view ? adjustForZoom(view->contentsHeight(), view) : 0;    
}

int HTMLBodyElement::scrollWidth() const
{
    // Update the document's layout.
    Document* doc = document();
    doc->updateLayoutIgnorePendingStylesheets();
    FrameView* view = doc->view();
    return view ? adjustForZoom(view->contentsWidth(), view) : 0;    
}

void HTMLBodyElement::addSubresourceAttributeURLs(ListHashSet<KURL>& urls) const
{
    HTMLElement::addSubresourceAttributeURLs(urls);

    addSubresourceURL(urls, document()->completeURL(getAttribute(backgroundAttr)));
}

void HTMLBodyElement::didMoveToNewOwnerDocument()
{
    // When moving body elements between documents, we should have to reset the parent sheet for any
    // link style declarations.  If we don't we might crash later.
    // In practice I can't reproduce this theoretical problem.
    // webarchive/adopt-attribute-styled-body-webarchive.html tries to make sure this crash won't surface.
    if (m_linkDecl)
        m_linkDecl->setParent(document()->elementSheet());
    
    HTMLElement::didMoveToNewOwnerDocument();
}

EventListener* HTMLBodyElement::onblur() const
{
    return document()->getWindowAttributeEventListener(eventNames().blurEvent);
}

void HTMLBodyElement::setOnblur(PassRefPtr<EventListener> eventListener)
{
    document()->setAttributeEventListener(eventNames().blurEvent, eventListener);
}

EventListener* HTMLBodyElement::onerror() const
{
    return document()->getWindowAttributeEventListener(eventNames().errorEvent);
}

void HTMLBodyElement::setOnerror(PassRefPtr<EventListener> eventListener)
{
    document()->setAttributeEventListener(eventNames().errorEvent, eventListener);
}

EventListener* HTMLBodyElement::onfocus() const
{
    return document()->getWindowAttributeEventListener(eventNames().focusEvent);
}

void HTMLBodyElement::setOnfocus(PassRefPtr<EventListener> eventListener)
{
    document()->setAttributeEventListener(eventNames().focusEvent, eventListener);
}

EventListener* HTMLBodyElement::onload() const
{
    return document()->getWindowAttributeEventListener(eventNames().loadEvent);
}

void HTMLBodyElement::setOnload(PassRefPtr<EventListener> eventListener)
{
    document()->setAttributeEventListener(eventNames().loadEvent, eventListener);
}

EventListener* HTMLBodyElement::onbeforeunload() const
{
    return document()->getWindowAttributeEventListener(eventNames().beforeunloadEvent);
}

void HTMLBodyElement::setOnbeforeunload(PassRefPtr<EventListener> eventListener)
{
    document()->setAttributeEventListener(eventNames().beforeunloadEvent, eventListener);
}

EventListener* HTMLBodyElement::onmessage() const
{
    return document()->getWindowAttributeEventListener(eventNames().messageEvent);
}

void HTMLBodyElement::setOnmessage(PassRefPtr<EventListener> eventListener)
{
    document()->setAttributeEventListener(eventNames().messageEvent, eventListener);
}

EventListener* HTMLBodyElement::onoffline() const
{
    return document()->getWindowAttributeEventListener(eventNames().offlineEvent);
}

void HTMLBodyElement::setOnoffline(PassRefPtr<EventListener> eventListener)
{
    document()->setAttributeEventListener(eventNames().offlineEvent, eventListener);
}

EventListener* HTMLBodyElement::ononline() const
{
    return document()->getWindowAttributeEventListener(eventNames().onlineEvent);
}

void HTMLBodyElement::setOnonline(PassRefPtr<EventListener> eventListener)
{
    document()->setAttributeEventListener(eventNames().onlineEvent, eventListener);
}

EventListener* HTMLBodyElement::onresize() const
{
    return document()->getWindowAttributeEventListener(eventNames().resizeEvent);
}

void HTMLBodyElement::setOnresize(PassRefPtr<EventListener> eventListener)
{
    document()->setAttributeEventListener(eventNames().resizeEvent, eventListener);
}

EventListener* HTMLBodyElement::onstorage() const
{
    return document()->getWindowAttributeEventListener(eventNames().storageEvent);
}

void HTMLBodyElement::setOnstorage(PassRefPtr<EventListener> eventListener)
{
    document()->setAttributeEventListener(eventNames().storageEvent, eventListener);
}

EventListener* HTMLBodyElement::onunload() const
{
    return document()->getWindowAttributeEventListener(eventNames().unloadEvent);
}

void HTMLBodyElement::setOnunload(PassRefPtr<EventListener> eventListener)
{
    document()->setAttributeEventListener(eventNames().unloadEvent, eventListener);
}

} // namespace WebCore
