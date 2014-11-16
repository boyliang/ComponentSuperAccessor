/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
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
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

WebInspector.BreakpointsSidebarPane = function()
{
    WebInspector.SidebarPane.call(this, WebInspector.UIString("Breakpoints"));

    this.breakpoints = {};

    this.listElement = document.createElement("ol");
    this.listElement.className = "breakpoint-list";

    this.emptyElement = document.createElement("div");
    this.emptyElement.className = "info";
    this.emptyElement.textContent = WebInspector.UIString("No Breakpoints");

    this.bodyElement.appendChild(this.emptyElement);
}

WebInspector.BreakpointsSidebarPane.prototype = {
    addBreakpoint: function(breakpoint)
    {
        if (this.breakpoints[breakpoint.id])
            return;

        this.breakpoints[breakpoint.id] = breakpoint;

        breakpoint.addEventListener("enabled", this._breakpointEnableChanged, this);
        breakpoint.addEventListener("disabled", this._breakpointEnableChanged, this);
        breakpoint.addEventListener("text-changed", this._breakpointTextChanged, this);

        this._appendBreakpointElement(breakpoint);

        if (this.emptyElement.parentElement) {
            this.bodyElement.removeChild(this.emptyElement);
            this.bodyElement.appendChild(this.listElement);
        }

        if (!InspectorController.debuggerEnabled() || !breakpoint.sourceID)
            return;

        if (breakpoint.enabled)
            InspectorController.addBreakpoint(breakpoint.sourceID, breakpoint.line);
    },

    _appendBreakpointElement: function(breakpoint)
    {
        function checkboxClicked()
        {
            breakpoint.enabled = !breakpoint.enabled;
        }

        function labelClicked()
        {
            var script = WebInspector.panels.scripts.scriptOrResourceForID(breakpoint.sourceID);
            if (script)
                WebInspector.panels.scripts.showScript(script, breakpoint.line);
        }

        var breakpointElement = document.createElement("li");
        breakpoint._breakpointListElement = breakpointElement;
        breakpointElement._breakpointObject = breakpoint;

        var checkboxElement = document.createElement("input");
        checkboxElement.className = "checkbox-elem";
        checkboxElement.type = "checkbox";
        checkboxElement.checked = breakpoint.enabled;
        checkboxElement.addEventListener("click", checkboxClicked, false);
        breakpointElement.appendChild(checkboxElement);

        var labelElement = document.createElement("a");
        labelElement.textContent = breakpoint.label;
        labelElement.addEventListener("click", labelClicked, false);
        breakpointElement.appendChild(labelElement);

        var sourceTextElement = document.createElement("div");
        sourceTextElement.textContent = breakpoint.sourceText;
        sourceTextElement.className = "source-text";
        breakpointElement.appendChild(sourceTextElement);

        var currentElement = this.listElement.firstChild;
        while (currentElement) {
            var currentBreak = currentElement._breakpointObject;
            if (currentBreak.url > breakpoint.url) {
                this.listElement.insertBefore(breakpointElement, currentElement);
                return;
            } else if (currentBreak.url == breakpoint.url && currentBreak.line > breakpoint.line) {
                this.listElement.insertBefore(breakpointElement, currentElement);
                return;
            }
            currentElement = currentElement.nextSibling;
        }
        this.listElement.appendChild(breakpointElement);
    },

    removeBreakpoint: function(breakpoint)
    {
        if (!this.breakpoints[breakpoint.id])
            return;
        delete this.breakpoints[breakpoint.id];

        breakpoint.removeEventListener("enabled", null, this);
        breakpoint.removeEventListener("disabled", null, this);
        breakpoint.removeEventListener("text-changed", null, this);

        var element = breakpoint._breakpointListElement;
        element.parentElement.removeChild(element);

        if (!this.listElement.firstChild) {
            this.bodyElement.removeChild(this.listElement);
            this.bodyElement.appendChild(this.emptyElement);
        }

        if (!InspectorController.debuggerEnabled() || !breakpoint.sourceID)
            return;

        InspectorController.removeBreakpoint(breakpoint.sourceID, breakpoint.line);
    },

    _breakpointEnableChanged: function(event)
    {
        var breakpoint = event.target;

        var checkbox = breakpoint._breakpointListElement.firstChild;
        checkbox.checked = breakpoint.enabled;

        if (!InspectorController.debuggerEnabled() || !breakpoint.sourceID)
            return;

        if (breakpoint.enabled)
            InspectorController.addBreakpoint(breakpoint.sourceID, breakpoint.line);
        else
            InspectorController.removeBreakpoint(breakpoint.sourceID, breakpoint.line);
    },

    _breakpointTextChanged: function(event)
    {
        var breakpoint = event.target;

        var sourceTextElement = breakpoint._breakpointListElement.firstChild.nextSibling.nextSibling;
        sourceTextElement.textContent = breakpoint.sourceText;
    }
}

WebInspector.BreakpointsSidebarPane.prototype.__proto__ = WebInspector.SidebarPane.prototype;
