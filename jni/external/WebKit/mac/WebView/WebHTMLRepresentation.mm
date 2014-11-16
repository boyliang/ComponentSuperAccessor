/*
 * Copyright (C) 2005, 2006, 2007 Apple Inc. All rights reserved.
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

#import "WebHTMLRepresentation.h"

#import "DOMElementInternal.h"
#import "DOMRangeInternal.h"
#import "WebArchive.h"
#import "WebBasePluginPackage.h"
#import "WebDataSourceInternal.h"
#import "WebDocumentPrivate.h"
#import "WebFrameInternal.h"
#import "WebKitNSStringExtras.h"
#import "WebKitStatisticsPrivate.h"
#import "WebNSAttributedStringExtras.h"
#import "WebNSObjectExtras.h"
#import "WebView.h"
#import <Foundation/NSURLResponse.h>
#import <WebCore/Document.h>
#import <WebCore/DocumentLoader.h>
#import <WebCore/Frame.h>
#import <WebCore/FrameLoader.h>
#import <WebCore/FrameLoaderClient.h>
#import <WebCore/HTMLFormControlElement.h>
#import <WebCore/HTMLFormElement.h>
#import <WebCore/HTMLInputElement.h>
#import <WebCore/HTMLNames.h>
#import <WebCore/MIMETypeRegistry.h>
#import <WebCore/Range.h>
#import <WebCore/TextResourceDecoder.h>
#import <WebKit/DOMHTMLInputElement.h>
#import <wtf/Assertions.h>
#import <wtf/StdLibExtras.h>

using namespace WebCore;
using namespace HTMLNames;

@interface WebHTMLRepresentationPrivate : NSObject {
@public
    WebDataSource *dataSource;
    
    BOOL hasSentResponseToPlugin;
    id <WebPluginManualLoader> manualLoader;
    NSView *pluginView;
}
@end

@implementation WebHTMLRepresentationPrivate
@end

@implementation WebHTMLRepresentation

static NSArray *stringArray(const HashSet<String>& set)
{
    NSMutableArray *array = [NSMutableArray arrayWithCapacity:set.size()];
    HashSet<String>::const_iterator end = set.end();
    for (HashSet<String>::const_iterator it = set.begin(); it != end; ++it)
        [array addObject:(NSString *)(*it)];
    return array;
}

static NSArray *concatenateArrays(NSArray *first, NSArray *second)
{
    NSMutableArray *result = [[first mutableCopy] autorelease];
    [result addObjectsFromArray:second];
    return result;
}

+ (NSArray *)supportedMIMETypes
{
    DEFINE_STATIC_LOCAL(RetainPtr<NSArray>, staticSupportedMIMETypes, (concatenateArrays([self supportedNonImageMIMETypes], [self supportedImageMIMETypes])));
    return staticSupportedMIMETypes.get();
}

+ (NSArray *)supportedNonImageMIMETypes
{
    DEFINE_STATIC_LOCAL(RetainPtr<NSArray>, staticSupportedNonImageMIMETypes, (stringArray(MIMETypeRegistry::getSupportedNonImageMIMETypes())));
    return staticSupportedNonImageMIMETypes.get();
}

+ (NSArray *)supportedImageMIMETypes
{
    DEFINE_STATIC_LOCAL(RetainPtr<NSArray>, staticSupportedImageMIMETypes, (stringArray(MIMETypeRegistry::getSupportedImageMIMETypes())));
    return staticSupportedImageMIMETypes.get();
}

- init
{
    self = [super init];
    if (!self)
        return nil;
    
    _private = [[WebHTMLRepresentationPrivate alloc] init];
    
    ++WebHTMLRepresentationCount;
    
    return self;
}

- (void)dealloc
{
    --WebHTMLRepresentationCount;
    
    [_private release];

    [super dealloc];
}

- (void)finalize
{
    --WebHTMLRepresentationCount;

    [super finalize];
}

- (void)_redirectDataToManualLoader:(id<WebPluginManualLoader>)manualLoader forPluginView:(NSView *)pluginView
{
    _private->manualLoader = manualLoader;
    _private->pluginView = pluginView;
}

- (void)setDataSource:(WebDataSource *)dataSource
{
    _private->dataSource = dataSource;
}

- (BOOL)_isDisplayingWebArchive
{
    return [[_private->dataSource _responseMIMEType] _webkit_isCaseInsensitiveEqualToString:@"application/x-webarchive"];
}

- (void)receivedData:(NSData *)data withDataSource:(WebDataSource *)dataSource
{
    WebFrame *webFrame = [dataSource webFrame];
    if (webFrame) {
        if (!_private->pluginView)
            [webFrame _receivedData:data textEncodingName:[[_private->dataSource response] textEncodingName]];
        
        // If the document is a stand-alone media document, now is the right time to cancel the WebKit load
        Frame* coreFrame = core(webFrame);
        if (coreFrame->document() && coreFrame->document()->isMediaDocument())
            coreFrame->loader()->documentLoader()->cancelMainResourceLoad(coreFrame->loader()->client()->pluginWillHandleLoadError(coreFrame->loader()->documentLoader()->response()));

        if (_private->pluginView) {
            if (!_private->hasSentResponseToPlugin) {
                [_private->manualLoader pluginView:_private->pluginView receivedResponse:[dataSource response]];
                _private->hasSentResponseToPlugin = YES;
            }
            
            [_private->manualLoader pluginView:_private->pluginView receivedData:data];
        }
    }
}

- (void)receivedError:(NSError *)error withDataSource:(WebDataSource *)dataSource
{
    if (_private->pluginView) {
        [_private->manualLoader pluginView:_private->pluginView receivedError:error];
    }
}

- (void)finishedLoadingWithDataSource:(WebDataSource *)dataSource
{
    WebFrame *frame = [dataSource webFrame];

    if (_private->pluginView) {
        [_private->manualLoader pluginViewFinishedLoading:_private->pluginView];
        return;
    }

    if (frame) {
        if (![self _isDisplayingWebArchive]) {
            // Telling the frame we received some data and passing nil as the data is our
            // way to get work done that is normally done when the first bit of data is
            // received, even for the case of a document with no data (like about:blank).
            [frame _receivedData:nil textEncodingName:[[_private->dataSource response] textEncodingName]];
        }
        
        WebView *webView = [frame webView];
        if ([webView isEditable])
            core(frame)->applyEditingStyleToBodyElement();
    }
}

- (BOOL)canProvideDocumentSource
{
    return [[_private->dataSource webFrame] _canProvideDocumentSource];
}

- (BOOL)canSaveAsWebArchive
{
    return [[_private->dataSource webFrame] _canSaveAsWebArchive];
}

- (NSString *)documentSource
{
    if ([self _isDisplayingWebArchive]) {            
        SharedBuffer *parsedArchiveData = [_private->dataSource _documentLoader]->parsedArchiveData();
        NSData *nsData = parsedArchiveData ? parsedArchiveData->createNSData() : nil;
        NSString *result = [[NSString alloc] initWithData:nsData encoding:NSUTF8StringEncoding];
        [nsData release];
        return [result autorelease];
    }

    Frame* coreFrame = core([_private->dataSource webFrame]);
    if (!coreFrame)
        return nil;
    Document* document = coreFrame->document();
    if (!document)
        return nil;
    TextResourceDecoder* decoder = document->decoder();
    if (!decoder)
        return nil;
    NSData *data = [_private->dataSource data];
    if (!data)
        return nil;
    return decoder->encoding().decode(reinterpret_cast<const char*>([data bytes]), [data length]);
}

- (NSString *)title
{
    return nsStringNilIfEmpty([_private->dataSource _documentLoader]->title());
}

- (DOMDocument *)DOMDocument
{
    return [[_private->dataSource webFrame] DOMDocument];
}

- (NSAttributedString *)attributedText
{
    // FIXME: Implement
    return nil;
}

- (NSAttributedString *)attributedStringFrom:(DOMNode *)startNode startOffset:(int)startOffset to:(DOMNode *)endNode endOffset:(int)endOffset
{
    return [NSAttributedString _web_attributedStringFromRange:Range::create(core(startNode)->document(), core(startNode), startOffset, core(endNode), endOffset).get()];
}

static HTMLFormElement* formElementFromDOMElement(DOMElement *element)
{
    Element* node = core(element);
    return node && node->hasTagName(formTag) ? static_cast<HTMLFormElement*>(node) : 0;
}

- (DOMElement *)elementWithName:(NSString *)name inForm:(DOMElement *)form
{
    HTMLFormElement* formElement = formElementFromDOMElement(form);
    if (!formElement)
        return nil;
    Vector<HTMLFormControlElement*>& elements = formElement->formElements;
    AtomicString targetName = name;
    for (unsigned i = 0; i < elements.size(); i++) {
        HTMLFormControlElement* elt = elements[i];
        if (elt->formControlName() == targetName)
            return kit(elt);
    }
    return nil;
}

static HTMLInputElement* inputElementFromDOMElement(DOMElement* element)
{
    Element* node = core(element);
    return node && node->hasTagName(inputTag) ? static_cast<HTMLInputElement*>(node) : 0;
}

- (BOOL)elementDoesAutoComplete:(DOMElement *)element
{
    HTMLInputElement* inputElement = inputElementFromDOMElement(element);
    return inputElement
        && inputElement->inputType() == HTMLInputElement::TEXT
        && inputElement->autoComplete();
}

- (BOOL)elementIsPassword:(DOMElement *)element
{
    HTMLInputElement* inputElement = inputElementFromDOMElement(element);
    return inputElement
        && inputElement->inputType() == HTMLInputElement::PASSWORD;
}

- (DOMElement *)formForElement:(DOMElement *)element
{
    HTMLInputElement* inputElement = inputElementFromDOMElement(element);
    return inputElement ? kit(inputElement->form()) : 0;
}

- (DOMElement *)currentForm
{
    return kit(core([_private->dataSource webFrame])->currentForm());
}

- (NSArray *)controlsInForm:(DOMElement *)form
{
    HTMLFormElement* formElement = formElementFromDOMElement(form);
    if (!formElement)
        return nil;
    NSMutableArray *results = nil;
    Vector<HTMLFormControlElement*>& elements = formElement->formElements;
    for (unsigned i = 0; i < elements.size(); i++) {
        if (elements[i]->isEnumeratable()) { // Skip option elements, other duds
            DOMElement* de = kit(elements[i]);
            if (!results)
                results = [NSMutableArray arrayWithObject:de];
            else
                [results addObject:de];
        }
    }
    return results;
}

- (NSString *)searchForLabels:(NSArray *)labels beforeElement:(DOMElement *)element
{
    return core([_private->dataSource webFrame])->searchForLabelsBeforeElement(labels, core(element));
}

- (NSString *)matchLabels:(NSArray *)labels againstElement:(DOMElement *)element
{
    return core([_private->dataSource webFrame])->matchLabelsAgainstElement(labels, core(element));
}

@end
