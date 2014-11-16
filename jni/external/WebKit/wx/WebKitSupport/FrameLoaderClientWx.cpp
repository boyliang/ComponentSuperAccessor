/*
 * Copyright (C) 2007 Kevin Ollivier <kevino@theolliviers.com>
 *
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
 */
 
#include "config.h"
#include "FrameLoaderClientWx.h"

#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/APICast.h>

#include "DocumentLoader.h"
#include "FormState.h"
#include "Frame.h"
#include "FrameLoaderTypes.h"
#include "FrameView.h"
#include "FrameTree.h"
#include "HTMLFormElement.h"
#include "HTMLFrameOwnerElement.h"
#include "NotImplemented.h"
#include "Page.h"
#include "PlatformString.h"
#include "ProgressTracker.h"
#include "RenderPart.h"
#include "ResourceError.h"
#include "ResourceResponse.h"
#include "ScriptController.h"
#include "ScriptString.h"

#include <stdio.h>

#include "WebFrame.h"
#include "WebView.h"
#include "WebViewPrivate.h"

namespace WebCore {

inline int wxNavTypeFromWebNavType(NavigationType type){
    if (type == NavigationTypeLinkClicked)
        return wxWEBVIEW_NAV_LINK_CLICKED;
    
    if (type == NavigationTypeFormSubmitted)
        return wxWEBVIEW_NAV_FORM_SUBMITTED;
        
    if (type == NavigationTypeBackForward)
        return wxWEBVIEW_NAV_BACK_NEXT;
        
    if (type == NavigationTypeReload)
        return wxWEBVIEW_NAV_RELOAD;
        
    if (type == NavigationTypeFormResubmitted)
        return wxWEBVIEW_NAV_FORM_RESUBMITTED;
        
    return wxWEBVIEW_NAV_OTHER;
}

FrameLoaderClientWx::FrameLoaderClientWx()
    : m_frame(0)
{
}


FrameLoaderClientWx::~FrameLoaderClientWx()
{
}

void FrameLoaderClientWx::setFrame(Frame *frame)
{
    m_frame = frame;
}

void FrameLoaderClientWx::setWebView(wxWebView *webview)
{
    m_webView = webview;
}

void FrameLoaderClientWx::detachFrameLoader()
{
    m_frame = 0;
}

bool FrameLoaderClientWx::hasWebView() const
{
    return m_webView != NULL;
}

bool FrameLoaderClientWx::hasBackForwardList() const
{
    notImplemented();
    return true;
}


void FrameLoaderClientWx::resetBackForwardList()
{
    notImplemented();
}


bool FrameLoaderClientWx::provisionalItemIsTarget() const
{
    notImplemented();
    return false;
}

void FrameLoaderClientWx::makeRepresentation(DocumentLoader*)
{
    notImplemented();
}


void FrameLoaderClientWx::forceLayout()
{
    notImplemented();
}


void FrameLoaderClientWx::forceLayoutForNonHTML()
{
    notImplemented();
}


void FrameLoaderClientWx::updateHistoryForCommit()
{
    notImplemented();
}


void FrameLoaderClientWx::updateHistoryForBackForwardNavigation()
{
    notImplemented();
}


void FrameLoaderClientWx::updateHistoryForReload()
{
    notImplemented();
}


void FrameLoaderClientWx::updateHistoryForStandardLoad()
{
    notImplemented();
}


void FrameLoaderClientWx::updateHistoryForInternalLoad()
{
    notImplemented();
}


void FrameLoaderClientWx::updateHistoryAfterClientRedirect()
{
    notImplemented();
}


void FrameLoaderClientWx::setCopiesOnScroll()
{
    // apparently mac specific
    notImplemented();
}


LoadErrorResetToken* FrameLoaderClientWx::tokenForLoadErrorReset()
{
    notImplemented();
    return 0;
}


void FrameLoaderClientWx::resetAfterLoadError(LoadErrorResetToken*)
{
    notImplemented();
}


void FrameLoaderClientWx::doNotResetAfterLoadError(LoadErrorResetToken*)
{
    notImplemented();
}


void FrameLoaderClientWx::willCloseDocument()
{
    notImplemented();
}


void FrameLoaderClientWx::detachedFromParent2()
{
    notImplemented();
}


void FrameLoaderClientWx::detachedFromParent3()
{
    notImplemented();
}

void FrameLoaderClientWx::dispatchDidHandleOnloadEvents()
{
    if (m_webView) {
        wxWebViewLoadEvent wkEvent(m_webView);
        wkEvent.SetState(wxWEBVIEW_LOAD_ONLOAD_HANDLED);
        wkEvent.SetURL(m_frame->loader()->documentLoader()->request().url().string());
        m_webView->GetEventHandler()->ProcessEvent(wkEvent);
    }
}


void FrameLoaderClientWx::dispatchDidReceiveServerRedirectForProvisionalLoad()
{
    notImplemented();
}


void FrameLoaderClientWx::dispatchDidCancelClientRedirect()
{
    notImplemented();
}


void FrameLoaderClientWx::dispatchWillPerformClientRedirect(const KURL&,
                                                            double interval,
                                                            double fireDate)
{
    notImplemented();
}


void FrameLoaderClientWx::dispatchDidChangeLocationWithinPage()
{
    notImplemented();
}


void FrameLoaderClientWx::dispatchWillClose()
{
    notImplemented();
}


void FrameLoaderClientWx::dispatchDidStartProvisionalLoad()
{
    if (m_webView) {
        wxWebViewLoadEvent wkEvent(m_webView);
        wkEvent.SetState(wxWEBVIEW_LOAD_NEGOTIATING);
        wkEvent.SetURL(m_frame->loader()->provisionalDocumentLoader()->request().url().string());
        m_webView->GetEventHandler()->ProcessEvent(wkEvent);
    }
}


void FrameLoaderClientWx::dispatchDidReceiveTitle(const String& title)
{
    if (m_webView) {
        m_webView->SetPageTitle(title);
        wxWebViewReceivedTitleEvent wkEvent(m_webView);
        wkEvent.SetTitle(title);
        m_webView->GetEventHandler()->ProcessEvent(wkEvent);
    }
}


void FrameLoaderClientWx::dispatchDidCommitLoad()
{
    if (m_webView) {
        wxWebViewLoadEvent wkEvent(m_webView);
        wkEvent.SetState(wxWEBVIEW_LOAD_TRANSFERRING);
        wkEvent.SetURL(m_frame->loader()->documentLoader()->request().url().string());
        m_webView->GetEventHandler()->ProcessEvent(wkEvent);
    }
}

void FrameLoaderClientWx::dispatchDidFinishDocumentLoad()
{
    if (m_webView) {
        wxWebViewLoadEvent wkEvent(m_webView);
        wkEvent.SetState(wxWEBVIEW_LOAD_DOC_COMPLETED);
        wkEvent.SetURL(m_frame->loader()->url().string());
        m_webView->GetEventHandler()->ProcessEvent(wkEvent);
    }
}

void FrameLoaderClientWx::dispatchDidFinishLoad()
{
    notImplemented();
}


void FrameLoaderClientWx::dispatchDidFirstLayout()
{
    notImplemented();
}

void FrameLoaderClientWx::dispatchDidFirstVisuallyNonEmptyLayout()
{
    notImplemented();
}

void FrameLoaderClientWx::dispatchShow()
{
    notImplemented();
}


void FrameLoaderClientWx::cancelPolicyCheck()
{
    notImplemented();
}


void FrameLoaderClientWx::dispatchWillSubmitForm(FramePolicyFunction function,
                                                 PassRefPtr<FormState>)
{
    // FIXME: Send an event to allow for alerts and cancellation
    if (!m_frame)
        return;
    (m_frame->loader()->*function)(PolicyUse);
}


void FrameLoaderClientWx::dispatchDidLoadMainResource(DocumentLoader*)
{
    notImplemented();
}


void FrameLoaderClientWx::revertToProvisionalState(DocumentLoader*)
{
    notImplemented();
}

void FrameLoaderClientWx::postProgressStartedNotification()
{
    notImplemented();
}

void FrameLoaderClientWx::postProgressEstimateChangedNotification()
{
    notImplemented();
}

void FrameLoaderClientWx::postProgressFinishedNotification()
{
    if (m_webView) {
        wxWebViewLoadEvent wkEvent(m_webView);
        wkEvent.SetState(wxWEBVIEW_LOAD_DL_COMPLETED);
        wkEvent.SetURL(m_frame->loader()->url().string());
        m_webView->GetEventHandler()->ProcessEvent(wkEvent);
    }
}

void FrameLoaderClientWx::progressStarted()
{
    notImplemented();
}


void FrameLoaderClientWx::progressCompleted()
{
    notImplemented();
}


void FrameLoaderClientWx::setMainFrameDocumentReady(bool b)
{
    notImplemented();
    // this is only interesting once we provide an external API for the DOM
}


void FrameLoaderClientWx::willChangeTitle(DocumentLoader*)
{
    notImplemented();
}


void FrameLoaderClientWx::didChangeTitle(DocumentLoader *l)
{
    setTitle(l->title(), l->url());
}


void FrameLoaderClientWx::finishedLoading(DocumentLoader*)
{
    notImplemented();
}


bool FrameLoaderClientWx::canShowMIMEType(const String& MIMEType) const
{
    notImplemented();
    return true;
}


bool FrameLoaderClientWx::representationExistsForURLScheme(const String& URLScheme) const
{
    notImplemented();
    return false;
}


String FrameLoaderClientWx::generatedMIMETypeForURLScheme(const String& URLScheme) const
{
    notImplemented();
    return String();
}


void FrameLoaderClientWx::frameLoadCompleted()
{
    notImplemented();
}

void FrameLoaderClientWx::saveViewStateToItem(HistoryItem*)
{
    notImplemented();
}

void FrameLoaderClientWx::restoreViewState()
{
    notImplemented();
}
        
void FrameLoaderClientWx::restoreScrollPositionAndViewState()
{
    notImplemented();
}


void FrameLoaderClientWx::provisionalLoadStarted()
{
    notImplemented();
}


bool FrameLoaderClientWx::shouldTreatURLAsSameAsCurrent(const KURL&) const
{
    notImplemented();
    return false;
}


void FrameLoaderClientWx::addHistoryItemForFragmentScroll()
{
    notImplemented();
}


void FrameLoaderClientWx::didFinishLoad()
{
    notImplemented();
}


void FrameLoaderClientWx::prepareForDataSourceReplacement()
{
    notImplemented();
}


void FrameLoaderClientWx::setTitle(const String& title, const KURL&)
{
    notImplemented();
}


String FrameLoaderClientWx::userAgent(const KURL&)
{
    // FIXME: Use the new APIs introduced by the GTK port to fill in these values.
    return String("Mozilla/5.0 (Macintosh; U; Intel Mac OS X; en) AppleWebKit/418.9.1 (KHTML, like Gecko) Safari/419.3");
}

void FrameLoaderClientWx::dispatchDidReceiveIcon()
{
    notImplemented();
}

void FrameLoaderClientWx::frameLoaderDestroyed()
{
    m_frame = 0;
    delete this;
}

bool FrameLoaderClientWx::canHandleRequest(const WebCore::ResourceRequest&) const
{
    notImplemented();
    return true;
}

void FrameLoaderClientWx::partClearedInBegin()
{
    notImplemented();
}

void FrameLoaderClientWx::updateGlobalHistory()
{
    notImplemented();
}

void FrameLoaderClientWx::updateGlobalHistoryRedirectLinks()
{
    notImplemented();
}

bool FrameLoaderClientWx::shouldGoToHistoryItem(WebCore::HistoryItem*) const
{
    notImplemented();
    return true;
}

void FrameLoaderClientWx::saveScrollPositionAndViewStateToItem(WebCore::HistoryItem*)
{
    notImplemented();
}

bool FrameLoaderClientWx::canCachePage() const
{
    return false;
}

void FrameLoaderClientWx::setMainDocumentError(WebCore::DocumentLoader*, const WebCore::ResourceError&)
{
    notImplemented();
}

void FrameLoaderClientWx::committedLoad(WebCore::DocumentLoader* loader, const char* data, int length)
{
    if (!m_frame)
        return;
    FrameLoader* fl = loader->frameLoader();
    fl->setEncoding(m_response.textEncodingName(), false);
    fl->addData(data, length);
}

WebCore::ResourceError FrameLoaderClientWx::cancelledError(const WebCore::ResourceRequest& request)
{
    notImplemented();
    return ResourceError(String(), WebKitErrorCannotShowURL, request.url().string(), String());
}

WebCore::ResourceError FrameLoaderClientWx::blockedError(const ResourceRequest& request)
{
    notImplemented();
    return ResourceError(String(), WebKitErrorCannotShowURL, request.url().string(), String());
}

WebCore::ResourceError FrameLoaderClientWx::cannotShowURLError(const WebCore::ResourceRequest& request)
{
    return ResourceError(String(), WebKitErrorCannotShowURL, request.url().string(), String());
}

WebCore::ResourceError FrameLoaderClientWx::interruptForPolicyChangeError(const WebCore::ResourceRequest& request)
{
    notImplemented();
    return ResourceError(String(), WebKitErrorFrameLoadInterruptedByPolicyChange, request.url().string(), String());
}

WebCore::ResourceError FrameLoaderClientWx::cannotShowMIMETypeError(const WebCore::ResourceResponse& response)
{
    notImplemented();
    return ResourceError(String(), WebKitErrorCannotShowMIMEType, response.url().string(), String());
}

WebCore::ResourceError FrameLoaderClientWx::fileDoesNotExistError(const WebCore::ResourceResponse& response)
{
    notImplemented();
    return ResourceError(String(), WebKitErrorCannotShowURL, response.url().string(), String());
}

bool FrameLoaderClientWx::shouldFallBack(const WebCore::ResourceError& error)
{
    notImplemented();
    return false;
}

WTF::PassRefPtr<DocumentLoader> FrameLoaderClientWx::createDocumentLoader(const ResourceRequest& request, const SubstituteData& substituteData)
{
    return DocumentLoader::create(request, substituteData);
}

void FrameLoaderClientWx::download(ResourceHandle*, const ResourceRequest&, const ResourceRequest&, const ResourceResponse&)
{
    notImplemented();
}

void FrameLoaderClientWx::assignIdentifierToInitialRequest(unsigned long identifier, DocumentLoader*, const ResourceRequest&)
{
    notImplemented();   
}

void FrameLoaderClientWx::dispatchWillSendRequest(DocumentLoader*, unsigned long, ResourceRequest& request, const ResourceResponse& response)
{
    notImplemented();
}

bool FrameLoaderClientWx::shouldUseCredentialStorage(DocumentLoader*, unsigned long)
{
    notImplemented();
    return false;
}

void FrameLoaderClientWx::dispatchDidReceiveAuthenticationChallenge(DocumentLoader*, unsigned long, const AuthenticationChallenge&)
{
    notImplemented();
}

void FrameLoaderClientWx::dispatchDidCancelAuthenticationChallenge(DocumentLoader*, unsigned long, const AuthenticationChallenge&)
{
    notImplemented();
}

void FrameLoaderClientWx::dispatchDidReceiveResponse(DocumentLoader* loader, unsigned long id, const ResourceResponse& response)
{
    notImplemented();
    m_response = response;
    m_firstData = true;
}

void FrameLoaderClientWx::dispatchDidReceiveContentLength(DocumentLoader* loader, unsigned long id, int length)
{
    notImplemented();
}

void FrameLoaderClientWx::dispatchDidFinishLoading(DocumentLoader*, unsigned long)
{
    notImplemented();
}

void FrameLoaderClientWx::dispatchDidFailLoading(DocumentLoader*, unsigned long, const ResourceError&)
{
    if (m_webView) {
        wxWebViewLoadEvent wkEvent(m_webView);
        wkEvent.SetState(wxWEBVIEW_LOAD_FAILED);
        wkEvent.SetURL(m_frame->loader()->documentLoader()->request().url().string());
        m_webView->GetEventHandler()->ProcessEvent(wkEvent);
    }
}

bool FrameLoaderClientWx::dispatchDidLoadResourceFromMemoryCache(DocumentLoader*, const ResourceRequest&, const ResourceResponse&, int)
{
    notImplemented();
    return false;
}

void FrameLoaderClientWx::dispatchDidLoadResourceByXMLHttpRequest(unsigned long, const ScriptString&)
{
  notImplemented();
}

void FrameLoaderClientWx::dispatchDidFailProvisionalLoad(const ResourceError&)
{
    notImplemented();
}

void FrameLoaderClientWx::dispatchDidFailLoad(const ResourceError&)
{
    notImplemented();
}

Frame* FrameLoaderClientWx::dispatchCreatePage()
{
    notImplemented();
    return false;
}

void FrameLoaderClientWx::dispatchDecidePolicyForMIMEType(FramePolicyFunction function, const String& mimetype, const ResourceRequest& request)
{
    if (!m_frame)
        return;
    
    notImplemented();
    (m_frame->loader()->*function)(PolicyUse);
}

void FrameLoaderClientWx::dispatchDecidePolicyForNewWindowAction(FramePolicyFunction function, const NavigationAction&, const ResourceRequest& request, PassRefPtr<FormState>, const String& targetName)
{
    if (!m_frame)
        return;

    if (m_webView) {
        wxWebViewNewWindowEvent wkEvent(m_webView);
        wkEvent.SetURL(request.url().string());
        wkEvent.SetTargetName(targetName);
        if (m_webView->GetEventHandler()->ProcessEvent(wkEvent)) {
            // if the app handles and doesn't skip the event, 
            // from WebKit's perspective treat it as blocked / ignored
            (m_frame->loader()->*function)(PolicyIgnore);
            return;
        }
    }
    
    (m_frame->loader()->*function)(PolicyUse);
}

void FrameLoaderClientWx::dispatchDecidePolicyForNavigationAction(FramePolicyFunction function, const NavigationAction& action, const ResourceRequest& request, PassRefPtr<FormState>)
{
    if (!m_frame)
        return;
        
    if (m_webView) {
        wxWebViewBeforeLoadEvent wkEvent(m_webView);
        wkEvent.SetNavigationType(wxNavTypeFromWebNavType(action.type()));
        wkEvent.SetURL(request.url().string());
        
        m_webView->GetEventHandler()->ProcessEvent(wkEvent);
        if (wkEvent.IsCancelled())
            (m_frame->loader()->*function)(PolicyIgnore);
        else
            (m_frame->loader()->*function)(PolicyUse);
        
    }
}

void FrameLoaderClientWx::dispatchUnableToImplementPolicy(const ResourceError&)
{
    notImplemented();
}

void FrameLoaderClientWx::startDownload(const ResourceRequest&)
{
    notImplemented();
}

PassRefPtr<Frame> FrameLoaderClientWx::createFrame(const KURL& url, const String& name, HTMLFrameOwnerElement* ownerElement,
                                   const String& referrer, bool allowsScrolling, int marginWidth, int marginHeight)
{
/*
    FIXME: Temporarily disabling code for loading subframes. While most 
    (i)frames load and are destroyed properly, the iframe created by
    google.com in its new homepage does not get destroyed when 
    document()->detach() is called, as other (i)frames do. It is destroyed on 
    app shutdown, but until that point, this 'in limbo' frame will do things
    like steal keyboard focus and crash when clicked on. (On some platforms,
    it is actually a visible object, even though it's not in a valid state.)
    
    Since just about every user is probably going to test against Google at 
    some point, I'm disabling this functionality until I have time to track down
    why it is not being destroyed.
*/

/*
    wxWindow* parent = m_webView;

    WebViewFrameData* data = new WebViewFrameData();
    data->name = name;
    data->ownerElement = ownerElement;
    data->url = url;
    data->referrer = referrer;
    data->marginWidth = marginWidth;
    data->marginHeight = marginHeight;

    wxWebView* newWin = new wxWebView(parent, -1, wxDefaultPosition, wxDefaultSize, data);

    RefPtr<Frame> childFrame = newWin->m_impl->frame;

    // FIXME: All of the below should probably be moved over into WebCore
    childFrame->tree()->setName(name);
    m_frame->tree()->appendChild(childFrame);
    // ### set override encoding if we have one

    FrameLoadType loadType = m_frame->loader()->loadType();
    FrameLoadType childLoadType = FrameLoadTypeInternal;

    childFrame->loader()->load(url, referrer, childLoadType,
                            String(), 0, 0);
    
    // The frame's onload handler may have removed it from the document.
    if (!childFrame->tree()->parent())
        return 0;
    
    delete data;
    
    return childFrame.get();
*/
    notImplemented();
    return 0;
}

ObjectContentType FrameLoaderClientWx::objectContentType(const KURL& url, const String& mimeType)
{
    notImplemented();
    return ObjectContentType();
}

PassRefPtr<Widget> FrameLoaderClientWx::createPlugin(const IntSize&, HTMLPlugInElement*, const KURL&, const Vector<String>&, const Vector<String>&, const String&, bool loadManually)
{
    notImplemented();
    return 0;
}

void FrameLoaderClientWx::redirectDataToPlugin(Widget* pluginWidget)
{
    notImplemented();
    return;
}

ResourceError FrameLoaderClientWx::pluginWillHandleLoadError(const ResourceResponse& response)
{
    notImplemented();
    return ResourceError(String(), WebKitErrorCannotLoadPlugIn, response.url().string(), String());
}

PassRefPtr<Widget> FrameLoaderClientWx::createJavaAppletWidget(const IntSize&, HTMLAppletElement*, const KURL& baseURL,
                                                    const Vector<String>& paramNames, const Vector<String>& paramValues)
{
    notImplemented();
    return 0;
}

String FrameLoaderClientWx::overrideMediaType() const
{
    notImplemented();
    return String();
}

void FrameLoaderClientWx::windowObjectCleared()
{
    if (m_webView) {
        wxWebViewWindowObjectClearedEvent wkEvent(m_webView);
        Frame* coreFrame = m_webView->GetMainFrame()->GetFrame();
        JSGlobalContextRef context = toGlobalRef(coreFrame->script()->globalObject()->globalExec());
        JSObjectRef windowObject = toRef(coreFrame->script()->globalObject());
        wkEvent.SetJSContext(context);
        wkEvent.SetWindowObject(windowObject);
        m_webView->GetEventHandler()->ProcessEvent(wkEvent);
    }
}

void FrameLoaderClientWx::documentElementAvailable()
{
}

void FrameLoaderClientWx::didPerformFirstNavigation() const
{
    notImplemented();
}

void FrameLoaderClientWx::registerForIconNotification(bool listen)
{
    notImplemented();
}

void FrameLoaderClientWx::savePlatformDataToCachedFrame(CachedFrame*)
{ 
    notImplemented();
}

void FrameLoaderClientWx::transitionToCommittedFromCachedFrame(CachedFrame*)
{ 
    notImplemented();
}

void FrameLoaderClientWx::transitionToCommittedForNewPage()
{ 
    ASSERT(m_frame);
    ASSERT(m_webView);
    
    Page* page = m_frame->page();
    ASSERT(page);

    bool isMainFrame = m_frame == page->mainFrame();

    m_frame->setView(0);

    RefPtr<FrameView> frameView;
    if (isMainFrame)
        frameView = FrameView::create(m_frame, IntRect(m_webView->GetRect()).size());
    else
        frameView = FrameView::create(m_frame);

    ASSERT(frameView);
    m_frame->setView(frameView);

    frameView->setPlatformWidget(m_webView);

    if (HTMLFrameOwnerElement* owner = m_frame->ownerElement())
        m_frame->view()->setScrollbarModes(owner->scrollingMode(), owner->scrollingMode());
}

}
