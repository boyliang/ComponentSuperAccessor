/*
 * Copyright (C) 2008, 2009 Daniel Bates (dbates@intudata.com)
 * All rights reserved.
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
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef XSSAuditor_h
#define XSSAuditor_h

#include "PlatformString.h"
#include "TextEncoding.h"

namespace WebCore {

    class Frame;
    class ScriptSourceCode;

    // The XSSAuditor class is used to prevent type 1 cross-site scripting
    // vulnerabilites (also known as reflected vulnerabilities).
    //
    // More specifically, the XSSAuditor class decides whether the execution of
    // a script is to be allowed or denied based on the content of any
    // user-submitted data, including:
    //
    // * the query string of the URL.
    // * the HTTP-POST data.
    //
    // If the source code of a script resembles any user-submitted data then it
    // is denied execution.
    //
    // When you instantiate the XSSAuditor you must specify the {@link Frame}
    // of the page that you wish to audit.
    //
    // Bindings
    //
    // An XSSAuditor is instantiated within the contructor of a
    // ScriptController object and passed the Frame the script originated. The
    // ScriptController calls back to the XSSAuditor to determine whether a
    // JavaScript script is safe to execute before executing it. The following
    // methods call into XSSAuditor:
    //
    // * ScriptController::evaluate - used to evaluate JavaScript scripts.
    // * ScriptController::createInlineEventListener - used to create JavaScript event handlers.
    // * HTMLTokenizer::scriptHandler - used to load external JavaScript scripts.
    //
    class XSSAuditor {
    public:
        XSSAuditor(Frame*);
        ~XSSAuditor();

        bool isEnabled() const;

        // Determines whether the script should be allowed or denied execution
        // based on the content of any user-submitted data.
        bool canEvaluate(const String& code) const;

        // Determines whether the JavaScript URL should be allowed or denied execution
        // based on the content of any user-submitted data.
        bool canEvaluateJavaScriptURL(const String& code) const;

        // Determines whether the event listener should be created based on the
        // content of any user-submitted data.
        bool canCreateInlineEventListener(const String& functionName, const String& code) const;

        // Determines whether the external script should be loaded based on the
        // content of any user-submitted data.
        bool canLoadExternalScriptFromSrc(const String& context, const String& url) const;

        // Determines whether object should be loaded based on the content of
        // any user-submitted data.
        //
        // This method is called by FrameLoader::requestObject.
        bool canLoadObject(const String& url) const;

        // Determines whether the base URL should be changed based on the content
        // of any user-submitted data.
        //
        // This method is called by HTMLBaseElement::process.
        bool canSetBaseElementURL(const String& url) const;

    private:
        static String canonicalize(const String&);
        
        static String decodeURL(const String& url, const TextEncoding& encoding = UTF8Encoding(), bool decodeHTMLentities = true);
        
        static String decodeHTMLEntities(const String&, bool leaveUndecodableHTMLEntitiesUntouched = true);

        bool findInRequest(const String&, bool decodeHTMLentities = true) const;

        bool findInRequest(Frame*, const String&, bool decodeHTMLentities = true) const;

        // The frame to audit.
        Frame* m_frame;
    };

} // namespace WebCore

#endif // XSSAuditor_h
