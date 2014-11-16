/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2009 Apple Inc. All rights reserved.
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
#include "HTMLSelectElement.h"

#include "AXObjectCache.h"
#include "EventNames.h"
#include "HTMLNames.h"
#include "HTMLOptionElement.h"
#include "HTMLOptionsCollection.h"
#include "MappedAttribute.h"
#include "RenderListBox.h"
#include "RenderMenuList.h"
#include "ScriptEventListener.h"

using namespace std;

namespace WebCore {

using namespace HTMLNames;

// Upper limit agreed upon with representatives of Opera and Mozilla.
static const unsigned maxSelectItems = 10000;

HTMLSelectElement::HTMLSelectElement(const QualifiedName& tagName, Document* document, HTMLFormElement* form)
    : HTMLFormControlElementWithState(tagName, document, form)
{
    ASSERT(hasTagName(selectTag) || hasTagName(keygenTag));
}

bool HTMLSelectElement::checkDTD(const Node* newChild)
{
    // Make sure to keep <optgroup> in sync with this.
    return newChild->isTextNode() || newChild->hasTagName(optionTag) || newChild->hasTagName(optgroupTag) || newChild->hasTagName(hrTag) ||
           newChild->hasTagName(scriptTag);
}

void HTMLSelectElement::recalcStyle(StyleChange change)
{
    SelectElement::recalcStyle(m_data, this);
    HTMLFormControlElementWithState::recalcStyle(change);
}

const AtomicString& HTMLSelectElement::formControlType() const
{
    DEFINE_STATIC_LOCAL(const AtomicString, selectMultiple, ("select-multiple"));
    DEFINE_STATIC_LOCAL(const AtomicString, selectOne, ("select-one"));
    return m_data.multiple() ? selectMultiple : selectOne;
}

int HTMLSelectElement::selectedIndex() const
{
    return SelectElement::selectedIndex(m_data, this);
}

void HTMLSelectElement::deselectItems(HTMLOptionElement* excludeElement)
{
    SelectElement::deselectItems(m_data, this, excludeElement);
}

void HTMLSelectElement::setSelectedIndex(int optionIndex, bool deselect)
{
    SelectElement::setSelectedIndex(m_data, this, optionIndex, deselect, false, false);
}

void HTMLSelectElement::setSelectedIndexByUser(int optionIndex, bool deselect, bool fireOnChangeNow)
{
    SelectElement::setSelectedIndex(m_data, this, optionIndex, deselect, fireOnChangeNow, true);
}

int HTMLSelectElement::activeSelectionStartListIndex() const
{
    if (m_data.activeSelectionAnchorIndex() >= 0)
        return m_data.activeSelectionAnchorIndex();
    return optionToListIndex(selectedIndex());
}

int HTMLSelectElement::activeSelectionEndListIndex() const
{
    if (m_data.activeSelectionEndIndex() >= 0)
        return m_data.activeSelectionEndIndex();
    return SelectElement::lastSelectedListIndex(m_data, this);
}

unsigned HTMLSelectElement::length() const
{
    return SelectElement::optionCount(m_data, this);
}

void HTMLSelectElement::add(HTMLElement *element, HTMLElement *before, ExceptionCode& ec)
{
    RefPtr<HTMLElement> protectNewChild(element); // make sure the element is ref'd and deref'd so we don't leak it

    if (!element || !(element->hasLocalName(optionTag) || element->hasLocalName(hrTag)))
        return;

    insertBefore(element, before, ec);
}

void HTMLSelectElement::remove(int index)
{
    int listIndex = optionToListIndex(index);
    if (listIndex < 0)
        return;

    Element* item = listItems()[listIndex];
    ASSERT(item->parentNode());
    ExceptionCode ec;
    item->parentNode()->removeChild(item, ec);
}

String HTMLSelectElement::value()
{
    const Vector<Element*>& items = listItems();
    for (unsigned i = 0; i < items.size(); i++) {
        if (items[i]->hasLocalName(optionTag) && static_cast<HTMLOptionElement*>(items[i])->selected())
            return static_cast<HTMLOptionElement*>(items[i])->value();
    }
    return "";
}

void HTMLSelectElement::setValue(const String &value)
{
    if (value.isNull())
        return;
    // find the option with value() matching the given parameter
    // and make it the current selection.
    const Vector<Element*>& items = listItems();
    unsigned optionIndex = 0;
    for (unsigned i = 0; i < items.size(); i++) {
        if (items[i]->hasLocalName(optionTag)) {
            if (static_cast<HTMLOptionElement*>(items[i])->value() == value) {
                setSelectedIndex(optionIndex, true);
                return;
            }
            optionIndex++;
        }
    }
}

bool HTMLSelectElement::saveFormControlState(String& value) const
{
    return SelectElement::saveFormControlState(m_data, this, value);
}

void HTMLSelectElement::restoreFormControlState(const String& state)
{
    SelectElement::restoreFormControlState(m_data, this, state);
}

void HTMLSelectElement::parseMappedAttribute(MappedAttribute* attr) 
{
    bool oldUsesMenuList = m_data.usesMenuList();
    if (attr->name() == sizeAttr) {
        int oldSize = m_data.size();
        // Set the attribute value to a number.
        // This is important since the style rules for this attribute can determine the appearance property.
        int size = attr->value().toInt();
        String attrSize = String::number(size);
        if (attrSize != attr->value())
            attr->setValue(attrSize);

        m_data.setSize(max(size, 1));
        if ((oldUsesMenuList != m_data.usesMenuList() || (!oldUsesMenuList && m_data.size() != oldSize)) && attached()) {
            detach();
            attach();
            setRecalcListItems();
        }
    } else if (attr->name() == multipleAttr)
        SelectElement::parseMultipleAttribute(m_data, this, attr);
    else if (attr->name() == accesskeyAttr) {
        // FIXME: ignore for the moment
    } else if (attr->name() == alignAttr) {
        // Don't map 'align' attribute.  This matches what Firefox, Opera and IE do.
        // See http://bugs.webkit.org/show_bug.cgi?id=12072
    } else if (attr->name() == onfocusAttr) {
        setAttributeEventListener(eventNames().focusEvent, createAttributeEventListener(this, attr));
    } else if (attr->name() == onblurAttr) {
        setAttributeEventListener(eventNames().blurEvent, createAttributeEventListener(this, attr));
    } else if (attr->name() == onchangeAttr) {
        setAttributeEventListener(eventNames().changeEvent, createAttributeEventListener(this, attr));
    } else
        HTMLFormControlElementWithState::parseMappedAttribute(attr);
}

bool HTMLSelectElement::isKeyboardFocusable(KeyboardEvent* event) const
{
    if (renderer())
        return isFocusable();
    return HTMLFormControlElementWithState::isKeyboardFocusable(event);
}

bool HTMLSelectElement::isMouseFocusable() const
{
    if (renderer())
        return isFocusable();
    return HTMLFormControlElementWithState::isMouseFocusable();
}

bool HTMLSelectElement::canSelectAll() const
{
    return !m_data.usesMenuList(); 
}

void HTMLSelectElement::selectAll()
{
    SelectElement::selectAll(m_data, this);
}

RenderObject* HTMLSelectElement::createRenderer(RenderArena* arena, RenderStyle*)
{
    if (m_data.usesMenuList())
        return new (arena) RenderMenuList(this);
    return new (arena) RenderListBox(this);
}

bool HTMLSelectElement::appendFormData(FormDataList& list, bool)
{
    return SelectElement::appendFormData(m_data, this, list);
}

int HTMLSelectElement::optionToListIndex(int optionIndex) const
{
    return SelectElement::optionToListIndex(m_data, this, optionIndex);
}

int HTMLSelectElement::listToOptionIndex(int listIndex) const
{
    return SelectElement::listToOptionIndex(m_data, this, listIndex);
}

PassRefPtr<HTMLOptionsCollection> HTMLSelectElement::options()
{
    return HTMLOptionsCollection::create(this);
}

void HTMLSelectElement::recalcListItems(bool updateSelectedStates) const
{
    SelectElement::recalcListItems(const_cast<SelectElementData&>(m_data), this, updateSelectedStates);
}

void HTMLSelectElement::childrenChanged(bool changedByParser, Node* beforeChange, Node* afterChange, int childCountDelta)
{
    setRecalcListItems();
    HTMLFormControlElementWithState::childrenChanged(changedByParser, beforeChange, afterChange, childCountDelta);
    
    if (AXObjectCache::accessibilityEnabled() && renderer())
        renderer()->document()->axObjectCache()->childrenChanged(renderer());
}

void HTMLSelectElement::setRecalcListItems()
{
    SelectElement::setRecalcListItems(m_data, this);

    if (!inDocument())
        m_collectionInfo.reset();
}

void HTMLSelectElement::reset()
{
    SelectElement::reset(m_data, this);
}

void HTMLSelectElement::dispatchFocusEvent()
{
    SelectElement::dispatchFocusEvent(m_data, this);
    HTMLFormControlElementWithState::dispatchFocusEvent();
}

void HTMLSelectElement::dispatchBlurEvent()
{
    SelectElement::dispatchBlurEvent(m_data, this);
    HTMLFormControlElementWithState::dispatchBlurEvent();
}

void HTMLSelectElement::defaultEventHandler(Event* event)
{
    SelectElement::defaultEventHandler(m_data, this, event, form());
    if (event->defaultHandled())
        return;
    HTMLFormControlElementWithState::defaultEventHandler(event);
}

void HTMLSelectElement::setActiveSelectionAnchorIndex(int index)
{
    SelectElement::setActiveSelectionAnchorIndex(m_data, this, index);
}

void HTMLSelectElement::setActiveSelectionEndIndex(int index)
{
    SelectElement::setActiveSelectionEndIndex(m_data, index);
}

void HTMLSelectElement::updateListBoxSelection(bool deselectOtherOptions)
{
    SelectElement::updateListBoxSelection(m_data, this, deselectOtherOptions);
}

void HTMLSelectElement::menuListOnChange()
{
    SelectElement::menuListOnChange(m_data, this);
}

void HTMLSelectElement::listBoxOnChange()
{
    SelectElement::listBoxOnChange(m_data, this);
}

void HTMLSelectElement::saveLastSelection()
{
    SelectElement::saveLastSelection(m_data, this);
}

void HTMLSelectElement::accessKeyAction(bool sendToAnyElement)
{
    focus();
    dispatchSimulatedClick(0, sendToAnyElement);
}

void HTMLSelectElement::accessKeySetSelectedIndex(int index)
{
    SelectElement::accessKeySetSelectedIndex(m_data, this, index);
}
    
void HTMLSelectElement::setMultiple(bool multiple)
{
    setAttribute(multipleAttr, multiple ? "" : 0);
}

void HTMLSelectElement::setSize(int size)
{
    setAttribute(sizeAttr, String::number(size));
}

Node* HTMLSelectElement::namedItem(const AtomicString& name)
{
    return options()->namedItem(name);
}

Node* HTMLSelectElement::item(unsigned index)
{
    return options()->item(index);
}

void HTMLSelectElement::setOption(unsigned index, HTMLOptionElement* option, ExceptionCode& ec)
{
    ec = 0;
    if (index > maxSelectItems - 1)
        index = maxSelectItems - 1;
    int diff = index  - length();
    HTMLElement* before = 0;
    // out of array bounds ? first insert empty dummies
    if (diff > 0) {
        setLength(index, ec);
        // replace an existing entry ?
    } else if (diff < 0) {
        before = static_cast<HTMLElement*>(options()->item(index+1));
        remove(index);
    }
    // finally add the new element
    if (!ec) {
        add(option, before, ec);
        if (diff >= 0 && option->selected())
            setSelectedIndex(index, !m_data.multiple());
    }
}

void HTMLSelectElement::setLength(unsigned newLen, ExceptionCode& ec)
{
    ec = 0;
    if (newLen > maxSelectItems)
        newLen = maxSelectItems;
    int diff = length() - newLen;

    if (diff < 0) { // add dummy elements
        do {
            RefPtr<Element> option = document()->createElement(optionTag, false);
            ASSERT(option);
            add(static_cast<HTMLElement*>(option.get()), 0, ec);
            if (ec)
                break;
        } while (++diff);
    } else {
        const Vector<Element*>& items = listItems();

        size_t optionIndex = 0;
        for (size_t listIndex = 0; listIndex < items.size(); listIndex++) {
            if (items[listIndex]->hasLocalName(optionTag) && optionIndex++ >= newLen) {
                Element *item = items[listIndex];
                ASSERT(item->parentNode());
                item->parentNode()->removeChild(item, ec);
            }
        }
    }
}

void HTMLSelectElement::scrollToSelection()
{
    SelectElement::scrollToSelection(m_data, this);
}

void HTMLSelectElement::insertedIntoTree(bool deep)
{
    SelectElement::insertedIntoTree(m_data, this);
    HTMLFormControlElementWithState::insertedIntoTree(deep);
}

} // namespace
