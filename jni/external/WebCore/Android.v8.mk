## 
##
## Copyright 2007, The Android Open Source Project
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##

# This comment block is read by tools/webkitsync/diff.cpp
# Don't remove it or move it. 
#
# The following files are intentionally not included
# LOCAL_SRC_FILES_EXCLUDED := \
#	DerivedSources.cpp \
#	WebCorePrefix.cpp \
#	accessibility/*.cpp \
#	bridge/test*.cpp \
#	css/CSSGrammar.y \
#	dom/XMLTokenizerQt.cpp \
#	editing/BackForwardListChromium.cpp \
#	editing/SmartReplace*.cpp \
#	history/BackForwardListChromium.cpp \
#	html/FileList.cpp \
#	loader/CachedXBLDocument.cpp \
#	loader/CachedXSLStyleSheet.cpp \
#	loader/FTP*.cpp \
#	loader/UserStyleSheetLoader.cpp \
#	loader/icon/IconDatabaseNone.cpp \
#	page/AXObjectCache.cpp \
#	page/Accessibility*.cpp \
#	page/InspectorController.cpp \
#	page/JavaScript*.cpp \
#	platform/ThreadingNone.cpp \
#	platform/graphics/FloatPoint3D.cpp \
#	rendering/RenderThemeChromium*.cpp \
#	rendering/RenderThemeSafari.cpp \
#	rendering/RenderThemeWin.cpp \
#	svg/SVGAllInOne.cpp \
#	xml/Access*.cpp \
#	xml/NativeXPathNSResolver.cpp \
#	xml/XPath* \
#	xml/XSL*.cpp \

# This comment block is read by tools/webkitsync/diff.cpp
# Don't remove it or move it. 
#
# The following directory wildcard matches are intentionally not included
# If an entry starts with '/', any subdirectory may match
# If an entry starts with '^', the first directory must match
# LOCAL_DIR_WILDCARD_EXCLUDED := \
#	/cairo/* \
#	/cf/* \
#	/cg/* \
#	/chromium/* \
#	/curl/* \
#	/gtk/* \
#	/image-decoders/* \
#	^inspector/* \
#	^ksvg2/* \
#	^loader\/appcache/* \
#	^loader\/archive/* \
#	/mac/* \
#	^manual-tests/* \
#	/objc/* \
#	^platform\/graphics\/filters/* \
#	^platform\/network\/soup/* \
#	/qt/* \
#	/skia/* \
#	/symbian/* \
#	/v8/* \
#	/win/* \
#	^wml/* \
#	/wx/* \

# This comment block is read by tools/webkitsync/diff.cpp
# Don't remove it or move it. 
# If you edit it, keep it in alphabetical order
#
# These files are Android extensions, or exclusion exceptions
# LOCAL_ANDROID_SRC_FILES_INCLUDED := \
#	dom/Touch*.cpp \
#	platform/graphics/skia/NativeImageSkia.cpp \
#	platform/image-decoders/skia/GIFImage*.cpp \

# The remainder of the file is read by tools/webkitsync/diff.cpp
# If you edit it, keep it in alphabetical order
LOCAL_SRC_FILES := \
	css/CSSBorderImageValue.cpp \
	css/CSSCanvasValue.cpp \
	css/CSSCharsetRule.cpp \
	css/CSSComputedStyleDeclaration.cpp \
	css/CSSCursorImageValue.cpp \
	css/CSSFontFace.cpp \
	css/CSSFontFaceRule.cpp \
	css/CSSFontFaceSource.cpp \
	css/CSSFontFaceSrcValue.cpp \
	css/CSSFontSelector.cpp \
	css/CSSFunctionValue.cpp \
	css/CSSGradientValue.cpp \
	css/CSSHelper.cpp \
	css/CSSImageGeneratorValue.cpp \
	css/CSSImageValue.cpp \
	css/CSSImportRule.cpp \
	css/CSSInheritedValue.cpp \
	css/CSSInitialValue.cpp \
	css/CSSMediaRule.cpp \
	css/CSSMutableStyleDeclaration.cpp \
	css/CSSPageRule.cpp \
	css/CSSParser.cpp \
	css/CSSParserValues.cpp \
	css/CSSPrimitiveValue.cpp \
	css/CSSProperty.cpp \
	css/CSSPropertyLonghand.cpp \
	css/CSSReflectValue.cpp \
	css/CSSRule.cpp \
	css/CSSRuleList.cpp \
	css/CSSSegmentedFontFace.cpp \
	css/CSSSelector.cpp \
	css/CSSSelectorList.cpp \
	css/CSSStyleDeclaration.cpp \
	css/CSSStyleRule.cpp \
	css/CSSStyleSelector.cpp \
	css/CSSStyleSheet.cpp \
	css/CSSTimingFunctionValue.cpp \
	css/CSSUnicodeRangeValue.cpp \
	css/CSSValueList.cpp \
	css/CSSVariableDependentValue.cpp \
	css/CSSVariablesDeclaration.cpp \
	css/CSSVariablesRule.cpp \
	css/FontFamilyValue.cpp \
	css/FontValue.cpp \
	css/Media.cpp \
	css/MediaFeatureNames.cpp \
	css/MediaList.cpp \
	css/MediaQuery.cpp \
	css/MediaQueryEvaluator.cpp \
	css/MediaQueryExp.cpp \
	css/RGBColor.cpp \
    
