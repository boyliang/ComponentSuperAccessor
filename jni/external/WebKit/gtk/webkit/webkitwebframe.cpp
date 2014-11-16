/*
 * Copyright (C) 2007, 2008 Holger Hans Peter Freyther
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2007 Apple Inc.
 * Copyright (C) 2008 Christian Dywan <christian@imendio.com>
 * Copyright (C) 2008 Collabora Ltd.
 * Copyright (C) 2008 Nuanti Ltd.
 * Copyright (C) 2009 Jan Alonzo <jmalonzo@gmail.com>
 * Copyright (C) 2009 Gustavo Noronha Silva <gns@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"

#include "webkitenumtypes.h"
#include "webkitwebframe.h"
#include "webkitwebview.h"
#include "webkitmarshal.h"
#include "webkitprivate.h"

#include "AccessibilityObjectWrapperAtk.h"
#include "AnimationController.h"
#include "AXObjectCache.h"
#include "CString.h"
#include "DocumentLoader.h"
#include "FrameLoader.h"
#include "FrameLoaderClientGtk.h"
#include "FrameTree.h"
#include "FrameView.h"
#include <glib/gi18n-lib.h>
#include "GCController.h"
#include "GraphicsContext.h"
#include "HTMLFrameOwnerElement.h"
#include "JSDOMWindow.h"
#include "JSLock.h"
#include "PrintContext.h"
#include "RenderView.h"
#include "RenderTreeAsText.h"
#include "JSDOMBinding.h"
#include "ScriptController.h"
#include "SubstituteData.h"

#include <atk/atk.h>
#include <JavaScriptCore/APICast.h>

/**
 * SECTION:webkitwebframe
 * @short_description: The content of a #WebKitWebView
 *
 * A #WebKitWebView contains a main #WebKitWebFrame. A #WebKitWebFrame
 * contains the content of one URI. The URI and name of the frame can
 * be retrieved, the load status and progress can be observed using the
 * signals and can be controlled using the methods of the #WebKitWebFrame.
 * A #WebKitWebFrame can have any number of children and one child can
 * be found by using #webkit_web_frame_find_frame.
 *
 * <informalexample><programlisting>
 * /<!-- -->* Get the frame from the #WebKitWebView *<!-- -->/
 * WebKitWebFrame *frame = webkit_web_view_get_main_frame (WEBKIT_WEB_VIEW(my_view));
 * g_print("The URI of this frame is '%s'", webkit_web_frame_get_uri (frame));
 * </programlisting></informalexample>
 */

using namespace WebKit;
using namespace WebCore;
using namespace std;

enum {
    CLEARED,
    LOAD_COMMITTED,
    LOAD_DONE,
    TITLE_CHANGED,
    HOVERING_OVER_LINK,
    LAST_SIGNAL
};

enum {
    PROP_0,

    PROP_NAME,
    PROP_TITLE,
    PROP_URI,
    PROP_LOAD_STATUS
};

static guint webkit_web_frame_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE(WebKitWebFrame, webkit_web_frame, G_TYPE_OBJECT)

