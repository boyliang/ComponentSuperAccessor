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

// TODO(andreip): upstream touch related changes to Chromium
#include "config.h"

#if ENABLE(TOUCH_EVENTS)
#include "TouchList.h"

#include "Touch.h"

#include "V8Binding.h"
#include "V8CustomBinding.h"
#include "V8DOMWrapper.h"

#include <wtf/RefPtr.h>

namespace WebCore {

INDEXED_PROPERTY_GETTER(TouchList)
{
    INC_STATS("DOM.TouchList.IndexedPropertyGetter");
    TouchList* imp = V8DOMWrapper::convertToNativeObject<TouchList>(V8ClassIndex::TOUCHLIST, info.Holder());
    RefPtr<Touch> result = imp->item(index);
    if (!result)
        return notHandledByInterceptor();

    return V8DOMWrapper::convertToV8Object(V8ClassIndex::TOUCH, result.get());
}
} // namespace WebCore

#endif // TOUCH_EVENTS
