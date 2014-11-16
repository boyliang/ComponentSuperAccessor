/*
 * Copyright (C) 2007 Apple Inc.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2008 Collabora Ltd.
 * Copyright (C) 2008, 2009 Google Inc.
 * Copyright (C) 2009 Kenneth Rohde Christiansen
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
 *
 */

#include "config.h"
#include "RenderThemeChromiumLinux.h"

#include "Color.h"
#include "CSSValueKeywords.h"
#include "RenderObject.h"
#include "UserAgentStyleSheets.h"

namespace WebCore {

PassRefPtr<RenderTheme> RenderThemeChromiumLinux::create()
{
    return adoptRef(new RenderThemeChromiumLinux());
}

PassRefPtr<RenderTheme> RenderTheme::themeForPage(Page* page)
{
    static RenderTheme* rt = RenderThemeChromiumLinux::create().releaseRef();
    return rt;
}

RenderThemeChromiumLinux::RenderThemeChromiumLinux()
{
}

RenderThemeChromiumLinux::~RenderThemeChromiumLinux()
{
}

Color RenderThemeChromiumLinux::systemColor(int cssValueId) const
{
    static const Color linuxButtonGrayColor(0xffdddddd);

    if (cssValueId == CSSValueButtonface)
        return linuxButtonGrayColor;
    return RenderTheme::systemColor(cssValueId);
}

String RenderThemeChromiumLinux::extraDefaultStyleSheet()
{
    return RenderThemeChromiumSkia::extraDefaultStyleSheet() +
           String(themeChromiumLinuxUserAgentStyleSheet, sizeof(themeChromiumLinuxUserAgentStyleSheet));
}

bool RenderThemeChromiumLinux::controlSupportsTints(const RenderObject* o) const
{
    return isEnabled(o);
}

Color RenderThemeChromiumLinux::activeListBoxSelectionBackgroundColor() const
{
    return Color(0x28, 0x28, 0x28);
}

Color RenderThemeChromiumLinux::activeListBoxSelectionForegroundColor() const
{
    return Color::black;
}

Color RenderThemeChromiumLinux::inactiveListBoxSelectionBackgroundColor() const
{
    return Color(0xc8, 0xc8, 0xc8);
}

Color RenderThemeChromiumLinux::inactiveListBoxSelectionForegroundColor() const
{
    return Color(0x32, 0x32, 0x32);
}

bool RenderThemeChromiumLinux::supportsControlTints() const
{
    return true;
}

} // namespace WebCore