static void webkit_web_frame_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec)
{
    WebKitWebFrame* frame = WEBKIT_WEB_FRAME(object);

    switch(prop_id) {
    case PROP_NAME:
        g_value_set_string(value, webkit_web_frame_get_name(frame));
        break;
    case PROP_TITLE:
        g_value_set_string(value, webkit_web_frame_get_title(frame));
        break;
    case PROP_URI:
        g_value_set_string(value, webkit_web_frame_get_uri(frame));
        break;
    case PROP_LOAD_STATUS:
        g_value_set_enum(value, webkit_web_frame_get_load_status(frame));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

// Called from the FrameLoaderClient when it is destroyed. Normally
// the unref in the FrameLoaderClient is destroying this object as
// well but due reference counting a user might have added a reference...
void webkit_web_frame_core_frame_gone(WebKitWebFrame* frame)
{
    ASSERT(WEBKIT_IS_WEB_FRAME(frame));
    frame->priv->coreFrame = 0;
}

static void webkit_web_frame_finalize(GObject* object)
{
    WebKitWebFrame* frame = WEBKIT_WEB_FRAME(object);
    WebKitWebFramePrivate* priv = frame->priv;

    if (priv->coreFrame) {
        priv->coreFrame->loader()->cancelAndClear();
        priv->coreFrame = 0;
    }

    g_free(priv->name);
    g_free(priv->title);
    g_free(priv->uri);

    G_OBJECT_CLASS(webkit_web_frame_parent_class)->finalize(object);
}

static void webkit_web_frame_class_init(WebKitWebFrameClass* frameClass)
{
    webkit_init();

    /*
     * signals
     */
    webkit_web_frame_signals[CLEARED] = g_signal_new("cleared",
            G_TYPE_FROM_CLASS(frameClass),
            (GSignalFlags)G_SIGNAL_RUN_LAST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    webkit_web_frame_signals[LOAD_COMMITTED] = g_signal_new("load-committed",
            G_TYPE_FROM_CLASS(frameClass),
            (GSignalFlags)G_SIGNAL_RUN_LAST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    /**
     * WebKitWebFrame::load-done
     * @web_frame: the object on which the signal is emitted
     *
     * Emitted when frame loading is done.
     *
     * Deprecated: Use WebKitWebView::load-finished instead, and/or
     * WebKitWebView::load-error to be notified of load errors
     */
    webkit_web_frame_signals[LOAD_DONE] = g_signal_new("load-done",
            G_TYPE_FROM_CLASS(frameClass),
            (GSignalFlags)G_SIGNAL_RUN_LAST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__BOOLEAN,
            G_TYPE_NONE, 1,
            G_TYPE_BOOLEAN);

    webkit_web_frame_signals[TITLE_CHANGED] = g_signal_new("title-changed",
            G_TYPE_FROM_CLASS(frameClass),
            (GSignalFlags)G_SIGNAL_RUN_LAST,
            0,
            NULL,
            NULL,
            webkit_marshal_VOID__STRING,
            G_TYPE_NONE, 1,
            G_TYPE_STRING);

    webkit_web_frame_signals[HOVERING_OVER_LINK] = g_signal_new("hovering-over-link",
            G_TYPE_FROM_CLASS(frameClass),
            (GSignalFlags)G_SIGNAL_RUN_LAST,
            0,
            NULL,
            NULL,
            webkit_marshal_VOID__STRING_STRING,
            G_TYPE_NONE, 2,
            G_TYPE_STRING, G_TYPE_STRING);

    /*
     * implementations of virtual methods
     */
    GObjectClass* objectClass = G_OBJECT_CLASS(frameClass);
    objectClass->finalize = webkit_web_frame_finalize;
    objectClass->get_property = webkit_web_frame_get_property;

    /*
     * properties
     */
    g_object_class_install_property(objectClass, PROP_NAME,
                                    g_param_spec_string("name",
                                                        _("Name"),
                                                        _("The name of the frame"),
                                                        NULL,
                                                        WEBKIT_PARAM_READABLE));

    g_object_class_install_property(objectClass, PROP_TITLE,
                                    g_param_spec_string("title",
                                                        _("Title"),
                                                        _("The document title of the frame"),
                                                        NULL,
                                                        WEBKIT_PARAM_READABLE));

    g_object_class_install_property(objectClass, PROP_URI,
                                    g_param_spec_string("uri",
                                                        _("URI"),
                                                        _("The current URI of the contents displayed by the frame"),
                                                        NULL,
                                                        WEBKIT_PARAM_READABLE));

    /**
    * WebKitWebFrame:load-status:
    *
    * Determines the current status of the load.
    *
    * Since: 1.1.7
    */
    g_object_class_install_property(objectClass, PROP_LOAD_STATUS,
                                    g_param_spec_enum("load-status",
                                                      "Load Status",
                                                      "Determines the current status of the load",
                                                      WEBKIT_TYPE_LOAD_STATUS,
                                                      WEBKIT_LOAD_FINISHED,
                                                      WEBKIT_PARAM_READABLE));

    g_type_class_add_private(frameClass, sizeof(WebKitWebFramePrivate));
}

static void webkit_web_frame_init(WebKitWebFrame* frame)
{
    WebKitWebFramePrivate* priv = WEBKIT_WEB_FRAME_GET_PRIVATE(frame);

    // TODO: Move constructor code here.
    frame->priv = priv;
}

/**
 * webkit_web_frame_new:
 * @web_view: the controlling #WebKitWebView
 *
 * Creates a new #WebKitWebFrame initialized with a controlling #WebKitWebView.
 *
 * Returns: a new #WebKitWebFrame
 *
 * Deprecated: 1.0.2: #WebKitWebFrame can only be used to inspect existing
 * frames.
 **/
WebKitWebFrame* webkit_web_frame_new(WebKitWebView* webView)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_VIEW(webView), NULL);

    WebKitWebFrame* frame = WEBKIT_WEB_FRAME(g_object_new(WEBKIT_TYPE_WEB_FRAME, NULL));
    WebKitWebFramePrivate* priv = frame->priv;
    WebKitWebViewPrivate* viewPriv = WEBKIT_WEB_VIEW_GET_PRIVATE(webView);

    priv->webView = webView;
    WebKit::FrameLoaderClient* client = new WebKit::FrameLoaderClient(frame);
    priv->coreFrame = Frame::create(viewPriv->corePage, 0, client).get();
    priv->coreFrame->init();

    return frame;
}

PassRefPtr<Frame> webkit_web_frame_init_with_web_view(WebKitWebView* webView, HTMLFrameOwnerElement* element)
{
    WebKitWebFrame* frame = WEBKIT_WEB_FRAME(g_object_new(WEBKIT_TYPE_WEB_FRAME, NULL));
    WebKitWebFramePrivate* priv = frame->priv;
    WebKitWebViewPrivate* viewPriv = WEBKIT_WEB_VIEW_GET_PRIVATE(webView);

    priv->webView = webView;
    WebKit::FrameLoaderClient* client = new WebKit::FrameLoaderClient(frame);

    RefPtr<Frame> coreFrame = Frame::create(viewPriv->corePage, element, client);
    priv->coreFrame = coreFrame.get();

    return coreFrame.release();
}

/**
 * webkit_web_frame_get_title:
 * @frame: a #WebKitWebFrame
 *
 * Returns the @frame's document title
 *
 * Return value: the title of @frame
 */
G_CONST_RETURN gchar* webkit_web_frame_get_title(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), NULL);

    WebKitWebFramePrivate* priv = frame->priv;
    return priv->title;
}

