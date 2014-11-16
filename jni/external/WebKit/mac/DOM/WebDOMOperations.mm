/*
 * Copyright (C) 2005, 2008 Apple Inc. All rights reserved.
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

#import "WebDOMOperationsPrivate.h"

#import "DOMDocumentInternal.h"
#import "DOMNodeInternal.h"
#import "DOMRangeInternal.h"
#import "WebArchiveInternal.h"
#import "WebDataSourcePrivate.h"
#import "WebFrameInternal.h"
#import "WebFramePrivate.h"
#import "WebKitNSStringExtras.h"
#import <WebCore/CSSHelper.h>
#import <WebCore/Document.h>
#import <WebCore/LegacyWebArchive.h>
#import <WebCore/markup.h>
#import <WebKit/DOMExtensions.h>
#import <WebKit/DOMHTML.h>
#import <wtf/Assertions.h>

using namespace WebCore;

@implementation DOMNode (WebDOMNodeOperations)

- (WebArchive *)webArchive
{
    return [[[WebArchive alloc] _initWithCoreLegacyWebArchive:LegacyWebArchive::create(core(self))] autorelease];
}

- (NSString *)markupString
{
    return createFullMarkup(core(self));
}

@end

/* This doesn't appear to be used by anyone.  We should consider removing this. */
@implementation DOMNode (WebDOMNodeOperationsInternal)

- (NSArray *)_subresourceURLs
{
    ListHashSet<KURL> urls;
    core(self)->getSubresourceURLs(urls);
    if (!urls.size())
        return nil;

    NSMutableArray *array = [NSMutableArray arrayWithCapacity:urls.size()];
    ListHashSet<KURL>::iterator end = urls.end();
    for (ListHashSet<KURL>::iterator it = urls.begin(); it != end; ++it)
        [array addObject:(NSURL *)*it];

    return array;
}

@end

@implementation DOMDocument (WebDOMDocumentOperations)

- (WebFrame *)webFrame
{
    Document* document = core(self);
    Frame* frame = document->frame();
    if (!frame)
        return nil;
    return kit(frame);
}

- (NSURL *)URLWithAttributeString:(NSString *)string
{
    // FIXME: Is deprecatedParseURL appropriate here?
    return core(self)->completeURL(deprecatedParseURL(string));
}

@end

@implementation DOMDocument (WebDOMDocumentOperationsInternal)

/* This doesn't appear to be used by anyone.  We should consider removing this. */
- (DOMRange *)_createRangeWithNode:(DOMNode *)node
{
    DOMRange *range = [self createRange];
    [range selectNode:node];
    return range;
}

- (DOMRange *)_documentRange
{
    return [self _createRangeWithNode:[self documentElement]];
}

@end

@implementation DOMDocument (WebDOMDocumentOperationsPrivate)

- (NSArray *)_focusableNodes
{
    Vector<RefPtr<Node> > nodes;
    core(self)->getFocusableNodes(nodes);
    NSMutableArray *array = [NSMutableArray arrayWithCapacity:nodes.size()];
    for (unsigned i = 0; i < nodes.size(); ++i)
        [array addObject:kit(nodes[i].get())];
    return array;
}

@end

@implementation DOMRange (WebDOMRangeOperations)

- (WebArchive *)webArchive
{
    return [[[WebArchive alloc] _initWithCoreLegacyWebArchive:LegacyWebArchive::create(core(self))] autorelease];
}

- (NSString *)markupString
{
    return createFullMarkup(core(self));
}

@end

@implementation DOMHTMLFrameElement (WebDOMHTMLFrameElementOperations)

- (WebFrame *)contentFrame
{
    return [[self contentDocument] webFrame];
}

@end

@implementation DOMHTMLIFrameElement (WebDOMHTMLIFrameElementOperations)

- (WebFrame *)contentFrame
{
    return [[self contentDocument] webFrame];
}

@end

@implementation DOMHTMLObjectElement (WebDOMHTMLObjectElementOperations)

- (WebFrame *)contentFrame
{
    return [[self contentDocument] webFrame];
}

@end