ifeq ($(ENABLE_SVG), true)
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) \
	css/SVGCSSComputedStyleDeclaration.cpp \
	css/SVGCSSParser.cpp \
	css/SVGCSSStyleSelector.cpp
endif

LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) \
	css/ShadowValue.cpp \
	css/StyleBase.cpp \
	css/StyleList.cpp \
	css/StyleSheet.cpp \
	css/StyleSheetList.cpp \
	css/WebKitCSSKeyframeRule.cpp \
	css/WebKitCSSKeyframesRule.cpp \
	css/WebKitCSSMatrix.cpp \
	css/WebKitCSSTransformValue.cpp \
	\
	dom/ActiveDOMObject.cpp \
	dom/Attr.cpp \
	dom/Attribute.cpp \
	dom/BeforeTextInsertedEvent.cpp \
	dom/BeforeUnloadEvent.cpp \
	dom/CDATASection.cpp \
	dom/CSSMappedAttributeDeclaration.cpp \
	dom/CharacterData.cpp \
	dom/CheckedRadioButtons.cpp \
	dom/ChildNodeList.cpp \
	dom/ClassNames.cpp \
	dom/ClassNodeList.cpp \
	dom/ClientRect.cpp \
	dom/ClientRectList.cpp \
	dom/Clipboard.cpp \
	dom/ClipboardEvent.cpp \
	dom/Comment.cpp \
	dom/ContainerNode.cpp \
	dom/DOMImplementation.cpp \
	dom/Document.cpp \
	dom/DocumentFragment.cpp \
	dom/DocumentType.cpp \
	dom/DynamicNodeList.cpp \
	dom/EditingText.cpp \
	dom/Element.cpp \
	dom/Entity.cpp \
	dom/EntityReference.cpp \
	dom/ErrorEvent.cpp \
	dom/Event.cpp \
	dom/EventNames.cpp \
	dom/EventTarget.cpp \
	dom/ExceptionBase.cpp \
	dom/ExceptionCode.cpp \
	dom/InputElement.cpp \
	dom/KeyboardEvent.cpp \
	dom/MappedAttribute.cpp \
	dom/MessageChannel.cpp \
	dom/MessageEvent.cpp \
	dom/MessagePort.cpp \
	dom/MessagePortChannel.cpp \
	dom/MouseEvent.cpp \
	dom/MouseRelatedEvent.cpp \
	dom/MutationEvent.cpp \
	dom/NameNodeList.cpp \
	dom/NamedAttrMap.cpp \
	dom/NamedMappedAttrMap.cpp \
	dom/Node.cpp \
	dom/NodeFilter.cpp \
	dom/NodeFilterCondition.cpp \
	dom/NodeIterator.cpp \
	dom/Notation.cpp \
	dom/OptionElement.cpp \
	dom/OptionGroupElement.cpp \
	dom/OverflowEvent.cpp \
	dom/Position.cpp \
	dom/PositionIterator.cpp \
	dom/ProcessingInstruction.cpp \
	dom/ProgressEvent.cpp \
	dom/QualifiedName.cpp \
	dom/Range.cpp \
	dom/RegisteredEventListener.cpp \
	dom/ScriptElement.cpp \
	dom/ScriptExecutionContext.cpp \
	dom/SelectElement.cpp \
	dom/SelectorNodeList.cpp \
	dom/StaticNodeList.cpp \
	dom/StyleElement.cpp \
	dom/StyledElement.cpp \
	dom/TagNodeList.cpp \
	dom/Text.cpp \
	dom/TextEvent.cpp \
	dom/Touch.cpp \
	dom/TouchEvent.cpp \
	dom/TouchList.cpp \
	dom/Traversal.cpp \
	dom/TreeWalker.cpp \
	dom/UIEvent.cpp \
	dom/UIEventWithKeyState.cpp \
	dom/WebKitAnimationEvent.cpp \
	dom/WebKitTransitionEvent.cpp \
	dom/WheelEvent.cpp \
	dom/XMLTokenizer.cpp \
	dom/XMLTokenizerLibxml2.cpp \
	dom/XMLTokenizerScope.cpp \
	\
	dom/default/PlatformMessagePortChannel.cpp \
	\
	editing/AppendNodeCommand.cpp \
	editing/ApplyStyleCommand.cpp \
	editing/BreakBlockquoteCommand.cpp \
	editing/CompositeEditCommand.cpp \
	editing/CreateLinkCommand.cpp \
	editing/DeleteButton.cpp \
	editing/DeleteButtonController.cpp \
	editing/DeleteFromTextNodeCommand.cpp \
	editing/DeleteSelectionCommand.cpp \
	editing/EditCommand.cpp \
	editing/Editor.cpp \
	editing/EditorCommand.cpp \
	editing/FormatBlockCommand.cpp \
	editing/HTMLInterchange.cpp \
	editing/IndentOutdentCommand.cpp \
	editing/InsertIntoTextNodeCommand.cpp \
	editing/InsertLineBreakCommand.cpp \
	editing/InsertListCommand.cpp \
	editing/InsertNodeBeforeCommand.cpp \
	editing/InsertParagraphSeparatorCommand.cpp \
	editing/InsertTextCommand.cpp \
	editing/JoinTextNodesCommand.cpp \
	editing/MergeIdenticalElementsCommand.cpp \
	editing/ModifySelectionListLevel.cpp \
	editing/MoveSelectionCommand.cpp \
	editing/RemoveCSSPropertyCommand.cpp \
	editing/RemoveFormatCommand.cpp \
	editing/RemoveNodeCommand.cpp \
	editing/RemoveNodePreservingChildrenCommand.cpp \
	editing/ReplaceNodeWithSpanCommand.cpp \
	editing/ReplaceSelectionCommand.cpp \
	editing/SelectionController.cpp \
	editing/SetNodeAttributeCommand.cpp \
	editing/SplitElementCommand.cpp \
	editing/SplitTextNodeCommand.cpp \
	editing/SplitTextNodeContainingElementCommand.cpp \
	editing/TextIterator.cpp \
	editing/TypingCommand.cpp \
	editing/UnlinkCommand.cpp \
	editing/VisiblePosition.cpp \
	editing/VisibleSelection.cpp \
	editing/WrapContentsInDummySpanCommand.cpp \
	\
	editing/android/EditorAndroid.cpp \
	editing/htmlediting.cpp \
	editing/markup.cpp \
	editing/visible_units.cpp \
	\
	history/BackForwardList.cpp \
	history/CachedFrame.cpp \
	history/CachedPage.cpp \
	history/HistoryItem.cpp \
	history/PageCache.cpp \
	\
	html/CollectionCache.cpp \
	html/File.cpp \
	html/FormDataList.cpp \
	html/HTMLCollection.cpp \
	html/HTMLDocument.cpp \
        html/HTMLElementsAllInOne.cpp \
	html/HTMLFormCollection.cpp \
	html/HTMLImageLoader.cpp \
	html/HTMLNameCollection.cpp \
	html/HTMLOptionsCollection.cpp \
	html/HTMLParser.cpp \
	html/HTMLParserErrorCodes.cpp \
	html/HTMLTableRowsCollection.cpp \
	html/HTMLTokenizer.cpp \
	html/HTMLViewSourceDocument.cpp \
	html/ImageData.cpp \
	html/PreloadScanner.cpp \
	html/TimeRanges.cpp \
	html/ValidityState.cpp \
	\
	html/canvas/CanvasGradient.cpp \
	html/canvas/CanvasPattern.cpp \
	html/canvas/CanvasPixelArray.cpp \
	html/canvas/CanvasRenderingContext2D.cpp \
	html/canvas/CanvasStyle.cpp \
	\
	loader/Cache.cpp \
	loader/CachedCSSStyleSheet.cpp \
	loader/CachedFont.cpp \
	loader/CachedImage.cpp \
	loader/CachedResource.cpp \
	loader/CachedResourceClientWalker.cpp \
	loader/CachedResourceHandle.cpp \
	loader/CachedScript.cpp \
	loader/CrossOriginAccessControl.cpp \
	loader/CrossOriginPreflightResultCache.cpp \
	loader/DocLoader.cpp \
	loader/DocumentLoader.cpp \
	loader/DocumentThreadableLoader.cpp \
	loader/WorkerThreadableLoader.cpp \
	loader/FormState.cpp \
	loader/FrameLoader.cpp \
	loader/ImageDocument.cpp \
	loader/ImageLoader.cpp \
	loader/MainResourceLoader.cpp \
	loader/MediaDocument.cpp \
	loader/NavigationAction.cpp \
	loader/NetscapePlugInStreamLoader.cpp \
	loader/PlaceholderDocument.cpp \
	loader/PluginDocument.cpp \
	loader/ProgressTracker.cpp \
	loader/Request.cpp \
	loader/ResourceLoader.cpp \
	loader/SubresourceLoader.cpp \
	loader/TextDocument.cpp \
	loader/TextResourceDecoder.cpp \
	loader/ThreadableLoader.cpp \
	loader/appcache/ApplicationCache.cpp \
	loader/appcache/ApplicationCacheGroup.cpp \
	loader/appcache/ApplicationCacheHost.cpp \
	loader/appcache/ApplicationCacheResource.cpp \
	loader/appcache/ApplicationCacheStorage.cpp \
	loader/appcache/DOMApplicationCache.cpp \
	loader/appcache/ManifestParser.cpp \
	\
	loader/icon/IconDatabase.cpp \
	loader/icon/IconFetcher.cpp \
	loader/icon/IconLoader.cpp \
	loader/icon/IconRecord.cpp \
	loader/icon/PageURLRecord.cpp \
	\
	loader/loader.cpp \
	\
	page/BarInfo.cpp \
	page/Chrome.cpp \
	page/Console.cpp \
	page/ContextMenuController.cpp \
	page/DOMSelection.cpp \
	page/DOMTimer.cpp \
	page/DOMWindow.cpp \
	page/DragController.cpp \
	page/EventHandler.cpp \
	page/FocusController.cpp \
	page/Frame.cpp \
	page/FrameTree.cpp \
	page/FrameView.cpp \
	page/Geolocation.cpp \
	page/History.cpp \
	page/Location.cpp \
	page/MouseEventWithHitTestResults.cpp \
	page/Navigator.cpp \
	page/NavigatorBase.cpp \
	page/Page.cpp \
	page/PageGroup.cpp \
	page/PageGroupLoadDeferrer.cpp \
	page/PrintContext.cpp \
	page/Screen.cpp \
	page/SecurityOrigin.cpp \
	page/Settings.cpp \
	page/WindowFeatures.cpp \
	page/WorkerNavigator.cpp \
	page/XSSAuditor.cpp \
	\
	page/android/DragControllerAndroid.cpp \
	page/android/EventHandlerAndroid.cpp \
	page/android/InspectorControllerAndroid.cpp \
	\
	page/animation/AnimationBase.cpp \
	page/animation/AnimationController.cpp \
	page/animation/CompositeAnimation.cpp \
	page/animation/ImplicitAnimation.cpp \
	page/animation/KeyframeAnimation.cpp \
	\
	platform/Arena.cpp \
	platform/ContentType.cpp \
	platform/ContextMenu.cpp \
	platform/CrossThreadCopier.cpp \
	platform/DeprecatedPtrListImpl.cpp \
	platform/DragData.cpp \
	platform/DragImage.cpp \
	platform/FileChooser.cpp \
	platform/GeolocationService.cpp \
	platform/KURL.cpp \
	platform/KURLGoogle.cpp \
	platform/Length.cpp \
	platform/LinkHash.cpp \
	platform/Logging.cpp \
	platform/MIMETypeRegistry.cpp \
	platform/ScrollView.cpp \
	platform/Scrollbar.cpp \
	platform/ScrollbarThemeComposite.cpp \
	platform/SharedBuffer.cpp \
	platform/Theme.cpp \
	platform/ThreadGlobalData.cpp \
	platform/ThreadTimers.cpp \
	platform/Timer.cpp \
	platform/Widget.cpp \
	\
	platform/android/ClipboardAndroid.cpp \
	platform/android/CursorAndroid.cpp \
	platform/android/DragDataAndroid.cpp \
	platform/android/EventLoopAndroid.cpp \
	platform/android/FileChooserAndroid.cpp \
	platform/android/FileSystemAndroid.cpp \
	platform/android/GeolocationServiceAndroid.cpp \
	platform/android/KeyEventAndroid.cpp \
	platform/android/LocalizedStringsAndroid.cpp \
	platform/android/PopupMenuAndroid.cpp \
	platform/android/RenderThemeAndroid.cpp \
	platform/android/ScreenAndroid.cpp \
	platform/android/ScrollViewAndroid.cpp \
	platform/android/SearchPopupMenuAndroid.cpp \
	platform/android/SharedTimerAndroid.cpp \
	platform/android/SoundAndroid.cpp \
	platform/android/SSLKeyGeneratorAndroid.cpp \
	platform/android/SystemTimeAndroid.cpp \
	platform/android/TemporaryLinkStubs.cpp \
	platform/android/WidgetAndroid.cpp \
	\
	platform/text/android/TextBreakIteratorInternalICU.cpp \
	\
	platform/animation/Animation.cpp \
	platform/animation/AnimationList.cpp \
	\
	platform/graphics/BitmapImage.cpp \
	platform/graphics/Color.cpp \
	platform/graphics/FloatPoint.cpp \
	platform/graphics/FloatPoint3D.cpp \
	platform/graphics/FloatQuad.cpp \
	platform/graphics/FloatRect.cpp \
	platform/graphics/FloatSize.cpp \
	platform/graphics/Font.cpp \
	platform/graphics/FontCache.cpp \
	platform/graphics/FontData.cpp \
	platform/graphics/FontDescription.cpp \
	platform/graphics/FontFallbackList.cpp \
	platform/graphics/FontFamily.cpp \
	platform/graphics/FontFastPath.cpp \
	platform/graphics/GeneratedImage.cpp \
	platform/graphics/GlyphPageTreeNode.cpp \
	platform/graphics/GlyphWidthMap.cpp \
	platform/graphics/Gradient.cpp \
	platform/graphics/GraphicsContext.cpp \
	platform/graphics/GraphicsLayer.cpp \
	platform/graphics/GraphicsTypes.cpp \
	platform/graphics/Image.cpp \
	platform/graphics/IntRect.cpp \
	platform/graphics/MediaPlayer.cpp \
	platform/graphics/Path.cpp \
	platform/graphics/PathTraversalState.cpp \
	platform/graphics/Pattern.cpp \
	platform/graphics/Pen.cpp \
	platform/graphics/SegmentedFontData.cpp \
	platform/graphics/SimpleFontData.cpp \
	platform/graphics/StringTruncator.cpp \
	platform/graphics/WidthIterator.cpp \
	\
	platform/graphics/android/BitmapAllocatorAndroid.cpp \
	platform/graphics/android/FontAndroid.cpp \
	platform/graphics/android/FontCacheAndroid.cpp \
	platform/graphics/android/FontCustomPlatformData.cpp \
	platform/graphics/android/FontDataAndroid.cpp \
	platform/graphics/android/FontPlatformDataAndroid.cpp \
	platform/graphics/android/GlyphMapAndroid.cpp \
	platform/graphics/android/GradientAndroid.cpp \
	platform/graphics/android/GraphicsContextAndroid.cpp \
	platform/graphics/android/ImageAndroid.cpp \
	platform/graphics/android/ImageBufferAndroid.cpp \
	platform/graphics/android/ImageSourceAndroid.cpp \
	platform/graphics/android/PathAndroid.cpp \
	platform/graphics/android/PatternAndroid.cpp \
	platform/graphics/android/PlatformGraphicsContext.cpp \
	platform/graphics/android/SharedBufferStream.cpp \
	platform/graphics/android/android_graphics.cpp \

