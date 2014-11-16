/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2006, 2008, 2009 Apple Inc. All rights reserved.
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

#ifndef HTMLAppletElement_h
#define HTMLAppletElement_h

#include "HTMLPlugInElement.h"

namespace WebCore {

class HTMLFormElement;
class HTMLImageLoader;

class HTMLAppletElement : public HTMLPlugInElement {
public:
    HTMLAppletElement(const QualifiedName&, Document*);

    String hspace() const;
    void setHspace(const String&);

    String vspace() const;
    void setVspace(const String&);

private:
    virtual int tagPriority() const { return 1; }

    virtual void parseMappedAttribute(MappedAttribute*);
    
    virtual bool rendererIsNeeded(RenderStyle*);
    virtual RenderObject* createRenderer(RenderArena*, RenderStyle*);
    virtual void finishParsingChildren();
    
    virtual RenderWidget* renderWidgetForJSBindings() const;

    void setupApplet() const;

    virtual void insertedIntoDocument();
    virtual void removedFromDocument();

    AtomicString m_id;
};

}

#endif
