/*
 * Copyright (C) 2007 Apple Inc.  All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef InspectorClientQt_h
#define InspectorClientQt_h

#include "InspectorClient.h"
#include "OwnPtr.h"
#include <QtCore/QString>

class QWebPage;

namespace WebCore {
    class Node;
    class Page;
    class String;
    class InspectorClientWebPage;

    class InspectorClientQt : public InspectorClient {
    public:
        InspectorClientQt(QWebPage*);

        virtual void inspectorDestroyed();

        virtual Page* createPage();

        virtual String localizedStringsURL();

        virtual String hiddenPanels();

        virtual void showWindow();
        virtual void closeWindow();
        virtual bool windowVisible();

        virtual void attachWindow();
        virtual void detachWindow();

        virtual void setAttachedWindowHeight(unsigned height);

        virtual void highlight(Node*);
        virtual void hideHighlight();
        virtual void inspectedURLChanged(const String& newURL);

        virtual void populateSetting(const String& key, InspectorController::Setting&);
        virtual void storeSetting(const String& key, const InspectorController::Setting&);
        virtual void removeSetting(const String& key);

        virtual void inspectorWindowObjectCleared();

    private:
        void updateWindowTitle();
        QWebPage* m_inspectedWebPage;
        OwnPtr<InspectorClientWebPage> m_webPage;
        QString m_inspectedURL;
    };
}

#endif