/**
 * webkit_web_frame_get_uri:
 * @frame: a #WebKitWebFrame
 *
 * Returns the current URI of the contents displayed by the @frame
 *
 * Return value: the URI of @frame
 */
G_CONST_RETURN gchar* webkit_web_frame_get_uri(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), NULL);

    WebKitWebFramePrivate* priv = frame->priv;
    return priv->uri;
}

/**
 * webkit_web_frame_get_web_view:
 * @frame: a #WebKitWebFrame
 *
 * Returns the #WebKitWebView that manages this #WebKitWebFrame.
 *
 * The #WebKitWebView returned manages the entire hierarchy of #WebKitWebFrame
 * objects that contains @frame.
 *
 * Return value: the #WebKitWebView that manages @frame
 */
WebKitWebView* webkit_web_frame_get_web_view(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), NULL);

    WebKitWebFramePrivate* priv = frame->priv;
    return priv->webView;
}

/**
 * webkit_web_frame_get_name:
 * @frame: a #WebKitWebFrame
 *
 * Returns the @frame's name
 *
 * Return value: the name of @frame
 */
G_CONST_RETURN gchar* webkit_web_frame_get_name(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), NULL);

    WebKitWebFramePrivate* priv = frame->priv;

    if (priv->name)
        return priv->name;

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return "";

    String string = coreFrame->tree()->name();
    priv->name = g_strdup(string.utf8().data());
    return priv->name;
}