ifeq ($(ENABLE_SVG), true)
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) \
	platform/graphics/filters/FEBlend.cpp \
	platform/graphics/filters/FEColorMatrix.cpp \
	platform/graphics/filters/FEComponentTransfer.cpp \
	platform/graphics/filters/FEComposite.cpp
endif

LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) \
	platform/graphics/skia/FloatPointSkia.cpp \
	platform/graphics/skia/FloatRectSkia.cpp \
	platform/graphics/skia/IntPointSkia.cpp \
	platform/graphics/skia/IntRectSkia.cpp \
	platform/graphics/skia/NativeImageSkia.cpp \
	platform/graphics/skia/SkiaUtils.cpp \
	platform/graphics/skia/TransformationMatrixSkia.cpp \
	\
	platform/graphics/transforms/Matrix3DTransformOperation.cpp \
	platform/graphics/transforms/MatrixTransformOperation.cpp \
	platform/graphics/transforms/PerspectiveTransformOperation.cpp \
	platform/graphics/transforms/RotateTransformOperation.cpp \
	platform/graphics/transforms/ScaleTransformOperation.cpp \
	platform/graphics/transforms/SkewTransformOperation.cpp \
	platform/graphics/transforms/TransformOperations.cpp \
	platform/graphics/transforms/TransformationMatrix.cpp \
	platform/graphics/transforms/TranslateTransformOperation.cpp \
	\
	platform/image-decoders/gif/GIFImageDecoder.cpp \
	platform/image-decoders/gif/GIFImageReader.cpp \
	platform/image-decoders/skia/ImageDecoderSkia.cpp \
	\
	platform/mock/GeolocationServiceMock.cpp \
	\
	platform/network/AuthenticationChallengeBase.cpp \
	platform/network/Credential.cpp \
	platform/network/FormData.cpp \
	platform/network/FormDataBuilder.cpp \
	platform/network/HTTPHeaderMap.cpp \
	platform/network/HTTPParsers.cpp \
	platform/network/NetworkStateNotifier.cpp \
	platform/network/ProtectionSpace.cpp \
	platform/network/ResourceErrorBase.cpp \
	platform/network/ResourceHandle.cpp \
	platform/network/ResourceRequestBase.cpp \
	platform/network/ResourceResponseBase.cpp \
	\
	platform/network/android/Cookie.cpp \
	platform/network/android/ResourceHandleAndroid.cpp \
	platform/network/android/NetworkStateNotifierAndroid.cpp \
	\
	platform/posix/FileSystemPOSIX.cpp \
  \
	platform/sql/SQLValue.cpp \
	platform/sql/SQLiteAuthorizer.cpp \
	platform/sql/SQLiteDatabase.cpp \
	platform/sql/SQLiteFileSystem.cpp \
	platform/sql/SQLiteStatement.cpp \
	platform/sql/SQLiteTransaction.cpp \
	\
	platform/text/AtomicString.cpp \
	platform/text/Base64.cpp \
	platform/text/BidiContext.cpp \
	platform/text/CString.cpp \
	platform/text/RegularExpression.cpp \
	platform/text/SegmentedString.cpp \
	platform/text/String.cpp \
	platform/text/StringBuilder.cpp \
	platform/text/StringImpl.cpp \
	platform/text/TextBoundariesICU.cpp \
	platform/text/TextBreakIteratorICU.cpp \
	platform/text/TextCodec.cpp \
	platform/text/TextCodecICU.cpp \
	platform/text/TextCodecLatin1.cpp \
	platform/text/TextCodecUTF16.cpp \
	platform/text/TextCodecUserDefined.cpp \
	platform/text/TextEncoding.cpp \
	platform/text/TextEncodingDetectorICU.cpp \
	platform/text/TextEncodingRegistry.cpp \
	platform/text/TextStream.cpp \
	platform/text/UnicodeRange.cpp \
	\
	plugins/MimeType.cpp \
	plugins/MimeTypeArray.cpp \
	plugins/Plugin.cpp \
	plugins/PluginArray.cpp \
	plugins/PluginData.cpp \
	plugins/PluginDatabase.cpp \
	plugins/PluginInfoStore.cpp \
	plugins/PluginMainThreadScheduler.cpp \
	plugins/PluginPackage.cpp \
	plugins/PluginStream.cpp \
	plugins/PluginView.cpp \
	plugins/npapi.cpp \
	\
	plugins/android/PluginDataAndroid.cpp \
	plugins/android/PluginPackageAndroid.cpp \
	plugins/android/PluginViewAndroid.cpp \
	\
	rendering/AutoTableLayout.cpp \
	rendering/CounterNode.cpp \
	rendering/EllipsisBox.cpp \
	rendering/FixedTableLayout.cpp \
	rendering/HitTestResult.cpp \
	rendering/InlineBox.cpp \
	rendering/InlineFlowBox.cpp \
	rendering/InlineTextBox.cpp \
	rendering/LayoutState.cpp \
	rendering/MediaControlElements.cpp \
	rendering/PointerEventsHitRules.cpp \
	rendering/RenderApplet.cpp \
	rendering/RenderArena.cpp \
	rendering/RenderBR.cpp \
	rendering/RenderBlock.cpp \
	rendering/RenderBlockLineLayout.cpp \
	rendering/RenderBox.cpp \
	rendering/RenderBoxModelObject.cpp \
	rendering/RenderButton.cpp \
	rendering/RenderCounter.cpp \
	rendering/RenderFieldset.cpp \
	rendering/RenderFileUploadControl.cpp \
	rendering/RenderFlexibleBox.cpp \
	rendering/RenderForeignObject.cpp \
	rendering/RenderFrame.cpp \
	rendering/RenderFrameSet.cpp \
	rendering/RenderHTMLCanvas.cpp \
	rendering/RenderImage.cpp \
	rendering/RenderImageGeneratedContent.cpp \
	rendering/RenderInline.cpp \
	rendering/RenderLayer.cpp \
	rendering/RenderLayerBacking.cpp \
	rendering/RenderLayerCompositor.cpp \
	rendering/RenderLineBoxList.cpp \
	rendering/RenderListBox.cpp \
	rendering/RenderListItem.cpp \
	rendering/RenderListMarker.cpp \
	rendering/RenderMarquee.cpp \
	rendering/RenderMedia.cpp \
	rendering/RenderMenuList.cpp \
	rendering/RenderObject.cpp \
	rendering/RenderObjectChildList.cpp \
	rendering/RenderPart.cpp \
	rendering/RenderPartObject.cpp \
	rendering/RenderPath.cpp \
	rendering/RenderReplaced.cpp \
	rendering/RenderReplica.cpp \
	rendering/RenderVideo.cpp
    
