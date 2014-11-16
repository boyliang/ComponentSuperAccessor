/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
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

#include "config.h"
#include "JSImageConstructor.h"

#include "HTMLImageElement.h"
#include "HTMLNames.h"
#include "JSHTMLImageElement.h"
#include "JSNode.h"
#include "ScriptExecutionContext.h"

using namespace JSC;

namespace WebCore {

ASSERT_CLASS_FITS_IN_CELL(JSImageConstructor);

const ClassInfo JSImageConstructor::s_info = { "ImageConstructor", 0, 0, 0 };

JSImageConstructor::JSImageConstructor(ExecState* exec, JSDOMGlobalObject* globalObject)
    : DOMConstructorWithDocument(JSImageConstructor::createStructure(globalObject->objectPrototype()), globalObject)
{
    putDirect(exec->propertyNames().prototype, JSHTMLImageElementPrototype::self(exec, globalObject), None);
}

static JSObject* constructImage(ExecState* exec, JSObject* constructor, const ArgList& args)
{
    bool widthSet = false;
    bool heightSet = false;
    int width = 0;
    int height = 0;
    if (args.size() > 0) {
        widthSet = true;
        width = args.at(0).toInt32(exec);
    }
    if (args.size() > 1) {
        heightSet = true;
        height = args.at(1).toInt32(exec);
    }

    JSImageConstructor* jsConstructor = static_cast<JSImageConstructor*>(constructor);
    Document* document = jsConstructor->document();
    if (!document)
        return throwError(exec, ReferenceError, "Image constructor associated document is unavailable");

    // Calling toJS on the document causes the JS document wrapper to be
    // added to the window object. This is done to ensure that JSDocument::mark
    // will be called (which will cause the image element to be marked if necessary).
    toJS(exec, jsConstructor->globalObject(), document);

    RefPtr<HTMLImageElement> image = new HTMLImageElement(HTMLNames::imgTag, document);
    if (widthSet)
        image->setWidth(width);
    if (heightSet)
        image->setHeight(height);
    return asObject(toJS(exec, jsConstructor->globalObject(), image.release()));
}

ConstructType JSImageConstructor::getConstructData(ConstructData& constructData)
{
    constructData.native.function = constructImage;
    return ConstructTypeHost;
}

} // namespace WebCore
