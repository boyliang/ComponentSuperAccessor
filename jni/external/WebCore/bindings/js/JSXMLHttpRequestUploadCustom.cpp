/*
 * Copyright (C) 2008, 2009 Apple Inc. All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "JSXMLHttpRequestUpload.h"

#include "DOMWindow.h"
#include "Document.h"
#include "Event.h"
#include "Frame.h"
#include "JSDOMWindowCustom.h"
#include "JSEvent.h"
#include "JSEventListener.h"
#include "XMLHttpRequest.h"
#include "XMLHttpRequestUpload.h"
#include <runtime/Error.h>

using namespace JSC;

namespace WebCore {

void JSXMLHttpRequestUpload::markChildren(MarkStack& markStack)
{
    Base::markChildren(markStack);

    if (XMLHttpRequest* xmlHttpRequest = m_impl->associatedXMLHttpRequest()) {
        DOMObject* wrapper = getCachedDOMObjectWrapper(*Heap::heap(this)->globalData(), xmlHttpRequest);
        if (wrapper)
            markStack.append(wrapper);
    }

    markIfNotNull(markStack, m_impl->onabort());
    markIfNotNull(markStack, m_impl->onerror());
    markIfNotNull(markStack, m_impl->onload());
    markIfNotNull(markStack, m_impl->onloadstart());
    markIfNotNull(markStack, m_impl->onprogress());
    
    typedef XMLHttpRequestUpload::EventListenersMap EventListenersMap;
    typedef XMLHttpRequestUpload::ListenerVector ListenerVector;
    EventListenersMap& eventListeners = m_impl->eventListeners();
    for (EventListenersMap::iterator mapIter = eventListeners.begin(); mapIter != eventListeners.end(); ++mapIter) {
        for (ListenerVector::iterator vecIter = mapIter->second.begin(); vecIter != mapIter->second.end(); ++vecIter)
            (*vecIter)->markJSFunction(markStack);
    }
}

JSValue JSXMLHttpRequestUpload::addEventListener(ExecState* exec, const ArgList& args)
{
    JSDOMGlobalObject* globalObject = toJSDOMGlobalObject(impl()->scriptExecutionContext());
    if (!globalObject)
        return jsUndefined();
    RefPtr<JSEventListener> listener = globalObject->findOrCreateJSEventListener(args.at(1));
    if (!listener)
        return jsUndefined();
    impl()->addEventListener(args.at(0).toString(exec), listener.release(), args.at(2).toBoolean(exec));
    return jsUndefined();
}

JSValue JSXMLHttpRequestUpload::removeEventListener(ExecState* exec, const ArgList& args)
{
    JSDOMGlobalObject* globalObject = toJSDOMGlobalObject(impl()->scriptExecutionContext());
    if (!globalObject)
        return jsUndefined();
    JSEventListener* listener = globalObject->findJSEventListener(args.at(1));
    if (!listener)
        return jsUndefined();
    impl()->removeEventListener(args.at(0).toString(exec), listener, args.at(2).toBoolean(exec));
    return jsUndefined();
}

} // namespace WebCore