ifeq ($(ENABLE_SVG), true)
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) \
	rendering/RenderSVGBlock.cpp \
	rendering/RenderSVGContainer.cpp \
	rendering/RenderSVGGradientStop.cpp \
	rendering/RenderSVGHiddenContainer.cpp \
	rendering/RenderSVGImage.cpp \
	rendering/RenderSVGInline.cpp \
	rendering/RenderSVGInlineText.cpp \
	rendering/RenderSVGRoot.cpp \
	rendering/RenderSVGTSpan.cpp \
	rendering/RenderSVGText.cpp \
	rendering/RenderSVGTextPath.cpp \
	rendering/RenderSVGTransformableContainer.cpp \
	rendering/RenderSVGViewportContainer.cpp
endif

LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) \
	rendering/RenderScrollbar.cpp \
	rendering/RenderScrollbarPart.cpp \
	rendering/RenderScrollbarTheme.cpp \
	rendering/RenderSlider.cpp \
	rendering/RenderTable.cpp \
	rendering/RenderTableCell.cpp \
	rendering/RenderTableCol.cpp \
	rendering/RenderTableRow.cpp \
	rendering/RenderTableSection.cpp \
	rendering/RenderText.cpp \
	rendering/RenderTextControl.cpp \
	rendering/RenderTextControlMultiLine.cpp \
	rendering/RenderTextControlSingleLine.cpp \
	rendering/RenderTextFragment.cpp \
	rendering/RenderTheme.cpp \
	rendering/RenderTreeAsText.cpp \
	rendering/RenderView.cpp \
	rendering/RenderWidget.cpp \
	rendering/RenderWordBreak.cpp \
	rendering/RootInlineBox.cpp \
    
