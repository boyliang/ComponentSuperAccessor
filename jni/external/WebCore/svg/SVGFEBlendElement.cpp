/*
    Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005, 2006 Rob Buis <buis@kde.org>

    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#if ENABLE(SVG) && ENABLE(FILTERS)
#include "SVGFEBlendElement.h"

#include "MappedAttribute.h"
#include "SVGResourceFilter.h"

namespace WebCore {

SVGFEBlendElement::SVGFEBlendElement(const QualifiedName& tagName, Document* doc)
    : SVGFilterPrimitiveStandardAttributes(tagName, doc)
    , m_in1(this, SVGNames::inAttr)
    , m_in2(this, SVGNames::in2Attr)
    , m_mode(this, SVGNames::modeAttr, FEBLEND_MODE_NORMAL)
{
}

SVGFEBlendElement::~SVGFEBlendElement()
{
}

void SVGFEBlendElement::parseMappedAttribute(MappedAttribute* attr)
{
    const String& value = attr->value();
    if (attr->name() == SVGNames::modeAttr) {
        if (value == "normal")
            setModeBaseValue(FEBLEND_MODE_NORMAL);
        else if (value == "multiply")
            setModeBaseValue(FEBLEND_MODE_MULTIPLY);
        else if (value == "screen")
            setModeBaseValue(FEBLEND_MODE_SCREEN);
        else if (value == "darken")
            setModeBaseValue(FEBLEND_MODE_DARKEN);
        else if (value == "lighten")
            setModeBaseValue(FEBLEND_MODE_LIGHTEN);
    } else if (attr->name() == SVGNames::inAttr)
        setIn1BaseValue(value);
    else if (attr->name() == SVGNames::in2Attr)
        setIn2BaseValue(value);
    else
        SVGFilterPrimitiveStandardAttributes::parseMappedAttribute(attr);
}

bool SVGFEBlendElement::build(SVGResourceFilter* filterResource)
{
    FilterEffect* input1 = filterResource->builder()->getEffectById(in1());
    FilterEffect* input2 = filterResource->builder()->getEffectById(in2());

    if (!input1 || !input2)
        return false;

    RefPtr<FilterEffect> effect = FEBlend::create(input1, input2, static_cast<BlendModeType>(mode()));
    filterResource->addFilterEffect(this, effect.release());

    return true;
}

}

#endif // ENABLE(SVG)

// vim:ts=4:noet