/**
 * webkit_web_frame_get_parent:
 * @frame: a #WebKitWebFrame
 *
 * Returns the @frame's parent frame, or %NULL if it has none.
 *
 * Return value: the parent #WebKitWebFrame or %NULL in case there is none
 */
WebKitWebFrame* webkit_web_frame_get_parent(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), NULL);

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return NULL;

    return kit(coreFrame->tree()->parent());
}

/**
 * webkit_web_frame_load_uri:
 * @frame: a #WebKitWebFrame
 * @uri: an URI string
 *
 * Requests loading of the specified URI string.
 *
 * Since: 1.1.1
 */
void webkit_web_frame_load_uri(WebKitWebFrame* frame, const gchar* uri)
{
    g_return_if_fail(WEBKIT_IS_WEB_FRAME(frame));
    g_return_if_fail(uri);

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return;

    coreFrame->loader()->load(ResourceRequest(KURL(KURL(), String::fromUTF8(uri))), false);
}

static void webkit_web_frame_load_data(WebKitWebFrame* frame, const gchar* content, const gchar* mimeType, const gchar* encoding, const gchar* baseURL, const gchar* unreachableURL)
{
    Frame* coreFrame = core(frame);
    ASSERT(coreFrame);

    KURL baseKURL = baseURL ? KURL(KURL(), String::fromUTF8(baseURL)) : blankURL();

    ResourceRequest request(baseKURL);

    RefPtr<SharedBuffer> sharedBuffer = SharedBuffer::create(content, strlen(content));
    SubstituteData substituteData(sharedBuffer.release(),
                                  mimeType ? String::fromUTF8(mimeType) : String::fromUTF8("text/html"),
                                  encoding ? String::fromUTF8(encoding) : String::fromUTF8("UTF-8"),
                                  baseKURL,
                                  KURL(KURL(), String::fromUTF8(unreachableURL)));

    coreFrame->loader()->load(request, substituteData, false);
}

/**
 * webkit_web_frame_load_string:
 * @frame: a #WebKitWebFrame
 * @content: an URI string
 * @mime_type: the MIME type, or %NULL
 * @encoding: the encoding, or %NULL
 * @base_uri: the base URI for relative locations
 *
 * Requests loading of the given @content with the specified @mime_type,
 * @encoding and @base_uri.
 *
 * If @mime_type is %NULL, "text/html" is assumed.
 *
 * If @encoding is %NULL, "UTF-8" is assumed.
 *
 * Since: 1.1.1
 */
void webkit_web_frame_load_string(WebKitWebFrame* frame, const gchar* content, const gchar* contentMimeType, const gchar* contentEncoding, const gchar* baseUri)
{
    g_return_if_fail(WEBKIT_IS_WEB_FRAME(frame));
    g_return_if_fail(content);

    webkit_web_frame_load_data(frame, content, contentMimeType, contentEncoding, baseUri, NULL);
}

/**
 * webkit_web_frame_load_alternate_string:
 * @frame: a #WebKitWebFrame
 * @content: the alternate content to display as the main page of the @frame
 * @base_url: the base URI for relative locations
 * @unreachable_url: the URL for the alternate page content
 *
 * Request loading of an alternate content for a URL that is unreachable.
 * Using this method will preserve the back-forward list. The URI passed in
 * @base_url has to be an absolute URI.
 *
 * Since: 1.1.6
 */
void webkit_web_frame_load_alternate_string(WebKitWebFrame* frame, const gchar* content, const gchar* baseURL, const gchar* unreachableURL)
{
    g_return_if_fail(WEBKIT_IS_WEB_FRAME(frame));
    g_return_if_fail(content);

    webkit_web_frame_load_data(frame, content, NULL, NULL, baseURL, unreachableURL);
}