ifeq ($(ENABLE_SVG), true)
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) \
	rendering/SVGCharacterLayoutInfo.cpp \
	rendering/SVGInlineFlowBox.cpp \
	rendering/SVGInlineTextBox.cpp \
	rendering/SVGRenderSupport.cpp \
	rendering/SVGRenderTreeAsText.cpp \
	rendering/SVGRootInlineBox.cpp
endif

LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) \
	rendering/ScrollBehavior.cpp \
	rendering/TextControlInnerElements.cpp \
	rendering/TransformState.cpp \
	rendering/break_lines.cpp \
	\
	rendering/style/BindingURI.cpp \
	rendering/style/ContentData.cpp \
	rendering/style/CounterDirectives.cpp \
	rendering/style/FillLayer.cpp \
	rendering/style/KeyframeList.cpp \
	rendering/style/NinePieceImage.cpp \
	rendering/style/RenderStyle.cpp \
    
ifeq ($(ENABLE_SVG), true)
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) \
	rendering/style/SVGRenderStyle.cpp \
	rendering/style/SVGRenderStyleDefs.cpp
endif

LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) \
	rendering/style/ShadowData.cpp \
	rendering/style/StyleBackgroundData.cpp \
	rendering/style/StyleBoxData.cpp \
	rendering/style/StyleCachedImage.cpp \
	rendering/style/StyleFlexibleBoxData.cpp \
	rendering/style/StyleGeneratedImage.cpp \
	rendering/style/StyleInheritedData.cpp \
	rendering/style/StyleMarqueeData.cpp \
	rendering/style/StyleMultiColData.cpp \
	rendering/style/StyleRareInheritedData.cpp \
	rendering/style/StyleRareNonInheritedData.cpp \
	rendering/style/StyleSurroundData.cpp \
	rendering/style/StyleTransformData.cpp \
	rendering/style/StyleVisualData.cpp \
	\
	storage/ChangeVersionWrapper.cpp \
	storage/Database.cpp \
	storage/DatabaseAuthorizer.cpp \
	storage/DatabaseTask.cpp \
	storage/DatabaseThread.cpp \
	storage/DatabaseTracker.cpp \
	storage/LocalStorageTask.cpp \
	storage/LocalStorageThread.cpp \
	storage/OriginQuotaManager.cpp \
	storage/OriginUsageRecord.cpp \
	storage/SQLResultSet.cpp \
	storage/SQLResultSetRowList.cpp \
	storage/SQLStatement.cpp \
	storage/SQLTransaction.cpp \
	storage/Storage.cpp \
	storage/StorageAreaImpl.cpp \
	storage/StorageAreaSync.cpp \
	storage/StorageEvent.cpp \
	storage/StorageMap.cpp \
	storage/StorageNamespace.cpp \
	storage/StorageNamespaceImpl.cpp \
	storage/StorageSyncManager.cpp \
    
