/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2008, 2009 Apple Inc. All rights reserved.
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
#include "HTMLBaseElement.h"

#include "CSSHelper.h"
#include "Document.h"
#include "Frame.h"
#include "HTMLNames.h"
#include "MappedAttribute.h"
#include "XSSAuditor.h"

namespace WebCore {

using namespace HTMLNames;

HTMLBaseElement::HTMLBaseElement(const QualifiedName& qName, Document* document)
    : HTMLElement(qName, document)
{
    ASSERT(hasTagName(baseTag));
}

void HTMLBaseElement::parseMappedAttribute(MappedAttribute* attr)
{
    if (attr->name() == hrefAttr) {
        m_hrefAttrValue = attr->value();
        m_href = deprecatedParseURL(attr->value());
        process();
    } else if (attr->name() == targetAttr) {
        m_target = attr->value();
        process();
    } else
        HTMLElement::parseMappedAttribute(attr);
}

void HTMLBaseElement::insertedIntoDocument()
{
    HTMLElement::insertedIntoDocument();
    process();
}

void HTMLBaseElement::removedFromDocument()
{
    HTMLElement::removedFromDocument();

    // Since the document doesn't have a base element, clear the base URL and target.
    // FIXME: This does not handle the case of multiple base elements correctly.
    document()->setBaseElementURL(KURL());
    document()->setBaseElementTarget(String());
}

void HTMLBaseElement::process()
{
    if (!inDocument())
        return;

    if (!m_href.isEmpty() && (!document()->frame() || document()->frame()->script()->xssAuditor()->canSetBaseElementURL(m_hrefAttrValue)))
        document()->setBaseElementURL(KURL(document()->url(), m_href));

    if (!m_target.isEmpty())
        document()->setBaseElementTarget(m_target);

    // FIXME: Changing a document's base URL should probably automatically update the resolved relative URLs of all images, stylesheets, etc.
}

}