/**
 * webkit_web_frame_load_request:
 * @frame: a #WebKitWebFrame
 * @request: a #WebKitNetworkRequest
 *
 * Connects to a given URI by initiating an asynchronous client request.
 *
 * Creates a provisional data source that will transition to a committed data
 * source once any data has been received. Use webkit_web_frame_stop_loading() to
 * stop the load. This function is typically invoked on the main frame.
 */
void webkit_web_frame_load_request(WebKitWebFrame* frame, WebKitNetworkRequest* request)
{
    g_return_if_fail(WEBKIT_IS_WEB_FRAME(frame));
    g_return_if_fail(WEBKIT_IS_NETWORK_REQUEST(request));

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return;

    coreFrame->loader()->load(core(request), false);
}

/**
 * webkit_web_frame_stop_loading:
 * @frame: a #WebKitWebFrame
 *
 * Stops any pending loads on @frame's data source, and those of its children.
 */
void webkit_web_frame_stop_loading(WebKitWebFrame* frame)
{
    g_return_if_fail(WEBKIT_IS_WEB_FRAME(frame));

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return;

    coreFrame->loader()->stopAllLoaders();
}

/**
 * webkit_web_frame_reload:
 * @frame: a #WebKitWebFrame
 *
 * Reloads the initial request.
 */
void webkit_web_frame_reload(WebKitWebFrame* frame)
{
    g_return_if_fail(WEBKIT_IS_WEB_FRAME(frame));

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return;

    coreFrame->loader()->reload();
}

/**
 * webkit_web_frame_find_frame:
 * @frame: a #WebKitWebFrame
 * @name: the name of the frame to be found
 *
 * For pre-defined names, returns @frame if @name is "_self" or "_current",
 * returns @frame's parent frame if @name is "_parent", and returns the main
 * frame if @name is "_top". Also returns @frame if it is the main frame and
 * @name is either "_parent" or "_top". For other names, this function returns
 * the first frame that matches @name. This function searches @frame and its
 * descendents first, then @frame's parent and its children moving up the
 * hierarchy until a match is found. If no match is found in @frame's
 * hierarchy, this function will search for a matching frame in other main
 * frame hierarchies. Returns %NULL if no match is found.
 *
 * Return value: the found #WebKitWebFrame or %NULL in case none is found
 */
WebKitWebFrame* webkit_web_frame_find_frame(WebKitWebFrame* frame, const gchar* name)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), NULL);
    g_return_val_if_fail(name, NULL);

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return NULL;

    String nameString = String::fromUTF8(name);
    return kit(coreFrame->tree()->find(AtomicString(nameString)));
}

/**
 * webkit_web_frame_get_global_context:
 * @frame: a #WebKitWebFrame
 *
 * Gets the global JavaScript execution context. Use this function to bridge
 * between the WebKit and JavaScriptCore APIs.
 *
 * Return value: the global JavaScript context
 */
JSGlobalContextRef webkit_web_frame_get_global_context(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), NULL);

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return NULL;

    return toGlobalRef(coreFrame->script()->globalObject()->globalExec());
}

/**
 * webkit_web_frame_get_children:
 * @frame: a #WebKitWebFrame
 *
 * Return value: child frames of @frame
 */
GSList* webkit_web_frame_get_children(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), NULL);

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return NULL;

    GSList* children = NULL;
    for (Frame* child = coreFrame->tree()->firstChild(); child; child = child->tree()->nextSibling()) {
        FrameLoader* loader = child->loader();
        WebKit::FrameLoaderClient* client = static_cast<WebKit::FrameLoaderClient*>(loader->client());
        if (client)
          children = g_slist_append(children, client->webFrame());
    }

    return children;
}

/**
 * webkit_web_frame_get_inner_text:
 * @frame: a #WebKitWebFrame
 *
 * Return value: inner text of @frame
 */
gchar* webkit_web_frame_get_inner_text(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), NULL);

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return g_strdup("");

    FrameView* view = coreFrame->view();

    if (view && view->layoutPending())
        view->layout();

    Element* documentElement = coreFrame->document()->documentElement();
    String string =  documentElement->innerText();
    return g_strdup(string.utf8().data());
}

