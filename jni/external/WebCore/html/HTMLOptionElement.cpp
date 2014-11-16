/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006 Apple Computer, Inc.
 *           (C) 2006 Alexey Proskuryakov (ap@nypop.com)
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

#include "config.h"
#include "HTMLOptionElement.h"

#include "CSSStyleSelector.h"
#include "Document.h"
#include "ExceptionCode.h"
#include "HTMLNames.h"
#include "HTMLSelectElement.h"
#include "MappedAttribute.h"
#include "NodeRenderStyle.h"
#include "RenderMenuList.h"
#include "Text.h"
#include <wtf/StdLibExtras.h>
#include <wtf/Vector.h>

namespace WebCore {

using namespace HTMLNames;

HTMLOptionElement::HTMLOptionElement(const QualifiedName& tagName, Document* doc, HTMLFormElement* f)
    : HTMLFormControlElement(tagName, doc, f)
    , m_style(0)
{
    ASSERT(hasTagName(optionTag));
}

bool HTMLOptionElement::checkDTD(const Node* newChild)
{
    return newChild->isTextNode() || newChild->hasTagName(scriptTag);
}

void HTMLOptionElement::attach()
{
    if (parentNode()->renderStyle())
        setRenderStyle(styleForRenderer());
    HTMLFormControlElement::attach();
}

void HTMLOptionElement::detach()
{
    m_style.clear();
    HTMLFormControlElement::detach();
}

bool HTMLOptionElement::isFocusable() const
{
    return HTMLElement::isFocusable();
}

const AtomicString& HTMLOptionElement::formControlType() const
{
    DEFINE_STATIC_LOCAL(const AtomicString, option, ("option"));
    return option;
}

String HTMLOptionElement::text() const
{
    return OptionElement::collectOptionLabelOrText(m_data, this);
}

void HTMLOptionElement::setText(const String &text, ExceptionCode& ec)
{
    // Handle the common special case where there's exactly 1 child node, and it's a text node.
    Node* child = firstChild();
    if (child && child->isTextNode() && !child->nextSibling()) {
        static_cast<Text *>(child)->setData(text, ec);
        return;
    }

    removeChildren();
    appendChild(new Text(document(), text), ec);
}

void HTMLOptionElement::accessKeyAction(bool)
{
    HTMLSelectElement* select = ownerSelectElement();
    if (select)
        select->accessKeySetSelectedIndex(index());
}

int HTMLOptionElement::index() const
{
    return OptionElement::optionIndex(ownerSelectElement(), this);
}

void HTMLOptionElement::parseMappedAttribute(MappedAttribute *attr)
{
    if (attr->name() == selectedAttr)
        m_data.setSelected(!attr->isNull());
    else if (attr->name() == valueAttr)
        m_data.setValue(attr->value());
    else if (attr->name() == labelAttr)
        m_data.setLabel(attr->value());
    else
        HTMLFormControlElement::parseMappedAttribute(attr);
}

String HTMLOptionElement::value() const
{
    return OptionElement::collectOptionValue(m_data, this);
}

void HTMLOptionElement::setValue(const String& value)
{
    setAttribute(valueAttr, value);
}

bool HTMLOptionElement::selected() const
{
    return m_data.selected();
}

void HTMLOptionElement::setSelected(bool selected)
{
    if (m_data.selected() == selected)
        return;

    OptionElement::setSelectedState(m_data, this, selected);

    if (HTMLSelectElement* select = ownerSelectElement())
        select->setSelectedIndex(selected ? index() : -1, false);
}

void HTMLOptionElement::setSelectedState(bool selected)
{
    OptionElement::setSelectedState(m_data, this, selected);
}

void HTMLOptionElement::childrenChanged(bool changedByParser, Node* beforeChange, Node* afterChange, int childCountDelta)
{
    HTMLSelectElement* select = ownerSelectElement();
    if (select)
        select->childrenChanged(changedByParser);
    HTMLFormControlElement::childrenChanged(changedByParser, beforeChange, afterChange, childCountDelta);
}

HTMLSelectElement* HTMLOptionElement::ownerSelectElement() const
{
    Node* select = parentNode();
#ifdef ANDROID_FIX
    while (select && !(select->hasTagName(selectTag) || select->hasTagName(keygenTag)))
#else
    while (select && !select->hasTagName(selectTag))
#endif
        select = select->parentNode();

    if (!select)
        return 0;
    
    return static_cast<HTMLSelectElement*>(select);
}

bool HTMLOptionElement::defaultSelected() const
{
    return !getAttribute(selectedAttr).isNull();
}

void HTMLOptionElement::setDefaultSelected(bool b)
{
    setAttribute(selectedAttr, b ? "" : 0);
}

String HTMLOptionElement::label() const
{
    return m_data.label();
}

void HTMLOptionElement::setLabel(const String& value)
{
    setAttribute(labelAttr, value);
}

void HTMLOptionElement::setRenderStyle(PassRefPtr<RenderStyle> newStyle)
{
    m_style = newStyle;
}

RenderStyle* HTMLOptionElement::nonRendererRenderStyle() const 
{ 
    return m_style.get(); 
}

String HTMLOptionElement::textIndentedToRespectGroupLabel() const
{
    return OptionElement::collectOptionTextRespectingGroupLabel(m_data, this);
}

bool HTMLOptionElement::disabled() const
{ 
    return HTMLFormControlElement::disabled() || (parentNode() && static_cast<HTMLFormControlElement*>(parentNode())->disabled()); 
}

void HTMLOptionElement::insertedIntoTree(bool deep)
{
    if (HTMLSelectElement* select = ownerSelectElement()) {
        select->setRecalcListItems();
        if (selected())
            select->setSelectedIndex(index(), false);
        select->scrollToSelection();
    }
    
    HTMLFormControlElement::insertedIntoTree(deep);
}

} // namespace
