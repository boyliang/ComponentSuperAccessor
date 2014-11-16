/*
 * Copyright (C) 2008 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
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

#ifndef InputElement_h
#define InputElement_h

#include "AtomicString.h"
#include "PlatformString.h"

namespace WebCore {

class Document;
class Element;
class Event;
class InputElementData;
class MappedAttribute;

class InputElement {
public:
    virtual ~InputElement() { }

    virtual bool isAutofilled() const = 0;
    virtual bool isChecked() const = 0;
    virtual bool isIndeterminate() const = 0;
    virtual bool isInputTypeHidden() const = 0;
    virtual bool isPasswordField() const = 0;
    virtual bool isSearchField() const = 0;
    virtual bool isTextField() const = 0;

    virtual bool placeholderShouldBeVisible() const = 0;
    virtual bool searchEventsShouldBeDispatched() const = 0;

    virtual int size() const = 0;
    virtual String value() const = 0;
    virtual void setValue(const String&) = 0;

    virtual String placeholder() const = 0;
    virtual void setPlaceholder(const String&) = 0;

    virtual String constrainValue(const String&) const = 0;
    virtual void setValueFromRenderer(const String&) = 0;

    virtual void cacheSelection(int start, int end) = 0;
    virtual void select() = 0;

    static const int s_maximumLength;
    static const int s_defaultSize;

protected:
    static void dispatchFocusEvent(InputElementData&, InputElement*, Element*);
    static void dispatchBlurEvent(InputElementData&, InputElement*, Element*);
    static void updatePlaceholderVisibility(InputElementData&, InputElement*, Element*, bool placeholderValueChanged = false);
    static void updateFocusAppearance(InputElementData&, InputElement*, Element*, bool restorePreviousSelection);
    static void updateSelectionRange(InputElement*, Element*, int start, int end);
    static void aboutToUnload(InputElement*, Element*);
    static void setValueFromRenderer(InputElementData&, InputElement*, Element*, const String&);
    static String constrainValue(const InputElement*, const String& proposedValue, int maxLength);
    static void handleBeforeTextInsertedEvent(InputElementData&, InputElement*, Document*, Event*);
    static void parseSizeAttribute(InputElementData&, Element*, MappedAttribute*);
    static void parseMaxLengthAttribute(InputElementData&, InputElement*, Element*, MappedAttribute*);
    static void updateValueIfNeeded(InputElementData&, InputElement*);
    static void notifyFormStateChanged(Element*);
};

// HTML/WMLInputElement hold this struct as member variable
// and pass it to the static helper functions in InputElement
class InputElementData {
public:
    InputElementData();

    bool placeholderShouldBeVisible() const { return m_placeholderShouldBeVisible; }
    void setPlaceholderShouldBeVisible(bool visible) { m_placeholderShouldBeVisible = visible; }

    const AtomicString& name() const;
    void setName(const AtomicString& value) { m_name = value; }

    String value() const { return m_value; }
    void setValue(const String& value) { m_value = value; }

    int size() const { return m_size; }
    void setSize(int value) { m_size = value; }

    int maxLength() const { return m_maxLength; }
    void setMaxLength(int value) { m_maxLength = value; }

    int cachedSelectionStart() const { return m_cachedSelectionStart; }
    void setCachedSelectionStart(int value) { m_cachedSelectionStart = value; }

    int cachedSelectionEnd() const { return m_cachedSelectionEnd; }
    void setCachedSelectionEnd(int value) { m_cachedSelectionEnd = value; }

private:
    bool m_placeholderShouldBeVisible;
    AtomicString m_name;
    String m_value;
    int m_size;
    int m_maxLength;
    int m_cachedSelectionStart;
    int m_cachedSelectionEnd;
};

InputElement* toInputElement(Element*);

}

#endif