/**
 * webkit_web_frame_dump_render_tree:
 * @frame: a #WebKitWebFrame
 *
 * Return value: Non-recursive render tree dump of @frame
 */
gchar* webkit_web_frame_dump_render_tree(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), NULL);

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return g_strdup("");

    FrameView* view = coreFrame->view();

    if (view && view->layoutPending())
        view->layout();

    String string = externalRepresentation(coreFrame->contentRenderer());
    return g_strdup(string.utf8().data());
}

static void begin_print_callback(GtkPrintOperation* op, GtkPrintContext* context, gpointer user_data)
{
    PrintContext* printContext = reinterpret_cast<PrintContext*>(user_data);

    float width = gtk_print_context_get_width(context);
    float height = gtk_print_context_get_height(context);
    FloatRect printRect = FloatRect(0, 0, width, height);

    printContext->begin(width);

    // TODO: Margin adjustments and header/footer support
    float headerHeight = 0;
    float footerHeight = 0;
    float pageHeight; // height of the page adjusted by margins
    printContext->computePageRects(printRect, headerHeight, footerHeight, 1.0, pageHeight);
    gtk_print_operation_set_n_pages(op, printContext->pageCount());
}

static void draw_page_callback(GtkPrintOperation* op, GtkPrintContext* context, gint page_nr, gpointer user_data)
{
    PrintContext* printContext = reinterpret_cast<PrintContext*>(user_data);

    cairo_t* cr = gtk_print_context_get_cairo_context(context);
    GraphicsContext ctx(cr);
    float width = gtk_print_context_get_width(context);
    printContext->spoolPage(ctx, page_nr, width);
}

static void end_print_callback(GtkPrintOperation* op, GtkPrintContext* context, gpointer user_data)
{
    PrintContext* printContext = reinterpret_cast<PrintContext*>(user_data);
    printContext->end();
}

/**
 * webkit_web_frame_print_full:
 * @frame: a #WebKitWebFrame to be printed
 * @operation: the #GtkPrintOperation to be carried
 * @action: the #GtkPrintOperationAction to be performed
 * @error: #GError for error return
 *
 * Prints the given #WebKitFrame, using the given #GtkPrintOperation
 * and #GtkPrintOperationAction. This function wraps a call to
 * gtk_print_operation_run() for printing the contents of the
 * #WebKitWebFrame.
 *
 * Since: 1.1.5
 */
GtkPrintOperationResult webkit_web_frame_print_full(WebKitWebFrame* frame, GtkPrintOperation* operation, GtkPrintOperationAction action, GError** error)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), GTK_PRINT_OPERATION_RESULT_ERROR);
    g_return_val_if_fail(GTK_IS_PRINT_OPERATION(operation), GTK_PRINT_OPERATION_RESULT_ERROR);

    GtkWidget* topLevel = gtk_widget_get_toplevel(GTK_WIDGET(webkit_web_frame_get_web_view(frame)));
    if (!GTK_WIDGET_TOPLEVEL(topLevel))
        topLevel = NULL;

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return GTK_PRINT_OPERATION_RESULT_ERROR;

    PrintContext printContext(coreFrame);

    g_signal_connect(operation, "begin-print", G_CALLBACK(begin_print_callback), &printContext);
    g_signal_connect(operation, "draw-page", G_CALLBACK(draw_page_callback), &printContext);
    g_signal_connect(operation, "end-print", G_CALLBACK(end_print_callback), &printContext);

    return gtk_print_operation_run(operation, action, GTK_WINDOW(topLevel), error);
}

/**
 * webkit_web_frame_print:
 * @frame: a #WebKitWebFrame
 *
 * Prints the given #WebKitFrame, by presenting a print dialog to the
 * user. If you need more control over the printing process, see
 * webkit_web_frame_print_full().
 *
 * Since: 1.1.5
 */
