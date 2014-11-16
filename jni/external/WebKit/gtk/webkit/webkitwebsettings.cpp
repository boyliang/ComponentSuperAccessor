/*
 * Copyright (C) 2008 Christian Dywan <christian@imendio.com>
 * Copyright (C) 2008 Nuanti Ltd.
 * Copyright (C) 2008 Collabora Ltd.
 * Copyright (C) 2008 Holger Hans Peter Freyther
 * Copyright (C) 2009 Jan Michael Alonzo
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
#include "webkitwebsettings.h"

#include "webkitprivate.h"
#include "webkitversion.h"

#include "CString.h"
#include "FileSystem.h"
#include "PluginDatabase.h"
#include "Language.h"
#include "PlatformString.h"

#include <glib/gi18n-lib.h>
#if PLATFORM(UNIX)
#include <sys/utsname.h>
#endif

/**
 * SECTION:webkitwebsettings
 * @short_description: Control the behaviour of a #WebKitWebView
 *
 * #WebKitWebSettings can be applied to a #WebKitWebView to control
 * the to be used text encoding, color, font sizes, printing mode,
 * script support, loading of images and various other things.
 *
 * <informalexample><programlisting>
 * /<!-- -->* Create a new websettings and disable java script *<!-- -->/
 * WebKitWebSettings *settings = webkit_web_settings_new ();
 * g_object_set (G_OBJECT(settings), "enable-scripts", FALSE, NULL);
 *
 * /<!-- -->* Apply the result *<!-- -->/
 * webkit_web_view_set_settings (WEBKIT_WEB_VIEW(my_webview), settings);
 * </programlisting></informalexample>
 */

using namespace WebCore;

G_DEFINE_TYPE(WebKitWebSettings, webkit_web_settings, G_TYPE_OBJECT)

struct _WebKitWebSettingsPrivate {
    gchar* default_encoding;
    gchar* cursive_font_family;
    gchar* default_font_family;
    gchar* fantasy_font_family;
    gchar* monospace_font_family;
    gchar* sans_serif_font_family;
    gchar* serif_font_family;
    guint default_font_size;
    guint default_monospace_font_size;
    guint minimum_font_size;
    guint minimum_logical_font_size;
    gboolean enforce_96_dpi;
    gboolean auto_load_images;
    gboolean auto_shrink_images;
    gboolean print_backgrounds;
    gboolean enable_scripts;
    gboolean enable_plugins;
    gboolean resizable_text_areas;
    gchar* user_stylesheet_uri;
    gfloat zoom_step;
    gboolean enable_developer_extras;
    gboolean enable_private_browsing;
    gboolean enable_spell_checking;
    gchar* spell_checking_languages;
    GSList* spell_checking_languages_list;
    gboolean enable_caret_browsing;
    gboolean enable_html5_database;
    gboolean enable_html5_local_storage;
    gboolean enable_xss_auditor;
    gchar* user_agent;
    gboolean javascript_can_open_windows_automatically;
    gboolean enable_offline_web_application_cache;
};

#define WEBKIT_WEB_SETTINGS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), WEBKIT_TYPE_WEB_SETTINGS, WebKitWebSettingsPrivate))

enum {
    PROP_0,

    PROP_DEFAULT_ENCODING,
    PROP_CURSIVE_FONT_FAMILY,
    PROP_DEFAULT_FONT_FAMILY,
    PROP_FANTASY_FONT_FAMILY,
    PROP_MONOSPACE_FONT_FAMILY,
    PROP_SANS_SERIF_FONT_FAMILY,
    PROP_SERIF_FONT_FAMILY,
    PROP_DEFAULT_FONT_SIZE,
    PROP_DEFAULT_MONOSPACE_FONT_SIZE,
    PROP_MINIMUM_FONT_SIZE,
    PROP_MINIMUM_LOGICAL_FONT_SIZE,
    PROP_ENFORCE_96_DPI,
    PROP_AUTO_LOAD_IMAGES,
    PROP_AUTO_SHRINK_IMAGES,
    PROP_PRINT_BACKGROUNDS,
    PROP_ENABLE_SCRIPTS,
    PROP_ENABLE_PLUGINS,
    PROP_RESIZABLE_TEXT_AREAS,
    PROP_USER_STYLESHEET_URI,
    PROP_ZOOM_STEP,
    PROP_ENABLE_DEVELOPER_EXTRAS,
    PROP_ENABLE_PRIVATE_BROWSING,
    PROP_ENABLE_SPELL_CHECKING,
    PROP_SPELL_CHECKING_LANGUAGES,
    PROP_ENABLE_CARET_BROWSING,
    PROP_ENABLE_HTML5_DATABASE,
    PROP_ENABLE_HTML5_LOCAL_STORAGE,
    PROP_ENABLE_XSS_AUDITOR,
    PROP_USER_AGENT,
    PROP_JAVASCRIPT_CAN_OPEN_WINDOWS_AUTOMATICALLY,
    PROP_ENABLE_OFFLINE_WEB_APPLICATION_CACHE
};

