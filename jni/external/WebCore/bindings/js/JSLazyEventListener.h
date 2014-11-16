/*
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2008, 2009 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef JSLazyEventListener_h
#define JSLazyEventListener_h

#include "JSEventListener.h"
#include "PlatformString.h"

namespace WebCore {

    class Node;

    class JSLazyEventListener : public JSEventListener {
    public:
        static PassRefPtr<JSLazyEventListener> create(const String& functionName, const String& eventParameterName, const String& code, JSDOMGlobalObject* globalObject, Node* node, int lineNumber)
        {
            return adoptRef(new JSLazyEventListener(functionName, eventParameterName, code, globalObject, node, lineNumber));
        }
        virtual ~JSLazyEventListener();

    private:
        JSLazyEventListener(const String& functionName, const String& eventParameterName, const String& code, JSDOMGlobalObject*, Node*, int lineNumber);

        virtual JSC::JSObject* jsFunction() const;
        virtual bool wasCreatedFromMarkup() const { return true; }

        void parseCode() const;

        mutable String m_functionName;
        mutable String m_eventParameterName;
        mutable String m_code;
        mutable bool m_parsed;
        int m_lineNumber;
        Node* m_originalNode;
    };

} // namespace WebCore

#endif // JSEventListener_h
