/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007 Apple Inc. All rights reserved.
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
#include "HTMLFormControlElement.h"

#include "ChromeClient.h"
#include "Document.h"
#include "EventHandler.h"
#include "EventNames.h"
#include "Frame.h"
#include "HTMLFormElement.h"
#include "HTMLInputElement.h"
#include "HTMLNames.h"
#include "HTMLParser.h"
#include "HTMLTokenizer.h"
#include "MappedAttribute.h"
#include "Page.h"
#include "RenderBox.h"
#include "RenderTheme.h"
#include "ValidityState.h"

namespace WebCore {

using namespace HTMLNames;

HTMLFormControlElement::HTMLFormControlElement(const QualifiedName& tagName, Document* doc, HTMLFormElement* f)
    : HTMLElement(tagName, doc)
    , m_form(f)
    , m_disabled(false)
    , m_readOnly(false)
    , m_valueMatchesRenderer(false)
{
    if (!m_form)
        m_form = findFormAncestor();
    if (m_form)
        m_form->registerFormElement(this);
}

HTMLFormControlElement::~HTMLFormControlElement()
{
    if (m_form)
        m_form->removeFormElement(this);
}

ValidityState* HTMLFormControlElement::validity()
{
    if (!m_validityState)
        m_validityState = ValidityState::create(this);

    return m_validityState.get();
}

void HTMLFormControlElement::parseMappedAttribute(MappedAttribute *attr)
{
    if (attr->name() == nameAttr) {
        // Do nothing.
    } else if (attr->name() == disabledAttr) {
        bool oldDisabled = m_disabled;
        m_disabled = !attr->isNull();
        if (oldDisabled != m_disabled) {
            setNeedsStyleRecalc();
            if (renderer() && renderer()->style()->hasAppearance())
                renderer()->theme()->stateChanged(renderer(), EnabledState);
        }
    } else if (attr->name() == readonlyAttr) {
        bool oldReadOnly = m_readOnly;
        m_readOnly = !attr->isNull();
        if (oldReadOnly != m_readOnly) {
            setNeedsStyleRecalc();
            if (renderer() && renderer()->style()->hasAppearance())
                renderer()->theme()->stateChanged(renderer(), ReadOnlyState);
        }
    } else
        HTMLElement::parseMappedAttribute(attr);
}

void HTMLFormControlElement::attach()
{
    ASSERT(!attached());

    HTMLElement::attach();

    // The call to updateFromElement() needs to go after the call through
    // to the base class's attach() because that can sometimes do a close
    // on the renderer.
    if (renderer())
        renderer()->updateFromElement();
        
    // Focus the element if it should honour its autofocus attribute.
    // We have to determine if the element is a TextArea/Input/Button/Select,
    // if input type hidden ignore autofocus. So if disabled or readonly.
    bool isInputTypeHidden = false;
    if (hasTagName(inputTag))
        isInputTypeHidden = static_cast<HTMLInputElement*>(this)->isInputTypeHidden();

    if (autofocus() && renderer() && !document()->ignoreAutofocus() && !isReadOnlyFormControl() &&
            ((hasTagName(inputTag) && !isInputTypeHidden) || hasTagName(selectTag) ||
              hasTagName(buttonTag) || hasTagName(textareaTag)))
         focus();
}

void HTMLFormControlElement::insertedIntoTree(bool deep)
{
    if (!m_form) {
        // This handles the case of a new form element being created by
        // JavaScript and inserted inside a form.  In the case of the parser
        // setting a form, we will already have a non-null value for m_form, 
        // and so we don't need to do anything.
        m_form = findFormAncestor();
        if (m_form)
            m_form->registerFormElement(this);
        else
            document()->checkedRadioButtons().addButton(this);
    }

    HTMLElement::insertedIntoTree(deep);
}

static inline Node* findRoot(Node* n)
{
    Node* root = n;
    for (; n; n = n->parentNode())
        root = n;
    return root;
}

void HTMLFormControlElement::removedFromTree(bool deep)
{
    // If the form and element are both in the same tree, preserve the connection to the form.
    // Otherwise, null out our form and remove ourselves from the form's list of elements.
    HTMLParser* parser = 0;
    if (Tokenizer* tokenizer = document()->tokenizer())
        if (tokenizer->isHTMLTokenizer())
            parser = static_cast<HTMLTokenizer*>(tokenizer)->htmlParser();
    
    if (m_form && !(parser && parser->isHandlingResidualStyleAcrossBlocks()) && findRoot(this) != findRoot(m_form)) {
        m_form->removeFormElement(this);
        m_form = 0;
    }

    HTMLElement::removedFromTree(deep);
}

const AtomicString& HTMLFormControlElement::formControlName() const
{
    const AtomicString& n = getAttribute(nameAttr);
    return n.isNull() ? emptyAtom : n;
}

void HTMLFormControlElement::setName(const AtomicString &value)
{
    setAttribute(nameAttr, value);
}

void HTMLFormControlElement::dispatchFormControlChangeEvent()
{
    dispatchEvent(eventNames().changeEvent, true, false);
}

bool HTMLFormControlElement::disabled() const
{
    return m_disabled;
}

void HTMLFormControlElement::setDisabled(bool b)
{
    setAttribute(disabledAttr, b ? "" : 0);
}

void HTMLFormControlElement::setReadOnly(bool b)
{
    setAttribute(readonlyAttr, b ? "" : 0);
}

bool HTMLFormControlElement::autofocus() const
{
    return hasAttribute(autofocusAttr);
}

void HTMLFormControlElement::setAutofocus(bool b)
{
    setAttribute(autofocusAttr, b ? "autofocus" : 0);
}

bool HTMLFormControlElement::required() const
{
    return hasAttribute(requiredAttr);
}

void HTMLFormControlElement::setRequired(bool b)
{
    setAttribute(requiredAttr, b ? "required" : 0);
}

void HTMLFormControlElement::recalcStyle(StyleChange change)
{
    HTMLElement::recalcStyle(change);

    if (renderer())
        renderer()->updateFromElement();
}

bool HTMLFormControlElement::isFocusable() const
{
    if (disabled() || !renderer() || 
        (renderer()->style() && renderer()->style()->visibility() != VISIBLE) || 
        !renderer()->isBox() || toRenderBox(renderer())->size().isEmpty())
        return false;
    return true;
}

bool HTMLFormControlElement::isKeyboardFocusable(KeyboardEvent* event) const
{
    if (isFocusable())
        if (document()->frame())
            return document()->frame()->eventHandler()->tabsToAllControls(event);
    return false;
}

bool HTMLFormControlElement::isMouseFocusable() const
{
#if PLATFORM(GTK)
    return HTMLElement::isMouseFocusable();
#else
    return false;
#endif
}

short HTMLFormControlElement::tabIndex() const
{
    // Skip the supportsFocus check in HTMLElement.
    return Element::tabIndex();
}

bool HTMLFormControlElement::willValidate() const
{
    // FIXME: Implementation shall be completed with these checks:
    //      The control does not have a repetition template as an ancestor.
    //      The control does not have a datalist element as an ancestor.
    //      The control is not an output element.
    return form() && name().length() && !disabled() && !isReadOnlyFormControl();
}

void HTMLFormControlElement::setCustomValidity(const String& error)
{
    validity()->setCustomErrorMessage(error);
}
    
void HTMLFormControlElement::dispatchFocusEvent()
{
    if (document()->frame() && document()->frame()->page())
        document()->frame()->page()->chrome()->client()->formDidFocus(this);

    HTMLElement::dispatchFocusEvent();
}

void HTMLFormControlElement::dispatchBlurEvent()
{
    if (document()->frame() && document()->frame()->page())
        document()->frame()->page()->chrome()->client()->formDidBlur(this);

    HTMLElement::dispatchBlurEvent();
}

bool HTMLFormControlElement::supportsFocus() const
{
    return isFocusable() || (!disabled() && !document()->haveStylesheetsLoaded());
}

HTMLFormElement* HTMLFormControlElement::virtualForm() const
{
    return m_form;
}

void HTMLFormControlElement::removeFromForm()
{
    if (!m_form)
        return;
    m_form->removeFormElement(this);
    m_form = 0;
}

HTMLFormControlElementWithState::HTMLFormControlElementWithState(const QualifiedName& tagName, Document* doc, HTMLFormElement* f)
    : HTMLFormControlElement(tagName, doc, f)
{
    document()->registerFormElementWithState(this);
}

HTMLFormControlElementWithState::~HTMLFormControlElementWithState()
{
    document()->unregisterFormElementWithState(this);
}

void HTMLFormControlElementWithState::willMoveToNewOwnerDocument()
{
    document()->unregisterFormElementWithState(this);
    HTMLFormControlElement::willMoveToNewOwnerDocument();
}

void HTMLFormControlElementWithState::didMoveToNewOwnerDocument()
{
    document()->registerFormElementWithState(this);
    HTMLFormControlElement::didMoveToNewOwnerDocument();
}

void HTMLFormControlElementWithState::finishParsingChildren()
{
    HTMLFormControlElement::finishParsingChildren();
    Document* doc = document();
    if (doc->hasStateForNewFormElements()) {
        String state;
        if (doc->takeStateForFormElement(name().impl(), type().impl(), state))
            restoreFormControlState(state);
    }
}

} // namespace Webcore