// Create a default user agent string
// This is a liberal interpretation of http://www.mozilla.org/build/revised-user-agent-strings.html
// See also http://developer.apple.com/internet/safari/faq.html#anchor2
static String webkit_get_user_agent()
{
    gchar* platform;
    gchar* osVersion;

#if PLATFORM(X11)
    platform = g_strdup("X11");
#elif PLATFORM(WIN_OS)
    platform = g_strdup("Windows");
#elif PLATFORM(MAC)
    platform = g_strdup("Macintosh");
#elif defined(GDK_WINDOWING_DIRECTFB)
    platform = g_strdup("DirectFB");
#else
    platform = g_strdup("Unknown");
#endif

   // FIXME: platform/version detection can be shared.
#if PLATFORM(DARWIN)

#if PLATFORM(X86)
    osVersion = g_strdup("Intel Mac OS X");
#else
    osVersion = g_strdup("PPC Mac OS X");
#endif

#elif PLATFORM(UNIX)
    struct utsname name;
    if (uname(&name) != -1)
        osVersion = g_strdup_printf("%s %s", name.sysname, name.machine);
    else
        osVersion = g_strdup("Unknown");

#elif PLATFORM(WIN_OS)
    // FIXME: Compute the Windows version
    osVersion = g_strdup("Windows");

#else
    osVersion = g_strdup("Unknown");
#endif

    // We mention Safari since many broken sites check for it (OmniWeb does this too)
    // We re-use the WebKit version, though it doesn't seem to matter much in practice

    DEFINE_STATIC_LOCAL(const String, uaVersion, (String::format("%d.%d+", WEBKIT_USER_AGENT_MAJOR_VERSION, WEBKIT_USER_AGENT_MINOR_VERSION)));
    DEFINE_STATIC_LOCAL(const String, staticUA, (String::format("Mozilla/5.0 (%s; U; %s; %s) AppleWebKit/%s (KHTML, like Gecko) Safari/%s",
                                                                platform, osVersion, defaultLanguage().utf8().data(), uaVersion.utf8().data(), uaVersion.utf8().data())));

    g_free(osVersion);
    g_free(platform);

    return staticUA;
}

static void webkit_web_settings_finalize(GObject* object);

static void webkit_web_settings_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec);

static void webkit_web_settings_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec);

