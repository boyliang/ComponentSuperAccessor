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

#include "config.h"
#include "MessagePort.h"

#include "AtomicString.h"
#include "DOMWindow.h"
#include "Document.h"
#include "EventException.h"
#include "EventNames.h"
#include "MessageEvent.h"
#include "SecurityOrigin.h"
#include "Timer.h"

namespace WebCore {

MessagePort::MessagePort(ScriptExecutionContext& scriptExecutionContext)
    : m_entangledChannel(0)
    , m_started(false)
    , m_scriptExecutionContext(&scriptExecutionContext)
{
    m_scriptExecutionContext->createdMessagePort(this);

    // Don't need to call processMessagePortMessagesSoon() here, because the port will not be opened until start() is invoked.
}

MessagePort::~MessagePort()
{
    close();
    if (m_scriptExecutionContext)
        m_scriptExecutionContext->destroyedMessagePort(this);
}

void MessagePort::postMessage(const String& message, ExceptionCode& ec)
{
    postMessage(message, 0, ec);
}

void MessagePort::postMessage(const String& message, MessagePort* dataPort, ExceptionCode& ec)
{
    if (!m_entangledChannel)
        return;
    ASSERT(m_scriptExecutionContext);

    OwnPtr<MessagePortChannel> channel;
    if (dataPort) {
        if (dataPort == this || m_entangledChannel->isConnectedTo(dataPort)) {
            ec = INVALID_STATE_ERR;
            return;
        }
        channel = dataPort->disentangle(ec);
        if (ec)
            return;
    }
    m_entangledChannel->postMessageToRemote(MessagePortChannel::EventData::create(message, channel.release()));
}

PassOwnPtr<MessagePortChannel> MessagePort::disentangle(ExceptionCode& ec)
{
    if (!m_entangledChannel)
        ec = INVALID_STATE_ERR;
    else {
        m_entangledChannel->disentangle();

        // We can't receive any messages or generate any events, so remove ourselves from the list of active ports.
        ASSERT(m_scriptExecutionContext);
        m_scriptExecutionContext->destroyedMessagePort(this);
        m_scriptExecutionContext = 0;
    }
    return m_entangledChannel.release();
}

// Invoked to notify us that there are messages available for this port.
// This code may be called from another thread, and so should not call any non-threadsafe APIs (i.e. should not call into the entangled channel or access mutable variables).
void MessagePort::messageAvailable()
{
    ASSERT(m_scriptExecutionContext);
    m_scriptExecutionContext->processMessagePortMessagesSoon();
}

void MessagePort::start()
{
    // Do nothing if we've been cloned
    if (!m_entangledChannel)
        return;

    ASSERT(m_scriptExecutionContext);
    if (m_started)
        return;

    m_started = true;
    m_scriptExecutionContext->processMessagePortMessagesSoon();
}

void MessagePort::close()
{
    if (!m_entangledChannel)
        return;
    m_entangledChannel->close();
}

void MessagePort::entangle(PassOwnPtr<MessagePortChannel> remote)
{
    // Only invoked to set our initial entanglement.
    ASSERT(!m_entangledChannel);
    ASSERT(m_scriptExecutionContext);

    // Don't entangle the ports if the channel is closed.
    if (remote->entangleIfOpen(this))
        m_entangledChannel = remote;
}

void MessagePort::contextDestroyed()
{
    ASSERT(m_scriptExecutionContext);
    // Must close port before blowing away the cached context, to ensure that we get no more calls to messageAvailable().
    close();
    m_scriptExecutionContext = 0;
}

ScriptExecutionContext* MessagePort::scriptExecutionContext() const
{
    return m_scriptExecutionContext;
}

void MessagePort::dispatchMessages()
{
    // Messages for contexts that are not fully active get dispatched too, but JSAbstractEventListener::handleEvent() doesn't call handlers for these.
    // The HTML5 spec specifies that any messages sent to a document that is not fully active should be dropped, so this behavior is OK.
    ASSERT(started());

    OwnPtr<MessagePortChannel::EventData> eventData;
    while (m_entangledChannel && m_entangledChannel->tryGetMessageFromRemote(eventData)) {
        RefPtr<MessagePort> port;
        OwnPtr<MessagePortChannel> channel = eventData->channel();
        if (channel) {
            // The remote side sent over a MessagePortChannel, so create a MessagePort to wrap it.
            port = MessagePort::create(*m_scriptExecutionContext);
            port->entangle(channel.release());
        }
        RefPtr<Event> evt = MessageEvent::create(eventData->message(), "", "", 0, port.release());

        if (m_onMessageListener) {
            evt->setTarget(this);
            evt->setCurrentTarget(this);
            m_onMessageListener->handleEvent(evt.get(), false);
        }

        ExceptionCode ec = 0;
        dispatchEvent(evt.release(), ec);
        ASSERT(!ec);
    }
}

void MessagePort::addEventListener(const AtomicString& eventType, PassRefPtr<EventListener> eventListener, bool)
{
    EventListenersMap::iterator iter = m_eventListeners.find(eventType);
    if (iter == m_eventListeners.end()) {
        ListenerVector listeners;
        listeners.append(eventListener);
        m_eventListeners.add(eventType, listeners);
    } else {
        ListenerVector& listeners = iter->second;
        for (ListenerVector::iterator listenerIter = listeners.begin(); listenerIter != listeners.end(); ++listenerIter) {
            if (*listenerIter == eventListener)
                return;
        }
        
        listeners.append(eventListener);
        m_eventListeners.add(eventType, listeners);
    }    
}

void MessagePort::removeEventListener(const AtomicString& eventType, EventListener* eventListener, bool)
{
    EventListenersMap::iterator iter = m_eventListeners.find(eventType);
    if (iter == m_eventListeners.end())
        return;
    
    ListenerVector& listeners = iter->second;
    for (ListenerVector::const_iterator listenerIter = listeners.begin(); listenerIter != listeners.end(); ++listenerIter) {
        if (*listenerIter == eventListener) {
            listeners.remove(listenerIter - listeners.begin());
            return;
        }
    }
}

bool MessagePort::dispatchEvent(PassRefPtr<Event> event, ExceptionCode& ec)
{
    if (!event || event->type().isEmpty()) {
        ec = EventException::UNSPECIFIED_EVENT_TYPE_ERR;
        return true;
    }
    
    ListenerVector listenersCopy = m_eventListeners.get(event->type());
    for (ListenerVector::const_iterator listenerIter = listenersCopy.begin(); listenerIter != listenersCopy.end(); ++listenerIter) {
        event->setTarget(this);
        event->setCurrentTarget(this);
        listenerIter->get()->handleEvent(event.get(), false);
    }
    
    return !event->defaultPrevented();
}

void MessagePort::setOnmessage(PassRefPtr<EventListener> eventListener)
{
    m_onMessageListener = eventListener;
    start();
}

bool MessagePort::hasPendingActivity()
{
    // The spec says that entangled message ports should always be treated as if they have a strong reference.
    // We'll also stipulate that the queue needs to be open (if the app drops its reference to the port before start()-ing it, then it's not really entangled as it's unreachable).
    return m_started && m_entangledChannel && m_entangledChannel->hasPendingActivity();
}

MessagePort* MessagePort::locallyEntangledPort()
{
    return m_entangledChannel ? m_entangledChannel->locallyEntangledPort(m_scriptExecutionContext) : 0;
}

} // namespace WebCore
