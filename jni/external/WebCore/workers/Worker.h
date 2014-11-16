/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 */

#ifndef Worker_h
#define Worker_h

#if ENABLE(WORKERS)

#include "AbstractWorker.h"
#include "ActiveDOMObject.h"
#include "AtomicStringHash.h"
#include "EventListener.h"
#include "EventTarget.h"
#include "WorkerScriptLoaderClient.h"
#include <wtf/OwnPtr.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>

namespace WebCore {

    class ScriptExecutionContext;
    class String;
    class WorkerContextProxy;
    class WorkerScriptLoader;

    typedef int ExceptionCode;

    class Worker : public AbstractWorker, private WorkerScriptLoaderClient {
    public:
        static PassRefPtr<Worker> create(const String& url, ScriptExecutionContext* context, ExceptionCode& ec) { return adoptRef(new Worker(url, context, ec)); }
        ~Worker();

        virtual Worker* toWorker() { return this; }

        void postMessage(const String&, ExceptionCode&);
        void postMessage(const String&, MessagePort*, ExceptionCode&);

        void terminate();

        void dispatchMessage(const String&, PassRefPtr<MessagePort>);
        void dispatchErrorEvent();

        virtual bool canSuspend() const;
        virtual void stop();
        virtual bool hasPendingActivity() const;

        void setOnmessage(PassRefPtr<EventListener> eventListener) { m_onMessageListener = eventListener; }
        EventListener* onmessage() const { return m_onMessageListener.get(); }

    private:
        Worker(const String&, ScriptExecutionContext*, ExceptionCode&);

        virtual void notifyFinished();

        virtual void refEventTarget() { ref(); }
        virtual void derefEventTarget() { deref(); }

        OwnPtr<WorkerScriptLoader> m_scriptLoader;

        WorkerContextProxy* m_contextProxy; // The proxy outlives the worker to perform thread shutdown.

        RefPtr<EventListener> m_onMessageListener;
    };

} // namespace WebCore

#endif // ENABLE(WORKERS)

#endif // Worker_h
