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
#include "JSDOMApplicationCache.h"

#if ENABLE(OFFLINE_WEB_APPLICATIONS)

#include "AtomicString.h"
#include "DOMApplicationCache.h"
#include "DOMWindow.h"
#include "Event.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "JSDOMWindowCustom.h"
#include "JSEvent.h"
#include "JSEventListener.h"

using namespace JSC;

namespace WebCore {

void JSDOMApplicationCache::markChildren(MarkStack& markStack)
{
    Base::markChildren(markStack);

    markIfNotNull(markStack, m_impl->onchecking());
    markIfNotNull(markStack, m_impl->onerror());
    markIfNotNull(markStack, m_impl->onnoupdate());
    markIfNotNull(markStack, m_impl->ondownloading());
    markIfNotNull(markStack, m_impl->onprogress());
    markIfNotNull(markStack, m_impl->onupdateready());
    markIfNotNull(markStack, m_impl->oncached());
    markIfNotNull(markStack, m_impl->onobsolete());

    typedef DOMApplicationCache::EventListenersMap EventListenersMap;
    typedef DOMApplicationCache::ListenerVector ListenerVector;
    EventListenersMap& eventListeners = m_impl->eventListeners();
    for (EventListenersMap::iterator mapIter = eventListeners.begin(); mapIter != eventListeners.end(); ++mapIter) {
        for (ListenerVector::iterator vecIter = mapIter->second.begin(); vecIter != mapIter->second.end(); ++vecIter)
            (*vecIter)->markJSFunction(markStack);
    }
}

#if ENABLE(APPLICATION_CACHE_DYNAMIC_ENTRIES)

JSValue JSDOMApplicationCache::hasItem(ExecState* exec, const ArgList& args)
{
    Frame* frame = asJSDOMWindow(exec->dynamicGlobalObject())->impl()->frame();
    if (!frame)
        return jsUndefined();
    const KURL& url = frame->loader()->completeURL(args.at(0).toString(exec));

    ExceptionCode ec = 0;
    bool result = impl()->hasItem(url, ec);
    setDOMException(exec, ec);
    return jsBoolean(result);
}

JSValue JSDOMApplicationCache::add(ExecState* exec, const ArgList& args)
{
    Frame* frame = asJSDOMWindow(exec->dynamicGlobalObject())->impl()->frame();
    if (!frame)
        return jsUndefined();
    const KURL& url = frame->loader()->completeURL(args.at(0).toString(exec));
    
    ExceptionCode ec = 0;
    impl()->add(url, ec);
    setDOMException(exec, ec);
    return jsUndefined();
}

JSValue JSDOMApplicationCache::remove(ExecState* exec, const ArgList& args)
{
    Frame* frame = asJSDOMWindow(exec->dynamicGlobalObject())->impl()->frame();
    if (!frame)
        return jsUndefined();
    const KURL& url = frame->loader()->completeURL(args.at(0).toString(exec));
    
    ExceptionCode ec = 0;
    impl()->remove(url, ec);
    setDOMException(exec, ec);
    return jsUndefined();
}

#endif // ENABLE(APPLICATION_CACHE_DYNAMIC_ENTRIES)

JSValue JSDOMApplicationCache::addEventListener(ExecState* exec, const ArgList& args)
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

JSValue JSDOMApplicationCache::removeEventListener(ExecState* exec, const ArgList& args)
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

#endif // ENABLE(OFFLINE_WEB_APPLICATIONS)
