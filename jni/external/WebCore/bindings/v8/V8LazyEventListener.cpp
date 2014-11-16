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

#include "config.h"
#include "V8LazyEventListener.h"

#include "Frame.h"
#include "V8Binding.h"
#include "V8Proxy.h"

namespace WebCore {

V8LazyEventListener::V8LazyEventListener(Frame *frame, const String& code, const String& functionName, bool isSVGEvent)
    : V8AbstractEventListener(frame, true)
    , m_code(code)
    , m_functionName(functionName)
    , m_isSVGEvent(isSVGEvent)
    , m_compiled(false)
    , m_wrappedFunctionCompiled(false)
{
}

V8LazyEventListener::~V8LazyEventListener()
{
    disposeListenerObject();

    // Dispose wrapped function
    if (!m_wrappedFunction.IsEmpty()) {
#ifndef NDEBUG
        V8GCController::unregisterGlobalHandle(this, m_wrappedFunction);
#endif
        m_wrappedFunction.Dispose();
        m_wrappedFunction.Clear();
    }
}

v8::Local<v8::Function> V8LazyEventListener::getListenerFunction()
{
    if (m_compiled) {
        ASSERT(m_listener.IsEmpty() || m_listener->IsFunction());
        return m_listener.IsEmpty() ? v8::Local<v8::Function>() : v8::Local<v8::Function>::New(v8::Persistent<v8::Function>::Cast(m_listener));
    }

    m_compiled = true;

    ASSERT(m_frame);

    {
        // Switch to the context of m_frame.
        v8::HandleScope handleScope;

        // Use the outer scope to hold context.
        v8::Handle<v8::Context> v8Context = V8Proxy::mainWorldContext(m_frame);
        // Bail out if we could not get the context.
        if (v8Context.IsEmpty())
            return v8::Local<v8::Function>();

        v8::Context::Scope scope(v8Context);

        // Wrap function around the event code.  The parenthesis around the function are needed so that evaluating the code yields
        // the function value.  Without the parenthesis the function value is thrown away.

        // Make it an anonymous function to avoid name conflict for cases like
        // <body onload='onload()'>
        // <script> function onload() { alert('hi'); } </script>.
        // Set function name to function object instead.
        // See issue 944690.
        //
        // The ECMAScript spec says (very obliquely) that the parameter to an event handler is named "evt".
        //
        // Don't use new lines so that lines in the modified handler
        // have the same numbers as in the original code.
        String code = "(function (evt) {";
        code.append(m_code);
        code.append("\n})");

        v8::Handle<v8::String> codeExternalString = v8ExternalString(code);
        v8::Handle<v8::Script> script = V8Proxy::compileScript(codeExternalString, m_frame->document()->url(), m_lineNumber - 1);
        if (!script.IsEmpty()) {
            V8Proxy* proxy = V8Proxy::retrieve(m_frame);
            ASSERT(proxy);
            v8::Local<v8::Value> value = proxy->runScript(script, false);
            if (!value.IsEmpty()) {
                ASSERT(value->IsFunction());
                v8::Local<v8::Function> listenerFunction = v8::Local<v8::Function>::Cast(value);
                listenerFunction->SetName(v8::String::New(fromWebCoreString(m_functionName), m_functionName.length()));

                m_listener = v8::Persistent<v8::Function>::New(listenerFunction);
#ifndef NDEBUG
                V8GCController::registerGlobalHandle(EVENT_LISTENER, this, m_listener);
#endif
            }
        }
    }

    ASSERT(m_listener.IsEmpty() || m_listener->IsFunction());
    return m_listener.IsEmpty() ? v8::Local<v8::Function>() : v8::Local<v8::Function>::New(v8::Persistent<v8::Function>::Cast(m_listener));
}

v8::Local<v8::Value> V8LazyEventListener::callListenerFunction(v8::Handle<v8::Value> jsEvent, Event* event, bool isWindowEvent)
{
    v8::Local<v8::Function> handlerFunction = getWrappedListenerFunction();
    v8::Local<v8::Object> receiver = getReceiverObject(event, isWindowEvent);
    if (handlerFunction.IsEmpty() || receiver.IsEmpty())
        return v8::Local<v8::Value>();

    v8::Handle<v8::Value> parameters[1] = { jsEvent };

    V8Proxy* proxy = V8Proxy::retrieve(m_frame);
    return proxy->callFunction(handlerFunction, receiver, 1, parameters);
}


static v8::Handle<v8::Value> V8LazyEventListenerToString(const v8::Arguments& args)
{
    return args.Callee()->GetHiddenValue(v8::String::New("toStringString"));
}


v8::Local<v8::Function> V8LazyEventListener::getWrappedListenerFunction()
{
    if (m_wrappedFunctionCompiled) {
        ASSERT(m_wrappedFunction.IsEmpty() || m_wrappedFunction->IsFunction());
        return m_wrappedFunction.IsEmpty() ? v8::Local<v8::Function>() : v8::Local<v8::Function>::New(m_wrappedFunction);
    }

    m_wrappedFunctionCompiled = true;

    {
        // Switch to the context of m_frame.
        v8::HandleScope handleScope;

        // Use the outer scope to hold context.
        v8::Handle<v8::Context> v8Context = V8Proxy::mainWorldContext(m_frame);
        // Bail out if we cannot get the context.
        if (v8Context.IsEmpty())
            return v8::Local<v8::Function>();

        v8::Context::Scope scope(v8Context);

        // FIXME: cache the wrapper function.

        // Nodes other than the document object, when executing inline event handlers push document, form, and the target node on the scope chain.
        // We do this by using 'with' statement.
        // See chrome/fast/forms/form-action.html
        //     chrome/fast/forms/selected-index-value.html
        //     base/fast/overflow/onscroll-layer-self-destruct.html
        //
        // Don't use new lines so that lines in the modified handler
        // have the same numbers as in the original code.
        String code = "(function (evt) {" \
                      "with (this.ownerDocument ? this.ownerDocument : {}) {" \
                      "with (this.form ? this.form : {}) {" \
                      "with (this) {" \
                      "return (function(evt){";
        code.append(m_code);
        // Insert '\n' otherwise //-style comments could break the handler.
        code.append(  "\n}).call(this, evt);}}}})");
        v8::Handle<v8::String> codeExternalString = v8ExternalString(code);
        v8::Handle<v8::Script> script = V8Proxy::compileScript(codeExternalString, m_frame->document()->url(), m_lineNumber);
        if (!script.IsEmpty()) {
            V8Proxy* proxy = V8Proxy::retrieve(m_frame);
            ASSERT(proxy);
            v8::Local<v8::Value> value = proxy->runScript(script, false);
            if (!value.IsEmpty()) {
                ASSERT(value->IsFunction());

                m_wrappedFunction = v8::Persistent<v8::Function>::New(v8::Local<v8::Function>::Cast(value));

                // Change the toString function on the wrapper function to avoid it returning the source for the actual wrapper function. Instead
                // it returns source for a clean wrapper function with the event argument wrapping the event source code. The reason for this
                // is that some web sites uses toString on event functions and the evals the source returned (some times a RegExp is applied as
                // well) for some other use. That fails miserably if the actual wrapper source is returned.
                v8::Local<v8::FunctionTemplate> toStringTemplate = v8::FunctionTemplate::New(V8LazyEventListenerToString);
                v8::Local<v8::Function> toStringFunction;
                if (!toStringTemplate.IsEmpty())
                    toStringFunction = toStringTemplate->GetFunction();
                if (!toStringFunction.IsEmpty()) {
                    String toStringResult = "function ";
                    toStringResult.append(m_functionName);
                    toStringResult.append("(");
                    if (m_isSVGEvent)
                        toStringResult.append("evt");
                    else
                        toStringResult.append("event");
                    toStringResult.append(") {\n  ");
                    toStringResult.append(m_code);
                    toStringResult.append("\n}");
                    toStringFunction->SetHiddenValue(v8::String::New("toStringString"), v8ExternalString(toStringResult));
                    m_wrappedFunction->Set(v8::String::New("toString"), toStringFunction);
                }

#ifndef NDEBUG
                V8GCController::registerGlobalHandle(EVENT_LISTENER, this, m_wrappedFunction);
#endif
                m_wrappedFunction->SetName(v8::String::New(fromWebCoreString(m_functionName), m_functionName.length()));
            }
        }
    }

    return v8::Local<v8::Function>::New(m_wrappedFunction);
}

} // namespace WebCore
