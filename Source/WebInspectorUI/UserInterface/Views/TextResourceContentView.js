/*
 * Copyright (C) 2013, 2015 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

WebInspector.TextResourceContentView = class TextResourceContentView extends WebInspector.ResourceContentView
{
    constructor(resource)
    {
        super(resource, "text");

        resource.addEventListener(WebInspector.SourceCode.Event.ContentDidChange, this._sourceCodeContentDidChange, this);

        this._textEditor = new WebInspector.SourceCodeTextEditor(resource);
        this._textEditor.addEventListener(WebInspector.TextEditor.Event.ExecutionLineNumberDidChange, this._executionLineNumberDidChange, this);
        this._textEditor.addEventListener(WebInspector.TextEditor.Event.NumberOfSearchResultsDidChange, this._numberOfSearchResultsDidChange, this);
        this._textEditor.addEventListener(WebInspector.TextEditor.Event.ContentDidChange, this._textEditorContentDidChange, this);
        this._textEditor.addEventListener(WebInspector.TextEditor.Event.FormattingDidChange, this._textEditorFormattingDidChange, this);
        this._textEditor.addEventListener(WebInspector.SourceCodeTextEditor.Event.ContentWillPopulate, this._contentWillPopulate, this);
        this._textEditor.addEventListener(WebInspector.SourceCodeTextEditor.Event.ContentDidPopulate, this._contentDidPopulate, this);

        WebInspector.probeManager.addEventListener(WebInspector.ProbeManager.Event.ProbeSetAdded, this._probeSetsChanged, this);
        WebInspector.probeManager.addEventListener(WebInspector.ProbeManager.Event.ProbeSetRemoved, this._probeSetsChanged, this);

        var toolTip = WebInspector.UIString("Pretty print");
        var activatedToolTip = WebInspector.UIString("Original formatting");
        this._prettyPrintButtonNavigationItem = new WebInspector.ActivateButtonNavigationItem("pretty-print", toolTip, activatedToolTip, "Images/NavigationItemCurleyBraces.svg", 13, 13);
        this._prettyPrintButtonNavigationItem.addEventListener(WebInspector.ButtonNavigationItem.Event.Clicked, this._togglePrettyPrint, this);
        this._prettyPrintButtonNavigationItem.enabled = false; // Enabled when the text editor is populated with content.

        var toolTipTypes = WebInspector.UIString("Show type information");
        var activatedToolTipTypes = WebInspector.UIString("Hide type information");
        this._showTypesButtonNavigationItem = new WebInspector.ActivateButtonNavigationItem("show-types", toolTipTypes, activatedToolTipTypes, "Images/NavigationItemTypes.svg", 13, 14);
        this._showTypesButtonNavigationItem.addEventListener(WebInspector.ButtonNavigationItem.Event.Clicked, this._toggleTypeAnnotations, this);
        this._showTypesButtonNavigationItem.enabled = false;

        WebInspector.showJavaScriptTypeInformationSetting.addEventListener(WebInspector.Setting.Event.Changed, this._showJavaScriptTypeInformationSettingChanged, this);
    }

    // Public

    get navigationItems()
    {
        return [this._prettyPrintButtonNavigationItem, this._showTypesButtonNavigationItem];
    }

    get managesOwnIssues()
    {
        // SourceCodeTextEditor manages the issues, we don't need ResourceContentView doing it.
        return true;
    }

    get textEditor()
    {
        return this._textEditor;
    }

    get supplementalRepresentedObjects()
    {
        var objects = WebInspector.probeManager.probeSets.filter(function(probeSet) {
            return this._resource.url === probeSet.breakpoint.url;
        }, this);

        // If the SourceCodeTextEditor has an executionLineNumber, we can assume
        // it is always the active call frame.
        if (!isNaN(this._textEditor.executionLineNumber))
            objects.push(WebInspector.debuggerManager.activeCallFrame);

        return objects;
    }

    revealPosition(position, textRangeToSelect, forceUnformatted)
    {
        this._textEditor.revealPosition(position, textRangeToSelect, forceUnformatted);
    }

    shown()
    {
        super.shown();

        this._textEditor.shown();
    }

    hidden()
    {
        super.hidden();

        this._textEditor.hidden();
    }

    closed()
    {
        super.closed();

        this.resource.removeEventListener(null, null, this);
        WebInspector.probeManager.removeEventListener(null, null, this);
        WebInspector.showJavaScriptTypeInformationSetting.removeEventListener(null, null, this);

        this._textEditor.close();
    }

    get supportsSave()
    {
        return true;
    }

    get saveData()
    {
        return {url: this.resource.url, content: this._textEditor.string};
    }

    get supportsSearch()
    {
        return true;
    }

    get numberOfSearchResults()
    {
        return this._textEditor.numberOfSearchResults;
    }

    get hasPerformedSearch()
    {
        return this._textEditor.currentSearchQuery !== null;
    }

    set automaticallyRevealFirstSearchResult(reveal)
    {
        this._textEditor.automaticallyRevealFirstSearchResult = reveal;
    }

    performSearch(query)
    {
        this._textEditor.performSearch(query);
    }

    searchCleared()
    {
        this._textEditor.searchCleared();
    }

    searchQueryWithSelection()
    {
        return this._textEditor.searchQueryWithSelection();
    }

    revealPreviousSearchResult(changeFocus)
    {
        this._textEditor.revealPreviousSearchResult(changeFocus);
    }

    revealNextSearchResult(changeFocus)
    {
        this._textEditor.revealNextSearchResult(changeFocus);
    }

    // Private

    _contentWillPopulate(event)
    {
        if (this._textEditor.parentView === this)
            return;

        // Check the MIME-type for CSS since Resource.Type.Stylesheet also includes XSL, which we can't edit yet.
        if (this.resource.type === WebInspector.Resource.Type.Stylesheet && this.resource.syntheticMIMEType === "text/css")
            this._textEditor.readOnly = false;

        // Allow editing any local file since edits can be saved and reloaded right from the Inspector.
        if (this.resource.urlComponents.scheme === "file")
            this._textEditor.readOnly = false;

        this.element.removeChildren();
        this.addSubview(this._textEditor);
    }

    _contentDidPopulate(event)
    {
        this._prettyPrintButtonNavigationItem.enabled = this._textEditor.canBeFormatted();
        this._showTypesButtonNavigationItem.enabled = this._textEditor.canShowTypeAnnotations();
        this._showTypesButtonNavigationItem.activated = WebInspector.showJavaScriptTypeInformationSetting.value;
    }

    _togglePrettyPrint(event)
    {
        var activated = !this._prettyPrintButtonNavigationItem.activated;
        this._textEditor.updateFormattedState(activated);
    }

    _toggleTypeAnnotations(event)
    {
        this._textEditor.toggleTypeAnnotations();
    }

    _showJavaScriptTypeInformationSettingChanged(event)
    {
        this._showTypesButtonNavigationItem.activated = WebInspector.showJavaScriptTypeInformationSetting.value;
    }

    _textEditorFormattingDidChange(event)
    {
        this._prettyPrintButtonNavigationItem.activated = this._textEditor.formatted;
    }

    _sourceCodeContentDidChange(event)
    {
        if (this._ignoreSourceCodeContentDidChangeEvent)
            return;

        this._textEditor.string = this.resource.currentRevision.content;
    }

    _textEditorContentDidChange(event)
    {
        this._ignoreSourceCodeContentDidChangeEvent = true;
        WebInspector.branchManager.currentBranch.revisionForRepresentedObject(this.resource).content = this._textEditor.string;
        delete this._ignoreSourceCodeContentDidChangeEvent;
    }

    _executionLineNumberDidChange(event)
    {
        this.dispatchEventToListeners(WebInspector.ContentView.Event.SupplementalRepresentedObjectsDidChange);
    }

    _numberOfSearchResultsDidChange(event)
    {
        this.dispatchEventToListeners(WebInspector.ContentView.Event.NumberOfSearchResultsDidChange);
    }

    _probeSetsChanged(event)
    {
        var breakpoint = event.data.probeSet.breakpoint;
        if (breakpoint.sourceCodeLocation.sourceCode === this.resource)
            this.dispatchEventToListeners(WebInspector.ContentView.Event.SupplementalRepresentedObjectsDidChange);
    }
};