void webkit_web_frame_print(WebKitWebFrame* frame)
{
    g_return_if_fail(WEBKIT_IS_WEB_FRAME(frame));

    WebKitWebFramePrivate* priv = frame->priv;
    GtkPrintOperation* operation = gtk_print_operation_new();
    GError* error = 0;

    webkit_web_frame_print_full(frame, operation, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, &error);
    g_object_unref(operation);

    if (error) {
        GtkWidget* window = gtk_widget_get_toplevel(GTK_WIDGET(priv->webView));
        GtkWidget* dialog = gtk_message_dialog_new(GTK_WIDGET_TOPLEVEL(window) ? GTK_WINDOW(window) : 0,
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "%s", error->message);
        g_error_free(error);

        g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
        gtk_widget_show(dialog);
    }
}

bool webkit_web_frame_pause_animation(WebKitWebFrame* frame, const gchar* name, double time, const gchar* element)
{
    ASSERT(core(frame));
    Element* coreElement = core(frame)->document()->getElementById(AtomicString(element));
    if (!coreElement || !coreElement->renderer())
        return false;
    return core(frame)->animation()->pauseAnimationAtTime(coreElement->renderer(), AtomicString(name), time);
}

bool webkit_web_frame_pause_transition(WebKitWebFrame* frame, const gchar* name, double time, const gchar* element)
{
    ASSERT(core(frame));
    Element* coreElement = core(frame)->document()->getElementById(AtomicString(element));
    if (!coreElement || !coreElement->renderer())
        return false;
    return core(frame)->animation()->pauseTransitionAtTime(coreElement->renderer(), AtomicString(name), time);
}

unsigned int webkit_web_frame_number_of_active_animations(WebKitWebFrame* frame)
{
    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return 0;

    AnimationController* controller = coreFrame->animation();
    if (!controller)
        return 0;

    return controller->numberOfActiveAnimations();
}

gchar* webkit_web_frame_get_response_mime_type(WebKitWebFrame* frame)
{
    Frame* coreFrame = core(frame);
    DocumentLoader* docLoader = coreFrame->loader()->documentLoader();
    String mimeType = docLoader->responseMIMEType();
    return g_strdup(mimeType.utf8().data());
}

/**
 * webkit_web_frame_get_load_status:
 * @frame: a #WebKitWebView
 *
 * Determines the current status of the load.
 *
 * Since: 1.1.7
 */
WebKitLoadStatus webkit_web_frame_get_load_status(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), WEBKIT_LOAD_FINISHED);

    WebKitWebFramePrivate* priv = frame->priv;
    return priv->loadStatus;
}

void webkit_web_frame_clear_main_frame_name(WebKitWebFrame* frame)
{
    g_return_if_fail(WEBKIT_IS_WEB_FRAME(frame));

    core(frame)->tree()->clearName();
}

void webkit_gc_collect_javascript_objects()
{
    gcController().garbageCollectNow();
}

void webkit_gc_collect_javascript_objects_on_alternate_thread(gboolean waitUntilDone)
{
    gcController().garbageCollectOnAlternateThreadForDebugging(waitUntilDone);
}

gsize webkit_gc_count_javascript_objects()
{
    JSC::JSLock lock(JSC::SilenceAssertionsOnly);
    return JSDOMWindow::commonJSGlobalData()->heap.objectCount();

}

AtkObject* webkit_web_frame_get_focused_accessible_element(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), NULL);

#if HAVE(ACCESSIBILITY)
    if (!AXObjectCache::accessibilityEnabled())
        AXObjectCache::enableAccessibility();

    WebKitWebFramePrivate* priv = frame->priv;
    if (!priv->coreFrame || !priv->coreFrame->document())
        return NULL;

    RenderView* root = toRenderView(priv->coreFrame->document()->renderer());
    if (!root)
        return NULL;

    AtkObject* wrapper =  priv->coreFrame->document()->axObjectCache()->getOrCreate(root)->wrapper();
    if (!wrapper)
        return NULL;

    return webkit_accessible_get_focused_element(WEBKIT_ACCESSIBLE(wrapper));
#else
    return NULL;
#endif
}
