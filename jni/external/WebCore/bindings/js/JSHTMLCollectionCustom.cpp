/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
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
#include "JSHTMLCollection.h"

#include "AtomicString.h"
#include "HTMLCollection.h"
#include "HTMLOptionsCollection.h"
#include "JSHTMLAllCollection.h"
#include "JSHTMLOptionsCollection.h"
#include "JSNamedNodesCollection.h"
#include "JSNode.h"
#include "Node.h"
#include "JSDOMBinding.h"
#include <wtf/Vector.h>

using namespace JSC;

namespace WebCore {

static JSValue getNamedItems(ExecState* exec, JSHTMLCollection* collection, const Identifier& propertyName)
{
    Vector<RefPtr<Node> > namedItems;
    collection->impl()->namedItems(propertyName, namedItems);

    if (namedItems.isEmpty())
        return jsUndefined();

    if (namedItems.size() == 1)
        return toJS(exec, collection->globalObject(), namedItems[0].get());

    return new (exec) JSNamedNodesCollection(exec, collection->globalObject(), namedItems);
}

// HTMLCollections are strange objects, they support both get and call,
// so that document.forms.item(0) and document.forms(0) both work.
static JSValue JSC_HOST_CALL callHTMLCollection(ExecState* exec, JSObject* function, JSValue, const ArgList& args)
{
    if (args.size() < 1)
        return jsUndefined();

    // Do not use thisObj here. It can be the JSHTMLDocument, in the document.forms(i) case.
    JSHTMLCollection* jsCollection = static_cast<JSHTMLCollection*>(function);
    HTMLCollection* collection = jsCollection->impl();

    // Also, do we need the TypeError test here ?

    if (args.size() == 1) {
        // Support for document.all(<index>) etc.
        bool ok;
        UString string = args.at(0).toString(exec);
        unsigned index = string.toUInt32(&ok, false);
        if (ok)
            return toJS(exec, jsCollection->globalObject(), collection->item(index));

        // Support for document.images('<name>') etc.
        return getNamedItems(exec, jsCollection, Identifier(exec, string));
    }

    // The second arg, if set, is the index of the item we want
    bool ok;
    UString string = args.at(0).toString(exec);
    unsigned index = args.at(1).toString(exec).toUInt32(&ok, false);
    if (ok) {
        String pstr = string;
        Node* node = collection->namedItem(pstr);
        while (node) {
            if (!index)
                return toJS(exec, jsCollection->globalObject(), node);
            node = collection->nextNamedItem(pstr);
            --index;
        }
    }

    return jsUndefined();
}

CallType JSHTMLCollection::getCallData(CallData& callData)
{
    callData.native.function = callHTMLCollection;
    return CallTypeHost;
}

bool JSHTMLCollection::canGetItemsForName(ExecState*, HTMLCollection* collection, const Identifier& propertyName)
{
    Vector<RefPtr<Node> > namedItems;
    collection->namedItems(propertyName, namedItems);
    return !namedItems.isEmpty();
}

JSValue JSHTMLCollection::nameGetter(ExecState* exec, const Identifier& propertyName, const PropertySlot& slot)
{
    JSHTMLCollection* thisObj = static_cast<JSHTMLCollection*>(asObject(slot.slotBase()));
    return getNamedItems(exec, thisObj, propertyName);
}

JSValue JSHTMLCollection::item(ExecState* exec, const ArgList& args)
{
    bool ok;
    uint32_t index = args.at(0).toString(exec).toUInt32(&ok, false);
    if (ok)
        return toJS(exec, globalObject(), impl()->item(index));
    return getNamedItems(exec, this, Identifier(exec, args.at(0).toString(exec)));
}

JSValue JSHTMLCollection::namedItem(ExecState* exec, const ArgList& args)
{
    return getNamedItems(exec, this, Identifier(exec, args.at(0).toString(exec)));
}

JSValue toJS(ExecState* exec, JSDOMGlobalObject* globalObject, HTMLCollection* collection)
{
    if (!collection)
        return jsNull();

    DOMObject* wrapper = getCachedDOMObjectWrapper(exec->globalData(), collection);

    if (wrapper)
        return wrapper;

    switch (collection->type()) {
        case SelectOptions:
            wrapper = CREATE_DOM_OBJECT_WRAPPER(exec, globalObject, HTMLOptionsCollection, collection);
            break;
        case DocAll:
            typedef HTMLCollection HTMLAllCollection;
            wrapper = CREATE_DOM_OBJECT_WRAPPER(exec, globalObject, HTMLAllCollection, collection);
            break;
        default:
            wrapper = CREATE_DOM_OBJECT_WRAPPER(exec, globalObject, HTMLCollection, collection);
            break;
    }

    return wrapper;
}

} // namespace WebCore
