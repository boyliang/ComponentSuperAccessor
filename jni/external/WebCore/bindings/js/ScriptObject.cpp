/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
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
#include "ScriptObject.h"

#include "JSDOMBinding.h"

#if ENABLE(JAVASCRIPT_DEBUGGER)
#include "JSInspectorBackend.h"
#endif

#include <runtime/JSLock.h>

using namespace JSC;

namespace WebCore {

ScriptObject::ScriptObject(ScriptState* scriptState, JSObject* object)
    : ScriptValue(object)
    , m_scriptState(scriptState)
{
}

static bool handleException(ScriptState* scriptState)
{
    if (!scriptState->hadException())
        return true;

    reportException(scriptState, scriptState->exception());
    return false;
}

bool ScriptObject::set(const String& name, const String& value)
{
    JSLock lock(SilenceAssertionsOnly);
    PutPropertySlot slot;
    jsObject()->put(m_scriptState, Identifier(m_scriptState, name), jsString(m_scriptState, value), slot);
    return handleException(m_scriptState);
}

bool ScriptObject::set(const char* name, const ScriptObject& value)
{
    JSLock lock(SilenceAssertionsOnly);
    PutPropertySlot slot;
    jsObject()->put(m_scriptState, Identifier(m_scriptState, name), value.jsObject(), slot);
    return handleException(m_scriptState);
}

bool ScriptObject::set(const char* name, const String& value)
{
    JSLock lock(SilenceAssertionsOnly);
    PutPropertySlot slot;
    jsObject()->put(m_scriptState, Identifier(m_scriptState, name), jsString(m_scriptState, value), slot);
    return handleException(m_scriptState);
}

bool ScriptObject::set(const char* name, double value)
{
    JSLock lock(SilenceAssertionsOnly);
    PutPropertySlot slot;
    jsObject()->put(m_scriptState, Identifier(m_scriptState, name), jsNumber(m_scriptState, value), slot);
    return handleException(m_scriptState);
}

bool ScriptObject::set(const char* name, long long value)
{
    JSLock lock(SilenceAssertionsOnly);
    PutPropertySlot slot;
    jsObject()->put(m_scriptState, Identifier(m_scriptState, name), jsNumber(m_scriptState, value), slot);
    return handleException(m_scriptState);
}

bool ScriptObject::set(const char* name, int value)
{
    JSLock lock(SilenceAssertionsOnly);
    PutPropertySlot slot;
    jsObject()->put(m_scriptState, Identifier(m_scriptState, name), jsNumber(m_scriptState, value), slot);
    return handleException(m_scriptState);
}

bool ScriptObject::set(const char* name, bool value)
{
    JSLock lock(SilenceAssertionsOnly);
    PutPropertySlot slot;
    jsObject()->put(m_scriptState, Identifier(m_scriptState, name), jsBoolean(value), slot);
    return handleException(m_scriptState);
}

ScriptObject ScriptObject::createNew(ScriptState* scriptState)
{
    JSLock lock(SilenceAssertionsOnly);
    return ScriptObject(scriptState, constructEmptyObject(scriptState));
}

bool ScriptGlobalObject::set(ScriptState* scriptState, const char* name, const ScriptObject& value)
{
    JSLock lock(SilenceAssertionsOnly);
    scriptState->lexicalGlobalObject()->putDirect(Identifier(scriptState, name), value.jsObject());
    return handleException(scriptState);
}

#if ENABLE(JAVASCRIPT_DEBUGGER)
bool ScriptGlobalObject::set(ScriptState* scriptState, const char* name, InspectorBackend* value)
{
    JSLock lock(SilenceAssertionsOnly);
    JSDOMGlobalObject* globalObject = static_cast<JSDOMGlobalObject*>(scriptState->lexicalGlobalObject());
    globalObject->putDirect(Identifier(scriptState, name), toJS(scriptState, globalObject, value));
    return handleException(scriptState);
}
#endif

bool ScriptGlobalObject::get(ScriptState* scriptState, const char* name, ScriptObject& value)
{
    JSLock lock(SilenceAssertionsOnly);
    JSValue jsValue = scriptState->lexicalGlobalObject()->get(scriptState, Identifier(scriptState, name));
    if (!jsValue)
        return false;

    if (!jsValue.isObject())
        return false;

    value = ScriptObject(scriptState, asObject(jsValue));
    return true;
}

bool ScriptGlobalObject::remove(ScriptState* scriptState, const char* name)
{
    JSLock lock(SilenceAssertionsOnly);
    scriptState->lexicalGlobalObject()->deleteProperty(scriptState, Identifier(scriptState, name));
    return handleException(scriptState);
}

} // namespace WebCore
