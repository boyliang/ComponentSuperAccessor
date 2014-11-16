/*
 * Copyright (C) 2006, 2007, 2008, 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef V8WorkerContextEventListener_h
#define V8WorkerContextEventListener_h

#if ENABLE(WORKERS)

#include "V8CustomEventListener.h"
#include <v8.h>
#include <wtf/PassRefPtr.h>

namespace WebCore {

    class Event;
    class WorkerContextExecutionProxy;

    class V8WorkerContextEventListener : public V8EventListener {
    public:
        static PassRefPtr<V8WorkerContextEventListener> create(WorkerContextExecutionProxy* proxy, v8::Local<v8::Object> listener, bool isInline)
        {
            return adoptRef(new V8WorkerContextEventListener(proxy, listener, isInline));
        }
        V8WorkerContextEventListener(WorkerContextExecutionProxy*, v8::Local<v8::Object> listener, bool isInline);

        virtual ~V8WorkerContextEventListener();
        virtual void handleEvent(Event*, bool isWindowEvent);
        virtual bool reportError(const String& message, const String& url, int lineNumber);
        virtual bool disconnected() const { return !m_proxy; }

        WorkerContextExecutionProxy* proxy() const { return m_proxy; }
        void disconnect() { m_proxy = 0; }

    private:
        virtual v8::Local<v8::Value> callListenerFunction(v8::Handle<v8::Value> jsEvent, Event*, bool isWindowEvent);
        v8::Local<v8::Object> getReceiverObject(Event*, bool isWindowEvent);
        WorkerContextExecutionProxy* m_proxy;
    };

} // namespace WebCore

#endif // WORKERS

#endif // V8WorkerContextEventListener_h