static void webkit_web_settings_class_init(WebKitWebSettingsClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = webkit_web_settings_finalize;
    gobject_class->set_property = webkit_web_settings_set_property;
    gobject_class->get_property = webkit_web_settings_get_property;

    webkit_init();

    GParamFlags flags = (GParamFlags)(WEBKIT_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    g_object_class_install_property(gobject_class,
                                    PROP_DEFAULT_ENCODING,
                                    g_param_spec_string(
                                    "default-encoding",
                                    _("Default Encoding"),
                                    _("The default encoding used to display text."),
                                    "iso-8859-1",
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_CURSIVE_FONT_FAMILY,
                                    g_param_spec_string(
                                    "cursive-font-family",
                                    _("Cursive Font Family"),
                                    _("The default Cursive font family used to display text."),
                                    "serif",
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_DEFAULT_FONT_FAMILY,
                                    g_param_spec_string(
                                    "default-font-family",
                                    _("Default Font Family"),
                                    _("The default font family used to display text."),
                                    "sans-serif",
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_FANTASY_FONT_FAMILY,
                                    g_param_spec_string(
                                    "fantasy-font-family",
                                    _("Fantasy Font Family"),
                                    _("The default Fantasy font family used to display text."),
                                    "serif",
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_MONOSPACE_FONT_FAMILY,
                                    g_param_spec_string(
                                    "monospace-font-family",
                                    _("Monospace Font Family"),
                                    _("The default font family used to display monospace text."),
                                    "monospace",
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_SANS_SERIF_FONT_FAMILY,
                                    g_param_spec_string(
                                    "sans-serif-font-family",
                                    _("Sans Serif Font Family"),
                                    _("The default Sans Serif font family used to display text."),
                                    "sans-serif",
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_SERIF_FONT_FAMILY,
                                    g_param_spec_string(
                                    "serif-font-family",
                                    _("Serif Font Family"),
                                    _("The default Serif font family used to display text."),
                                    "serif",
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_DEFAULT_FONT_SIZE,
                                    g_param_spec_int(
                                    "default-font-size",
                                    _("Default Font Size"),
                                    _("The default font size used to display text."),
                                    5, G_MAXINT, 12,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_DEFAULT_MONOSPACE_FONT_SIZE,
                                    g_param_spec_int(
                                    "default-monospace-font-size",
                                    _("Default Monospace Font Size"),
                                    _("The default font size used to display monospace text."),
                                    5, G_MAXINT, 10,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_MINIMUM_FONT_SIZE,
                                    g_param_spec_int(
                                    "minimum-font-size",
                                    _("Minimum Font Size"),
                                    _("The minimum font size used to display text."),
                                    1, G_MAXINT, 5,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_MINIMUM_LOGICAL_FONT_SIZE,
                                    g_param_spec_int(
                                    "minimum-logical-font-size",
                                    _("Minimum Logical Font Size"),
                                    _("The minimum logical font size used to display text."),
                                    1, G_MAXINT, 5,
                                    flags));

    /**
    * WebKitWebSettings:enforce-96-dpi:
    *
    * Enforce a resolution of 96 DPI. This is meant for compatibility
    * with web pages which cope badly with different screen resolutions
    * and for automated testing.
    * Web browsers and applications that typically display arbitrary
    * content from the web should provide a preference for this.
    *
    * Since: 1.0.3
    */
    g_object_class_install_property(gobject_class,
                                    PROP_ENFORCE_96_DPI,
                                    g_param_spec_boolean(
                                    "enforce-96-dpi",
                                    _("Enforce 96 DPI"),
                                    _("Enforce a resolution of 96 DPI"),
                                    FALSE,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_AUTO_LOAD_IMAGES,
                                    g_param_spec_boolean(
                                    "auto-load-images",
                                    _("Auto Load Images"),
                                    _("Load images automatically."),
                                    TRUE,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_AUTO_SHRINK_IMAGES,
                                    g_param_spec_boolean(
                                    "auto-shrink-images",
                                    _("Auto Shrink Images"),
                                    _("Automatically shrink standalone images to fit."),
                                    TRUE,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_PRINT_BACKGROUNDS,
                                    g_param_spec_boolean(
                                    "print-backgrounds",
                                    _("Print Backgrounds"),
                                    _("Whether background images should be printed."),
                                    TRUE,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_ENABLE_SCRIPTS,
                                    g_param_spec_boolean(
                                    "enable-scripts",
                                    _("Enable Scripts"),
                                    _("Enable embedded scripting languages."),
                                    TRUE,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_ENABLE_PLUGINS,
                                    g_param_spec_boolean(
                                    "enable-plugins",
                                    _("Enable Plugins"),
                                    _("Enable embedded plugin objects."),
                                    TRUE,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_RESIZABLE_TEXT_AREAS,
                                    g_param_spec_boolean(
                                    "resizable-text-areas",
                                    _("Resizable Text Areas"),
                                    _("Whether text areas are resizable."),
                                    TRUE,
                                    flags));

    g_object_class_install_property(gobject_class,
                                    PROP_USER_STYLESHEET_URI,
                                    g_param_spec_string("user-stylesheet-uri",
                                    _("User Stylesheet URI"),
                                    _("The URI of a stylesheet that is applied to every page."),
                                    0,
                                    flags));

    /**
    * WebKitWebSettings:zoom-step:
    *
    * The value by which the zoom level is changed when zooming in or out.
    *
    * Since: 1.0.1
    */
    g_object_class_install_property(gobject_class,
                                    PROP_ZOOM_STEP,
                                    g_param_spec_float(
                                    "zoom-step",
                                    _("Zoom Stepping Value"),
                                    _("The value by which the zoom level is changed when zooming in or out."),
                                    0.0f, G_MAXFLOAT, 0.1f,
                                    flags));

    /**
    * WebKitWebSettings:enable-developer-extras:
    *
    * Whether developer extensions should be enabled. This enables,
    * for now, the Web Inspector, which can be controlled using the
    * #WebKitWebInspector instance held by the #WebKitWebView this
    * setting is enabled for.
    *
    * Since: 1.0.3
    */
    g_object_class_install_property(gobject_class,
                                    PROP_ENABLE_DEVELOPER_EXTRAS,
                                    g_param_spec_boolean(
                                    "enable-developer-extras",
                                    _("Enable Developer Extras"),
                                    _("Enables special extensions that help developers"),
                                    FALSE,
                                    flags));

    /**
    * WebKitWebSettings:enable-private-browsing:
    *
    * Whether to enable private browsing mode. Private browsing mode prevents
    * WebKit from updating the global history and storing any session
    * information e.g., on-disk cache, as well as suppressing any messages
    * from being printed into the (javascript) console.
    *
    * This is currently experimental for WebKitGtk.
    *
    * Since 1.1.2
    */
    g_object_class_install_property(gobject_class,
                                    PROP_ENABLE_PRIVATE_BROWSING,
                                    g_param_spec_boolean(
                                    "enable-private-browsing",
                                    _("Enable Private Browsing"),
                                    _("Enables private browsing mode"),
                                    FALSE,
                                    flags));

    /**
    * WebKitWebSettings:enable-spell-checking:
    *
    * Whether to enable spell checking while typing.
    *
    * Since 1.1.6
    */
    g_object_class_install_property(gobject_class,
                                    PROP_ENABLE_SPELL_CHECKING,
                                    g_param_spec_boolean(
                                    "enable-spell-checking",
                                    _("Enable Spell Checking"),
                                    _("Enables spell checking while typing"),
                                    FALSE,
                                    flags));

    /**
    * WebKitWebSettings:spell-checking-languages:
    *
    * The languages to be used for spell checking, separated by commas.
    *
    * The locale string typically is in the form lang_COUNTRY, where lang
    * is an ISO-639 language code, and COUNTRY is an ISO-3166 country code.
    * For instance, sv_FI for Swedish as written in Finland or pt_BR
    * for Portuguese as written in Brazil.
    *
    * If no value is specified then the value returned by
    * gtk_get_default_language will be used.
    *
    * Since 1.1.6
    */
    g_object_class_install_property(gobject_class,
                                    PROP_SPELL_CHECKING_LANGUAGES,
                                    g_param_spec_string(
                                    "spell-checking-languages",
                                    _("Languages to use for spell checking"),
                                    _("Comma separated list of languages to use for spell checking"),
                                    0,
                                    flags));

    /**
    * WebKitWebSettings:enable-caret-browsing:
    *
    * Whether to enable caret browsing mode.
    *
    * Since 1.1.6
    */
    g_object_class_install_property(gobject_class,
                                    PROP_ENABLE_CARET_BROWSING,
                                    g_param_spec_boolean("enable-caret-browsing",
                                                         _("Enable Caret Browsing"),
                                                         _("Whether to enable accesibility enhanced keyboard navigation"),
                                                         FALSE,
                                                         flags));
    /**
    * WebKitWebSettings:enable-html5-database:
    *
    * Whether to enable HTML5 client-side SQL database support. Client-side
    * SQL database allows web pages to store structured data and be able to
    * use SQL to manipulate that data asynchronously.
    *
    * Since 1.1.8
    */
    g_object_class_install_property(gobject_class,
                                    PROP_ENABLE_HTML5_DATABASE,
                                    g_param_spec_boolean("enable-html5-database",
                                                         _("Enable HTML5 Database"),
                                                         _("Whether to enable HTML5 database support"),
                                                         TRUE,
                                                         flags));

    /**
    * WebKitWebSettings:enable-html5-local-storage:
    *
    * Whether to enable HTML5 localStorage support. localStorage provides
    * simple synchronous storage access.
    *
    * Since 1.1.8
    */
    g_object_class_install_property(gobject_class,
                                    PROP_ENABLE_HTML5_LOCAL_STORAGE,
                                    g_param_spec_boolean("enable-html5-local-storage",
                                                         _("Enable HTML5 Local Storage"),
                                                         _("Whether to enable HTML5 Local Storage support"),
                                                         TRUE,
                                                         flags));
    /**
    * WebKitWebSettings:enable-xss-auditor
    *
    * Whether to enable the XSS Auditor. This feature filters some kinds of
    * reflective XSS attacks on vulnerable web sites.
    *
    * Since 1.1.11
    */
    g_object_class_install_property(gobject_class,
                                    PROP_ENABLE_XSS_AUDITOR,
                                    g_param_spec_boolean("enable-xss-auditor",
                                                         _("Enable XSS Auditor"),
                                                         _("Whether to enable teh XSS auditor"),
                                                         TRUE,
                                                         flags));

    /**
     * WebKitWebSettings:user-agent:
     *
     * The User-Agent string used by WebKitGtk.
     *
     * This will return a default User-Agent string if a custom string wasn't
     * provided by the application. Setting this property to a NULL value or
     * an empty string will result in the User-Agent string being reset to the
     * default value.
     *
     * Since: 1.1.11
     */
    g_object_class_install_property(gobject_class, PROP_USER_AGENT,
                                    g_param_spec_string("user-agent",
                                                        _("User Agent"),
                                                        _("The User-Agent string used by WebKitGtk"),
                                                        webkit_get_user_agent().utf8().data(),
                                                        flags));

    /**
    * WebKitWebSettings:javascript-can-open-windows-automatically
    *
    * Whether JavaScript can open popup windows automatically without user
    * intervention.
    *
    * Since 1.1.11
    */
    g_object_class_install_property(gobject_class,
                                    PROP_JAVASCRIPT_CAN_OPEN_WINDOWS_AUTOMATICALLY,
                                    g_param_spec_boolean("javascript-can-open-windows-automatically",
                                                         _("JavaScript can open windows automatically"),
                                                         _("Whether JavaScript can open windows automatically"),
                                                         FALSE,
                                                         flags));
    /**
    * WebKitWebSettings:enable-offline-web-application-cache
    *
    * Whether to enable HTML5 offline web application cache support. Offline
    * Web Application Cache ensures web applications are available even when
    * the user is not connected to the network.
    *
    * Since 1.1.13
    */
    g_object_class_install_property(gobject_class,
                                    PROP_ENABLE_OFFLINE_WEB_APPLICATION_CACHE,
                                    g_param_spec_boolean("enable-offline-web-application-cache",
                                                         _("Enable offline web application cache"),
                                                         _("Whether to enable offline web application cache"),
                                                         TRUE,
                                                         flags));



    g_type_class_add_private(klass, sizeof(WebKitWebSettingsPrivate));
}

static void webkit_web_settings_init(WebKitWebSettings* web_settings)
{
    web_settings->priv = WEBKIT_WEB_SETTINGS_GET_PRIVATE(web_settings);
}

static void free_spell_checking_language(gpointer data, gpointer user_data)
{
    SpellLanguage* language = static_cast<SpellLanguage*>(data);
    if (language->config) {
        if (language->speller)
            enchant_broker_free_dict(language->config, language->speller);

        enchant_broker_free(language->config);
    }
    g_slice_free(SpellLanguage, language);
}

static void webkit_web_settings_finalize(GObject* object)
{
    WebKitWebSettings* web_settings = WEBKIT_WEB_SETTINGS(object);
    WebKitWebSettingsPrivate* priv = web_settings->priv;

    g_free(priv->default_encoding);
    g_free(priv->cursive_font_family);
    g_free(priv->default_font_family);
    g_free(priv->fantasy_font_family);
    g_free(priv->monospace_font_family);
    g_free(priv->sans_serif_font_family);
    g_free(priv->serif_font_family);
    g_free(priv->user_stylesheet_uri);
    g_free(priv->spell_checking_languages);

    g_slist_foreach(priv->spell_checking_languages_list, free_spell_checking_language, NULL);
    g_slist_free(priv->spell_checking_languages_list);

    g_free(priv->user_agent);

    G_OBJECT_CLASS(webkit_web_settings_parent_class)->finalize(object);
}

static void webkit_web_settings_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec)
{
    WebKitWebSettings* web_settings = WEBKIT_WEB_SETTINGS(object);
    WebKitWebSettingsPrivate* priv = web_settings->priv;
    SpellLanguage* lang;
    GSList* spellLanguages = NULL;

    switch(prop_id) {
    case PROP_DEFAULT_ENCODING:
        g_free(priv->default_encoding);
        priv->default_encoding = g_strdup(g_value_get_string(value));
        break;
    case PROP_CURSIVE_FONT_FAMILY:
        g_free(priv->cursive_font_family);
        priv->cursive_font_family = g_strdup(g_value_get_string(value));
        break;
    case PROP_DEFAULT_FONT_FAMILY:
        g_free(priv->default_font_family);
        priv->default_font_family = g_strdup(g_value_get_string(value));
        break;
    case PROP_FANTASY_FONT_FAMILY:
        g_free(priv->fantasy_font_family);
        priv->fantasy_font_family = g_strdup(g_value_get_string(value));
        break;
    case PROP_MONOSPACE_FONT_FAMILY:
        g_free(priv->monospace_font_family);
        priv->monospace_font_family = g_strdup(g_value_get_string(value));
        break;
    case PROP_SANS_SERIF_FONT_FAMILY:
        g_free(priv->sans_serif_font_family);
        priv->sans_serif_font_family = g_strdup(g_value_get_string(value));
        break;
    case PROP_SERIF_FONT_FAMILY:
        g_free(priv->serif_font_family);
        priv->serif_font_family = g_strdup(g_value_get_string(value));
        break;
    case PROP_DEFAULT_FONT_SIZE:
        priv->default_font_size = g_value_get_int(value);
        break;
    case PROP_DEFAULT_MONOSPACE_FONT_SIZE:
        priv->default_monospace_font_size = g_value_get_int(value);
        break;
    case PROP_MINIMUM_FONT_SIZE:
        priv->minimum_font_size = g_value_get_int(value);
        break;
    case PROP_MINIMUM_LOGICAL_FONT_SIZE:
        priv->minimum_logical_font_size = g_value_get_int(value);
        break;
    case PROP_ENFORCE_96_DPI:
        priv->enforce_96_dpi = g_value_get_boolean(value);
        break;
    case PROP_AUTO_LOAD_IMAGES:
        priv->auto_load_images = g_value_get_boolean(value);
        break;
    case PROP_AUTO_SHRINK_IMAGES:
        priv->auto_shrink_images = g_value_get_boolean(value);
        break;
    case PROP_PRINT_BACKGROUNDS:
        priv->print_backgrounds = g_value_get_boolean(value);
        break;
    case PROP_ENABLE_SCRIPTS:
        priv->enable_scripts = g_value_get_boolean(value);
        break;
    case PROP_ENABLE_PLUGINS:
        priv->enable_plugins = g_value_get_boolean(value);
        break;
    case PROP_RESIZABLE_TEXT_AREAS:
        priv->resizable_text_areas = g_value_get_boolean(value);
        break;
    case PROP_USER_STYLESHEET_URI:
        g_free(priv->user_stylesheet_uri);
        priv->user_stylesheet_uri = g_strdup(g_value_get_string(value));
        break;
    case PROP_ZOOM_STEP:
        priv->zoom_step = g_value_get_float(value);
        break;
    case PROP_ENABLE_DEVELOPER_EXTRAS:
        priv->enable_developer_extras = g_value_get_boolean(value);
        break;
    case PROP_ENABLE_PRIVATE_BROWSING:
        priv->enable_private_browsing = g_value_get_boolean(value);
        break;
    case PROP_ENABLE_CARET_BROWSING:
        priv->enable_caret_browsing = g_value_get_boolean(value);
        break;
    case PROP_ENABLE_HTML5_DATABASE:
        priv->enable_html5_database = g_value_get_boolean(value);
        break;
    case PROP_ENABLE_HTML5_LOCAL_STORAGE:
        priv->enable_html5_local_storage = g_value_get_boolean(value);
        break;
    case PROP_ENABLE_SPELL_CHECKING:
        priv->enable_spell_checking = g_value_get_boolean(value);
        break;
    case PROP_SPELL_CHECKING_LANGUAGES:
        priv->spell_checking_languages = g_strdup(g_value_get_string(value));

        if (priv->spell_checking_languages) {
            char** langs = g_strsplit(priv->spell_checking_languages, ",", -1);
            for (int i = 0; langs[i]; i++) {
                lang = g_slice_new0(SpellLanguage);
                lang->config = enchant_broker_init();
                lang->speller = enchant_broker_request_dict(lang->config, langs[i]);

                spellLanguages = g_slist_append(spellLanguages, lang);
            }

            g_strfreev(langs);
        } else {
            const char* language = pango_language_to_string(gtk_get_default_language());

            lang = g_slice_new0(SpellLanguage);
            lang->config = enchant_broker_init();
            lang->speller = enchant_broker_request_dict(lang->config, language);

            spellLanguages = g_slist_append(spellLanguages, lang);
        }
        g_slist_foreach(priv->spell_checking_languages_list, free_spell_checking_language, NULL);
        g_slist_free(priv->spell_checking_languages_list);
        priv->spell_checking_languages_list = spellLanguages;
        break;
    case PROP_ENABLE_XSS_AUDITOR:
        priv->enable_xss_auditor = g_value_get_boolean(value);
        break;
    case PROP_USER_AGENT:
        g_free(priv->user_agent);
        if (!g_value_get_string(value) || !strlen(g_value_get_string(value)))
            priv->user_agent = g_strdup(webkit_get_user_agent().utf8().data());
        else
            priv->user_agent = g_strdup(g_value_get_string(value));
        break;
    case PROP_JAVASCRIPT_CAN_OPEN_WINDOWS_AUTOMATICALLY:
        priv->javascript_can_open_windows_automatically = g_value_get_boolean(value);
        break;
    case PROP_ENABLE_OFFLINE_WEB_APPLICATION_CACHE:
        priv->enable_offline_web_application_cache = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void webkit_web_settings_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec)
{
    WebKitWebSettings* web_settings = WEBKIT_WEB_SETTINGS(object);
    WebKitWebSettingsPrivate* priv = web_settings->priv;

    switch (prop_id) {
    case PROP_DEFAULT_ENCODING:
        g_value_set_string(value, priv->default_encoding);
        break;
    case PROP_CURSIVE_FONT_FAMILY:
        g_value_set_string(value, priv->cursive_font_family);
        break;
    case PROP_DEFAULT_FONT_FAMILY:
        g_value_set_string(value, priv->default_font_family);
        break;
    case PROP_FANTASY_FONT_FAMILY:
        g_value_set_string(value, priv->fantasy_font_family);
        break;
    case PROP_MONOSPACE_FONT_FAMILY:
        g_value_set_string(value, priv->monospace_font_family);
        break;
    case PROP_SANS_SERIF_FONT_FAMILY:
        g_value_set_string(value, priv->sans_serif_font_family);
        break;
    case PROP_SERIF_FONT_FAMILY:
        g_value_set_string(value, priv->serif_font_family);
        break;
    case PROP_DEFAULT_FONT_SIZE:
        g_value_set_int(value, priv->default_font_size);
        break;
    case PROP_DEFAULT_MONOSPACE_FONT_SIZE:
        g_value_set_int(value, priv->default_monospace_font_size);
        break;
    case PROP_MINIMUM_FONT_SIZE:
        g_value_set_int(value, priv->minimum_font_size);
        break;
    case PROP_MINIMUM_LOGICAL_FONT_SIZE:
        g_value_set_int(value, priv->minimum_logical_font_size);
        break;
    case PROP_ENFORCE_96_DPI:
        g_value_set_boolean(value, priv->enforce_96_dpi);
        break;
    case PROP_AUTO_LOAD_IMAGES:
        g_value_set_boolean(value, priv->auto_load_images);
        break;
    case PROP_AUTO_SHRINK_IMAGES:
        g_value_set_boolean(value, priv->auto_shrink_images);
        break;
    case PROP_PRINT_BACKGROUNDS:
        g_value_set_boolean(value, priv->print_backgrounds);
        break;
    case PROP_ENABLE_SCRIPTS:
        g_value_set_boolean(value, priv->enable_scripts);
        break;
    case PROP_ENABLE_PLUGINS:
        g_value_set_boolean(value, priv->enable_plugins);
        break;
    case PROP_RESIZABLE_TEXT_AREAS:
        g_value_set_boolean(value, priv->resizable_text_areas);
        break;
    case PROP_USER_STYLESHEET_URI:
        g_value_set_string(value, priv->user_stylesheet_uri);
        break;
    case PROP_ZOOM_STEP:
        g_value_set_float(value, priv->zoom_step);
        break;
    case PROP_ENABLE_DEVELOPER_EXTRAS:
        g_value_set_boolean(value, priv->enable_developer_extras);
        break;
    case PROP_ENABLE_PRIVATE_BROWSING:
        g_value_set_boolean(value, priv->enable_private_browsing);
        break;
    case PROP_ENABLE_CARET_BROWSING:
        g_value_set_boolean(value, priv->enable_caret_browsing);
        break;
    case PROP_ENABLE_HTML5_DATABASE:
        g_value_set_boolean(value, priv->enable_html5_database);
        break;
    case PROP_ENABLE_HTML5_LOCAL_STORAGE:
        g_value_set_boolean(value, priv->enable_html5_local_storage);
        break;
    case PROP_ENABLE_SPELL_CHECKING:
        g_value_set_boolean(value, priv->enable_spell_checking);
        break;
    case PROP_SPELL_CHECKING_LANGUAGES:
        g_value_set_string(value, priv->spell_checking_languages);
        break;
    case PROP_ENABLE_XSS_AUDITOR:
        g_value_set_boolean(value, priv->enable_xss_auditor);
        break;
    case PROP_USER_AGENT:
        g_value_set_string(value, priv->user_agent);
        break;
    case PROP_JAVASCRIPT_CAN_OPEN_WINDOWS_AUTOMATICALLY:
        g_value_set_boolean(value, priv->javascript_can_open_windows_automatically);
        break;
   case PROP_ENABLE_OFFLINE_WEB_APPLICATION_CACHE:
        g_value_set_boolean(value, priv->enable_offline_web_application_cache);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

/**
 * webkit_web_settings_new:
 *
 * Creates a new #WebKitWebSettings instance with default values. It must
 * be manually attached to a WebView.
 *
 * Returns: a new #WebKitWebSettings instance
 **/
WebKitWebSettings* webkit_web_settings_new()
{
    return WEBKIT_WEB_SETTINGS(g_object_new(WEBKIT_TYPE_WEB_SETTINGS, NULL));
}

/**
 * webkit_web_settings_copy:
 *
 * Copies an existing #WebKitWebSettings instance.
 *
 * Returns: a new #WebKitWebSettings instance
 **/
WebKitWebSettings* webkit_web_settings_copy(WebKitWebSettings* web_settings)
{
    WebKitWebSettingsPrivate* priv = web_settings->priv;

    WebKitWebSettings* copy = WEBKIT_WEB_SETTINGS(g_object_new(WEBKIT_TYPE_WEB_SETTINGS,
                 "default-encoding", priv->default_encoding,
                 "cursive-font-family", priv->cursive_font_family,
                 "default-font-family", priv->default_font_family,
                 "fantasy-font-family", priv->fantasy_font_family,
                 "monospace-font-family", priv->monospace_font_family,
                 "sans-serif-font-family", priv->sans_serif_font_family,
                 "serif-font-family", priv->serif_font_family,
                 "default-font-size", priv->default_font_size,
                 "default-monospace-font-size", priv->default_monospace_font_size,
                 "minimum-font-size", priv->minimum_font_size,
                 "minimum-logical-font-size", priv->minimum_logical_font_size,
                 "auto-load-images", priv->auto_load_images,
                 "auto-shrink-images", priv->auto_shrink_images,
                 "print-backgrounds", priv->print_backgrounds,
                 "enable-scripts", priv->enable_scripts,
                 "enable-plugins", priv->enable_plugins,
                 "resizable-text-areas", priv->resizable_text_areas,
                 "user-stylesheet-uri", priv->user_stylesheet_uri,
                 "zoom-step", priv->zoom_step,
                 "enable-developer-extras", priv->enable_developer_extras,
                 "enable-private-browsing", priv->enable_private_browsing,
                 "enable-spell-checking", priv->enable_spell_checking,
                 "spell-checking-languages", priv->spell_checking_languages,
                 "spell-checking-languages-list", priv->spell_checking_languages_list,
                 "enable-caret-browsing", priv->enable_caret_browsing,
                 "enable-html5-database", priv->enable_html5_database,
                 "enable-html5-local-storage", priv->enable_html5_local_storage,
                 "enable-xss-auditor", priv->enable_xss_auditor,
                 "user-agent", webkit_web_settings_get_user_agent(web_settings),
                 "javascript-can-open-windows-automatically", priv->javascript_can_open_windows_automatically,
                 "enable-offline-web-application-cache", priv->enable_offline_web_application_cache,
                 NULL));

    return copy;
}

/**
 * webkit_web_settings_add_extra_plugin_directory:
 * @web_view: a #WebKitWebView
 * @directory: the directory to add
 *
 * Adds the @directory to paths where @web_view will search for plugins.
 *
 * Since: 1.0.3
 */
void webkit_web_settings_add_extra_plugin_directory(WebKitWebView* webView, const gchar* directory)
{
    g_return_if_fail(WEBKIT_IS_WEB_VIEW(webView));

    PluginDatabase::installedPlugins()->addExtraPluginDirectory(filenameToString(directory));
}

/**
 * webkit_web_settings_get_spell_languages:
 * @web_view: a #WebKitWebView
 *
 * Internal use only. Retrieves a GSList of SpellLanguages from the
 * #WebKitWebSettings of @web_view.
 *
 * Since: 1.1.6
 */
GSList* webkit_web_settings_get_spell_languages(WebKitWebView *web_view)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_VIEW(web_view), 0);

    WebKitWebSettings* settings = webkit_web_view_get_settings(web_view);
    WebKitWebSettingsPrivate* priv = settings->priv;
    GSList* list = priv->spell_checking_languages_list;

    return list;
}

/**
 * webkit_web_settings_get_user_agent:
 * @web_settings: a #WebKitWebSettings
 *
 * Returns the User-Agent string currently used by the web view(s) associated
 * with the @web_settings.
 *
 * Since: 1.1.11
 */
G_CONST_RETURN gchar* webkit_web_settings_get_user_agent(WebKitWebSettings* webSettings)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_SETTINGS(webSettings), NULL);

    WebKitWebSettingsPrivate* priv = webSettings->priv;

    return priv->user_agent;
}