ifeq ($(ENABLE_SVG), true)
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) \
	svg/ColorDistance.cpp \
	svg/Filter.cpp \
	svg/FilterEffect.cpp \
	svg/SVGAElement.cpp \
	svg/SVGAltGlyphElement.cpp \
	svg/SVGAngle.cpp \
	svg/SVGAnimateColorElement.cpp \
	svg/SVGAnimateElement.cpp \
	svg/SVGAnimateMotionElement.cpp \
	svg/SVGAnimateTransformElement.cpp \
	svg/SVGAnimatedPathData.cpp \
	svg/SVGAnimatedPoints.cpp \
	svg/SVGAnimationElement.cpp \
	svg/SVGCircleElement.cpp \
	svg/SVGClipPathElement.cpp \
	svg/SVGColor.cpp \
	svg/SVGComponentTransferFunctionElement.cpp \
	svg/SVGCursorElement.cpp \
	svg/SVGDefinitionSrcElement.cpp \
	svg/SVGDefsElement.cpp \
	svg/SVGDescElement.cpp \
	svg/SVGDocument.cpp \
	svg/SVGDocumentExtensions.cpp \
	svg/SVGElement.cpp \
	svg/SVGElementInstance.cpp \
	svg/SVGElementInstanceList.cpp \
	svg/SVGEllipseElement.cpp \
	svg/SVGExternalResourcesRequired.cpp \
	svg/SVGFEBlendElement.cpp \
	svg/SVGFEColorMatrixElement.cpp \
	svg/SVGFEComponentTransferElement.cpp \
	svg/SVGFECompositeElement.cpp \
	svg/SVGFEDiffuseLightingElement.cpp \
	svg/SVGFEDisplacementMapElement.cpp \
	svg/SVGFEDistantLightElement.cpp \
	svg/SVGFEFloodElement.cpp \
	svg/SVGFEFuncAElement.cpp \
	svg/SVGFEFuncBElement.cpp \
	svg/SVGFEFuncGElement.cpp \
	svg/SVGFEFuncRElement.cpp \
	svg/SVGFEGaussianBlurElement.cpp \
	svg/SVGFEImageElement.cpp \
	svg/SVGFELightElement.cpp \
	svg/SVGFEMergeElement.cpp \
	svg/SVGFEMergeNodeElement.cpp \
	svg/SVGFEOffsetElement.cpp \
	svg/SVGFEPointLightElement.cpp \
	svg/SVGFESpecularLightingElement.cpp \
	svg/SVGFESpotLightElement.cpp \
	svg/SVGFETileElement.cpp \
	svg/SVGFETurbulenceElement.cpp \
	svg/SVGFilterElement.cpp \
	svg/SVGFilterPrimitiveStandardAttributes.cpp \
	svg/SVGFitToViewBox.cpp \
	svg/SVGFont.cpp \
	svg/SVGFontData.cpp \
	svg/SVGFontElement.cpp \
	svg/SVGFontFaceElement.cpp \
	svg/SVGFontFaceFormatElement.cpp \
	svg/SVGFontFaceNameElement.cpp \
	svg/SVGFontFaceSrcElement.cpp \
	svg/SVGFontFaceUriElement.cpp \
	svg/SVGForeignObjectElement.cpp \
	svg/SVGGElement.cpp \
	svg/SVGGlyphElement.cpp \
	svg/SVGGradientElement.cpp \
	svg/SVGHKernElement.cpp \
	svg/SVGImageElement.cpp \
	svg/SVGImageLoader.cpp \
	svg/SVGLangSpace.cpp \
	svg/SVGLength.cpp \
	svg/SVGLengthList.cpp \
	svg/SVGLineElement.cpp \
	svg/SVGLinearGradientElement.cpp \
	svg/SVGLocatable.cpp \
	svg/SVGMPathElement.cpp \
	svg/SVGMarkerElement.cpp \
	svg/SVGMaskElement.cpp \
	svg/SVGMetadataElement.cpp \
	svg/SVGMissingGlyphElement.cpp \
	svg/SVGNumberList.cpp \
	svg/SVGPaint.cpp \
	svg/SVGParserUtilities.cpp \
	svg/SVGPathElement.cpp \
	svg/SVGPathSegArc.cpp \
	svg/SVGPathSegClosePath.cpp \
	svg/SVGPathSegCurvetoCubic.cpp \
	svg/SVGPathSegCurvetoCubicSmooth.cpp \
	svg/SVGPathSegCurvetoQuadratic.cpp \
	svg/SVGPathSegCurvetoQuadraticSmooth.cpp \
	svg/SVGPathSegLineto.cpp \
	svg/SVGPathSegLinetoHorizontal.cpp \
	svg/SVGPathSegLinetoVertical.cpp \
	svg/SVGPathSegList.cpp \
	svg/SVGPathSegMoveto.cpp \
	svg/SVGPatternElement.cpp \
	svg/SVGPointList.cpp \
	svg/SVGPolyElement.cpp \
	svg/SVGPolygonElement.cpp \
	svg/SVGPolylineElement.cpp \
	svg/SVGPreserveAspectRatio.cpp \
	svg/SVGRadialGradientElement.cpp \
	svg/SVGRectElement.cpp \
	svg/SVGSVGElement.cpp \
	svg/SVGScriptElement.cpp \
	svg/SVGSetElement.cpp \
	svg/SVGStopElement.cpp \
	svg/SVGStringList.cpp \
	svg/SVGStylable.cpp \
	svg/SVGStyleElement.cpp \
	svg/SVGStyledElement.cpp \
	svg/SVGStyledLocatableElement.cpp \
	svg/SVGStyledTransformableElement.cpp \
	svg/SVGSwitchElement.cpp \
	svg/SVGSymbolElement.cpp \
	svg/SVGTRefElement.cpp \
	svg/SVGTSpanElement.cpp \
	svg/SVGTests.cpp \
	svg/SVGTextContentElement.cpp \
	svg/SVGTextElement.cpp \
	svg/SVGTextPathElement.cpp \
	svg/SVGTextPositioningElement.cpp \
	svg/SVGTitleElement.cpp \
	svg/SVGTransform.cpp \
	svg/SVGTransformDistance.cpp \
	svg/SVGTransformList.cpp \
	svg/SVGTransformable.cpp \
	svg/SVGURIReference.cpp \
	svg/SVGUseElement.cpp \
	svg/SVGViewElement.cpp \
	svg/SVGViewSpec.cpp \
	svg/SVGZoomAndPan.cpp \
	svg/SVGZoomEvent.cpp \
	\
	svg/animation/SMILTime.cpp \
	svg/animation/SMILTimeContainer.cpp \
	svg/animation/SVGSMILElement.cpp \
	\
	svg/graphics/SVGImage.cpp \
	svg/graphics/SVGPaintServer.cpp \
	svg/graphics/SVGPaintServerGradient.cpp \
	svg/graphics/SVGPaintServerLinearGradient.cpp \
	svg/graphics/SVGPaintServerPattern.cpp \
	svg/graphics/SVGPaintServerRadialGradient.cpp \
	svg/graphics/SVGPaintServerSolid.cpp \
	svg/graphics/SVGResource.cpp \
	svg/graphics/SVGResourceClipper.cpp \
	svg/graphics/SVGResourceFilter.cpp \
	svg/graphics/SVGResourceMarker.cpp \
	svg/graphics/SVGResourceMasker.cpp \
	\
	svg/graphics/filters/SVGFEConvolveMatrix.cpp \
	svg/graphics/filters/SVGFEDiffuseLighting.cpp \
	svg/graphics/filters/SVGFEDisplacementMap.cpp \
	svg/graphics/filters/SVGFEFlood.cpp \
	svg/graphics/filters/SVGFEGaussianBlur.cpp \
	svg/graphics/filters/SVGFEImage.cpp \
	svg/graphics/filters/SVGFEMerge.cpp \
	svg/graphics/filters/SVGFEMorphology.cpp \
	svg/graphics/filters/SVGFEOffset.cpp \
	svg/graphics/filters/SVGFESpecularLighting.cpp \
	svg/graphics/filters/SVGFETile.cpp \
	svg/graphics/filters/SVGFETurbulence.cpp \
	svg/graphics/filters/SVGFilterEffect.cpp \
	svg/graphics/filters/SVGLightSource.cpp \
	\
	svg/graphics/skia/SVGResourceFilterSkia.cpp
endif

LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) \
	workers/AbstractWorker.cpp \
	workers/DedicatedWorkerContext.cpp \
	workers/DedicatedWorkerThread.cpp \
	workers/DefaultSharedWorkerRepository.cpp \
	workers/SharedWorker.cpp \
	workers/SharedWorkerContext.cpp \
	workers/SharedWorkerThread.cpp \
	workers/Worker.cpp \
	workers/WorkerContext.cpp \
	workers/WorkerLocation.cpp \
	workers/WorkerMessagingProxy.cpp \
	workers/WorkerRunLoop.cpp \
	workers/WorkerScriptLoader.cpp \
	workers/WorkerThread.cpp \
	\
	xml/DOMParser.cpp \
	xml/XMLHttpRequest.cpp \
	xml/XMLHttpRequestUpload.cpp \
	xml/XMLSerializer.cpp
