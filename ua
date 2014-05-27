# HG changeset patch
# Parent 52632d896ef75107104861ae63083c452f25b161
# User Brian Grinstead <bgrinstead@mozilla.com>
Bug 935803 - Display browser styles in the rule view (UA style sheet rules)

diff --git a/browser/app/profile/firefox.js b/browser/app/profile/firefox.js
--- a/browser/app/profile/firefox.js
+++ b/browser/app/profile/firefox.js
@@ -1269,16 +1269,18 @@ pref("devtools.inspector.enabled", true)
 pref("devtools.inspector.activeSidebar", "ruleview");
 // Enable the markup preview
 pref("devtools.inspector.markupPreview", false);
 pref("devtools.inspector.remote", false);
 // Expand pseudo-elements by default in the rule-view
 pref("devtools.inspector.show_pseudo_elements", true);
 // The default size for image preview tooltips in the rule-view/computed-view/markup-view
 pref("devtools.inspector.imagePreviewTooltipSize", 300);
+// Enable user agent style inspection in rule-view
+pref("devtools.inspector.showUserAgentStyles", false);
 
 // DevTools default color unit
 pref("devtools.defaultColorUnit", "hex");
 
 // Enable the Responsive UI tool
 pref("devtools.responsiveUI.no-reload-notification", false);
 
 // Enable the Debugger
diff --git a/browser/devtools/framework/toolbox-options.xul b/browser/devtools/framework/toolbox-options.xul
--- a/browser/devtools/framework/toolbox-options.xul
+++ b/browser/devtools/framework/toolbox-options.xul
@@ -39,16 +39,19 @@
         <label value="&options.commonPrefs.label;"/>
         <vbox id="commonprefs-options" class="options-groupbox">
           <checkbox label="&options.enablePersistentLogs.label;"
                     tooltiptext="&options.enablePersistentLogs.tooltip;"
                     data-pref="devtools.webconsole.persistlog"/>
         </vbox>
         <label value="&options.context.inspector;"/>
         <vbox id="inspector-options" class="options-groupbox">
+          <checkbox label="&options.showUserAgentStyles.label;"
+                    tooltiptext="&options.showUserAgentStyles.tooltip;"
+                    data-pref="devtools.inspector.showUserAgentStyles"/>
           <hbox align="center">
             <label value="&options.defaultColorUnit.label;"
               control="defaultColorUnitMenuList"
               accesskey="&options.defaultColorUnit.accesskey;"/>
             <menulist id="defaultColorUnitMenuList"
                       label="&options.defaultColorUnit.label;"
                       data-pref="devtools.defaultColorUnit">
               <menupopup>
diff --git a/browser/devtools/styleinspector/rule-view.js b/browser/devtools/styleinspector/rule-view.js
--- a/browser/devtools/styleinspector/rule-view.js
+++ b/browser/devtools/styleinspector/rule-view.js
@@ -17,16 +17,17 @@ const {OutputParser} = require("devtools
 const {PrefObserver, PREF_ORIG_SOURCES} = require("devtools/styleeditor/utils");
 const {parseSingleValue, parseDeclarations} = require("devtools/styleinspector/css-parsing-utils");
 
 Cu.import("resource://gre/modules/Services.jsm");
 Cu.import("resource://gre/modules/XPCOMUtils.jsm");
 
 const HTML_NS = "http://www.w3.org/1999/xhtml";
 const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";
+const PREF_UA_STYLES = "devtools.inspector.showUserAgentStyles";
 
 /**
  * These regular expressions are adapted from firebug's css.js, and are
  * used to parse CSSStyleDeclaration's cssText attribute.
  */
 
 // Used to split on css line separators
 const CSS_LINE_RE = /(?:[^;\(]*(?:\([^\)]*?\))?[^;\(]*)*;?/g;
@@ -107,23 +108,26 @@ function createDummyDocument() {
  *        The element whose style we are viewing.
  * @param {object} aStore
  *        The ElementStyle can use this object to store metadata
  *        that might outlast the rule view, particularly the current
  *        set of disabled properties.
  * @param {PageStyleFront} aPageStyle
  *        Front for the page style actor that will be providing
  *        the style information.
+ * @param {bool} aShowUserAgentStyles
+ *        Should user agent styles be inspected?
  *
  * @constructor
  */
-function ElementStyle(aElement, aStore, aPageStyle) {
+function ElementStyle(aElement, aStore, aPageStyle, aShowUserAgentStyles) {
   this.element = aElement;
   this.store = aStore || {};
   this.pageStyle = aPageStyle;
+  this.showUserAgentStyles = aShowUserAgentStyles;
 
   // We don't want to overwrite this.store.userProperties so we only create it
   // if it doesn't already exist.
   if (!("userProperties" in this.store)) {
     this.store.userProperties = new UserProperties();
   }
 
   if (!("disabled" in this.store)) {
@@ -180,17 +184,18 @@ ElementStyle.prototype = {
    * Upon completion, this.rules[] will hold a list of Rule objects.
    *
    * Returns a promise that will be resolved when the elementStyle is
    * ready.
    */
   populate: function() {
     let populated = this.pageStyle.getApplied(this.element, {
       inherited: true,
-      matchedSelectors: true
+      matchedSelectors: true,
+      filter: this.showUserAgentStyles ? "ua" : undefined,
     }).then(entries => {
       // Make sure the dummy element has been created before continuing...
       return this.dummyElementPromise.then(() => {
         if (this.populated != populated) {
           // Don't care anymore.
           return;
         }
 
@@ -396,25 +401,27 @@ ElementStyle.prototype = {
  *
  * @param {ElementStyle} aElementStyle
  *        The ElementStyle to which this rule belongs.
  * @param {object} aOptions
  *        The information used to construct this rule.  Properties include:
  *          rule: A StyleRuleActor
  *          inherited: An element this rule was inherited from.  If omitted,
  *            the rule applies directly to the current element.
+ *          isSystem: Is this a user agent style?
  * @constructor
  */
 function Rule(aElementStyle, aOptions) {
   this.elementStyle = aElementStyle;
   this.domRule = aOptions.rule || null;
   this.style = aOptions.rule;
   this.matchedSelectors = aOptions.matchedSelectors || [];
   this.pseudoElement = aOptions.pseudoElement || "";
 
+  this.isSystem = aOptions.isSystem;
   this.inherited = aOptions.inherited || null;
   this._modificationDepth = 0;
 
   if (this.domRule) {
     let parentRule = this.domRule.parentRule;
     if (parentRule && parentRule.type == Ci.nsIDOMCSSRule.MEDIA_RULE) {
       this.mediaText = parentRule.mediaText;
     }
@@ -1077,16 +1084,18 @@ function CssRuleView(aInspector, aDoc, a
 
   this._handlePrefChange = this._handlePrefChange.bind(this);
   gDevTools.on("pref-changed", this._handlePrefChange);
 
   this._onSourcePrefChanged = this._onSourcePrefChanged.bind(this);
   this._prefObserver = new PrefObserver("devtools.");
   this._prefObserver.on(PREF_ORIG_SOURCES, this._onSourcePrefChanged);
 
+  this.showUserAgentStyles = Services.prefs.getBoolPref(PREF_UA_STYLES);
+
   let options = {
     autoSelect: true,
     theme: "auto"
   };
   this.popup = new AutocompletePopup(aDoc.defaultView.parent.document, options);
 
   // Create a tooltip for previewing things in the rule view (images for now)
   this.previewTooltip = new Tooltip(this.inspector.panelDoc);
@@ -1328,17 +1337,23 @@ CssRuleView.prototype = {
    * Return {bool} true if the rule view currently has an input editor visible.
    */
   get isEditing() {
     return this.element.querySelectorAll(".styleinspector-propertyeditor").length > 0
       || this.colorPicker.tooltip.isShown();
   },
 
   _handlePrefChange: function(event, data) {
-    if (data.pref == "devtools.defaultColorUnit") {
+    if (data.pref == PREF_UA_STYLES) {
+      this.showUserAgentStyles = Services.prefs.getBoolPref(PREF_UA_STYLES);
+    }
+
+    // Reselect the currently selected element
+    if (data.pref === PREF_UA_STYLES ||
+        data.pref === "devtools.defaultColorUnit") {
       let element = this._viewedElement;
       this._viewedElement = null;
       this.highlight(element);
     }
   },
 
   _onSourcePrefChanged: function() {
     if (this.menuitemSources) {
@@ -1423,17 +1438,17 @@ CssRuleView.prototype = {
     }
 
     this._viewedElement = aElement;
     if (!this._viewedElement) {
       this._showEmpty();
       return promise.resolve(undefined);
     }
 
-    this._elementStyle = new ElementStyle(aElement, this.store, this.pageStyle);
+    this._elementStyle = new ElementStyle(aElement, this.store, this.pageStyle, this.showUserAgentStyles);
     return this._elementStyle.init().then(() => {
       return this._populate();
     }).then(() => {
       // A new node may already be selected, in which this._elementStyle will
       // be null.
       if (this._elementStyle) {
         this._elementStyle.onChanged = () => {
           this._changed();
@@ -1645,27 +1660,29 @@ CssRuleView.prototype = {
  * @param {Rule} aRule
  *        The Rule object we're editing.
  * @constructor
  */
 function RuleEditor(aRuleView, aRule) {
   this.ruleView = aRuleView;
   this.doc = this.ruleView.doc;
   this.rule = aRule;
+  this.editable = !aRule.isSystem;
 
   this._onNewProperty = this._onNewProperty.bind(this);
   this._newPropertyDestroy = this._newPropertyDestroy.bind(this);
 
   this._create();
 }
 
 RuleEditor.prototype = {
   _create: function() {
     this.element = this.doc.createElementNS(HTML_NS, "div");
     this.element.className = "ruleview-rule theme-separator";
+    this.element.setAttribute("uneditable", !this.editable);
     this.element._ruleEditor = this;
     if (this.rule.pseudoElement) {
       this.element.classList.add("ruleview-rule-pseudo-element");
     }
 
     // Give a relative position for the inplace editor's measurement
     // span to be placed absolutely against.
     this.element.style.position = "relative";
@@ -1699,26 +1716,27 @@ RuleEditor.prototype = {
       class: "ruleview-selector theme-fg-color3"
     });
 
     this.openBrace = createChild(header, "span", {
       class: "ruleview-ruleopen",
       textContent: " {"
     });
 
-    code.addEventListener("click", function() {
-      let selection = this.doc.defaultView.getSelection();
-      if (selection.isCollapsed) {
-        this.newProperty();
-      }
-    }.bind(this), false);
-
-    this.element.addEventListener("mousedown", function() {
-      this.doc.defaultView.focus();
-    }.bind(this), false);
+    this.propertyList = createChild(code, "ul", {
+      class: "ruleview-propertylist"
+    });
+
+    this.populate();
+
+    this.closeBrace = createChild(code, "div", {
+      class: "ruleview-ruleclose",
+      tabindex: this.editable ? "0" : "-1",
+      textContent: "}"
+    });
 
     this.element.addEventListener("contextmenu", event => {
       try {
         // In the sidebar we do not have this.doc.popupNode so we need to save
         // the node ourselves.
         this.doc.popupNode = event.explicitOriginalTarget;
         let win = this.doc.defaultView;
         win.focus();
@@ -1726,46 +1744,51 @@ RuleEditor.prototype = {
         this.ruleView._contextmenu.openPopupAtScreen(
           event.screenX, event.screenY, true);
 
       } catch(e) {
         console.error(e);
       }
     }, false);
 
-    this.propertyList = createChild(code, "ul", {
-      class: "ruleview-propertylist"
-    });
-
-    this.populate();
-
-    this.closeBrace = createChild(code, "div", {
-      class: "ruleview-ruleclose",
-      tabindex: "0",
-      textContent: "}"
-    });
-
-    // Create a property editor when the close brace is clicked.
-    editableItem({ element: this.closeBrace }, (aElement) => {
-      this.newProperty();
-    });
+    if (this.editable) {
+      code.addEventListener("click", function() {
+        let selection = this.doc.defaultView.getSelection();
+        if (selection.isCollapsed) {
+          this.newProperty();
+        }
+      }.bind(this), false);
+
+      this.element.addEventListener("mousedown", function() {
+        this.doc.defaultView.focus();
+      }.bind(this), false);
+
+      // Create a property editor when the close brace is clicked.
+      editableItem({ element: this.closeBrace }, (aElement) => {
+        this.newProperty();
+      });
+    }
   },
 
   updateSourceLink: function RuleEditor_updateSourceLink()
   {
     let sourceLabel = this.element.querySelector(".source-link-label");
-    sourceLabel.setAttribute("value", this.rule.title);
-
     let sourceHref = (this.rule.sheet && this.rule.sheet.href) ?
       this.rule.sheet.href : this.rule.title;
-
     sourceLabel.setAttribute("tooltiptext", sourceHref);
 
+    if (this.rule.isSystem) {
+      // xxx: internationalize string
+      sourceLabel.setAttribute("value", "(user agent) " + this.rule.title);
+    } else {
+      sourceLabel.setAttribute("value", this.rule.title);
+    }
+
     let showOrig = Services.prefs.getBoolPref(PREF_ORIG_SOURCES);
-    if (showOrig && this.rule.domRule.type != ELEMENT_STYLE) {
+    if (showOrig && !this.rule.isSystem && this.rule.domRule.type != ELEMENT_STYLE) {
       this.rule.getOriginalSourceStrings().then((strings) => {
         sourceLabel.setAttribute("value", strings.short);
         sourceLabel.setAttribute("tooltiptext", strings.full);
       })
     }
   },
 
   /**
@@ -2009,92 +2032,51 @@ TextPropertyEditor.prototype = {
     this.element = this.doc.createElementNS(HTML_NS, "li");
     this.element.classList.add("ruleview-property");
 
     // The enable checkbox will disable or enable the rule.
     this.enable = createChild(this.element, "div", {
       class: "ruleview-enableproperty theme-checkbox",
       tabindex: "-1"
     });
-    this.enable.addEventListener("click", this._onEnableClicked, true);
 
     // Click to expand the computed properties of the text property.
     this.expander = createChild(this.element, "span", {
       class: "ruleview-expander theme-twisty"
     });
-    this.expander.addEventListener("click", this._onExpandClicked, true);
 
     this.nameContainer = createChild(this.element, "span", {
       class: "ruleview-namecontainer"
     });
-    this.nameContainer.addEventListener("click", (aEvent) => {
-      // Clicks within the name shouldn't propagate any further.
-      aEvent.stopPropagation();
-      if (aEvent.target === propertyContainer) {
-        this.nameSpan.click();
-      }
-    }, false);
 
     // Property name, editable when focused.  Property name
     // is committed when the editor is unfocused.
     this.nameSpan = createChild(this.nameContainer, "span", {
       class: "ruleview-propertyname theme-fg-color5",
-      tabindex: "0",
+      tabindex: this.ruleEditor.editable ? "0" : "-1",
     });
 
-    editableField({
-      start: this._onStartEditing,
-      element: this.nameSpan,
-      done: this._onNameDone,
-      destroy: this.update,
-      advanceChars: ':',
-      contentType: InplaceEditor.CONTENT_TYPES.CSS_PROPERTY,
-      popup: this.popup
-    });
-
-    // Auto blur name field on multiple CSS rules get pasted in.
-    this.nameContainer.addEventListener("paste",
-      blurOnMultipleProperties, false);
-
     appendText(this.nameContainer, ": ");
 
     // Create a span that will hold the property and semicolon.
     // Use this span to create a slightly larger click target
     // for the value.
     let propertyContainer = createChild(this.element, "span", {
       class: "ruleview-propertycontainer"
     });
 
-    propertyContainer.addEventListener("click", (aEvent) => {
-      // Clicks within the value shouldn't propagate any further.
-      aEvent.stopPropagation();
-
-      if (aEvent.target === propertyContainer) {
-        this.valueSpan.click();
-      }
-    }, false);
 
     // Property value, editable when focused.  Changes to the
     // property value are applied as they are typed, and reverted
     // if the user presses escape.
     this.valueSpan = createChild(propertyContainer, "span", {
       class: "ruleview-propertyvalue theme-fg-color1",
-      tabindex: "0",
+      tabindex: this.ruleEditor.editable ? "0" : "-1",
     });
 
-    this.valueSpan.addEventListener("click", (event) => {
-      let target = event.target;
-
-      if (target.nodeName === "a") {
-        event.stopPropagation();
-        event.preventDefault();
-        this.browserWindow.openUILinkIn(target.href, "tab");
-      }
-    }, false);
-
     // Storing the TextProperty on the valuespan for easy access
     // (for instance by the tooltip)
     this.valueSpan.textProperty = this.prop;
 
     // Save the initial value as the last committed value,
     // for restoring after pressing escape.
     this.committed = { name: this.prop.name,
                        value: this.prop.value,
@@ -2109,27 +2091,74 @@ TextPropertyEditor.prototype = {
     });
 
     // Holds the viewers for the computed properties.
     // will be populated in |_updateComputed|.
     this.computed = createChild(this.element, "ul", {
       class: "ruleview-computedlist",
     });
 
-    editableField({
-      start: this._onStartEditing,
-      element: this.valueSpan,
-      done: this._onValueDone,
-      destroy: this.update,
-      validate: this._onValidate,
-      advanceChars: ';',
-      contentType: InplaceEditor.CONTENT_TYPES.CSS_VALUE,
-      property: this.prop,
-      popup: this.popup
-    });
+    // Only bind event handlers if the rule is editable.
+    if (this.ruleEditor.editable) {
+      this.enable.addEventListener("click", this._onEnableClicked, true);
+      this.expander.addEventListener("click", this._onExpandClicked, true);
+
+      this.nameContainer.addEventListener("click", (aEvent) => {
+        // Clicks within the name shouldn't propagate any further.
+        aEvent.stopPropagation();
+        if (aEvent.target === propertyContainer) {
+          this.nameSpan.click();
+        }
+      }, false);
+
+      editableField({
+        start: this._onStartEditing,
+        element: this.nameSpan,
+        done: this._onNameDone,
+        destroy: this.update,
+        advanceChars: ':',
+        contentType: InplaceEditor.CONTENT_TYPES.CSS_PROPERTY,
+        popup: this.popup
+      });
+
+      // Auto blur name field on multiple CSS rules get pasted in.
+      this.nameContainer.addEventListener("paste",
+        blurOnMultipleProperties, false);
+
+      propertyContainer.addEventListener("click", (aEvent) => {
+        // Clicks within the value shouldn't propagate any further.
+        aEvent.stopPropagation();
+
+        if (aEvent.target === propertyContainer) {
+          this.valueSpan.click();
+        }
+      }, false);
+
+      this.valueSpan.addEventListener("click", (event) => {
+        let target = event.target;
+
+        if (target.nodeName === "a") {
+          event.stopPropagation();
+          event.preventDefault();
+          this.browserWindow.openUILinkIn(target.href, "tab");
+        }
+      }, false);
+
+      editableField({
+        start: this._onStartEditing,
+        element: this.valueSpan,
+        done: this._onValueDone,
+        destroy: this.update,
+        validate: this._onValidate,
+        advanceChars: ';',
+        contentType: InplaceEditor.CONTENT_TYPES.CSS_VALUE,
+        property: this.prop,
+        popup: this.popup
+      });
+    }
   },
 
   /**
    * Get the path from which to resolve requests for this
    * rule's stylesheet.
    * @return {string} the stylesheet's href.
    */
   get sheetHref() {
@@ -2232,25 +2261,27 @@ TextPropertyEditor.prototype = {
       urlClass: "theme-link",
       baseURI: this.sheetURI
     });
     this.valueSpan.innerHTML = "";
     this.valueSpan.appendChild(frag);
 
     // Attach the color picker tooltip to the color swatches
     this._swatchSpans = this.valueSpan.querySelectorAll("." + swatchClass);
-    for (let span of this._swatchSpans) {
-      // Capture the original declaration value to be able to revert later
-      let originalValue = this.valueSpan.textContent;
-      // Adding this swatch to the list of swatches our colorpicker knows about
-      this.ruleEditor.ruleView.colorPicker.addSwatch(span, {
-        onPreview: () => this._previewValue(this.valueSpan.textContent),
-        onCommit: () => this._applyNewValue(this.valueSpan.textContent),
-        onRevert: () => this._applyNewValue(originalValue)
-      });
+    if (this.ruleEditor.editable) {
+      for (let span of this._swatchSpans) {
+        // Capture the original declaration value to be able to revert later
+        let originalValue = this.valueSpan.textContent;
+        // Adding this swatch to the list of swatches our colorpicker knows about
+        this.ruleEditor.ruleView.colorPicker.addSwatch(span, {
+          onPreview: () => this._previewValue(this.valueSpan.textContent),
+          onCommit: () => this._applyNewValue(this.valueSpan.textContent),
+          onRevert: () => this._applyNewValue(originalValue)
+        });
+      }
     }
 
     // Populate the computed styles.
     this._updateComputed();
   },
 
   _onStartEditing: function() {
     this.element.classList.remove("ruleview-overridden");
diff --git a/browser/devtools/styleinspector/test/browser.ini b/browser/devtools/styleinspector/test/browser.ini
--- a/browser/devtools/styleinspector/test/browser.ini
+++ b/browser/devtools/styleinspector/test/browser.ini
@@ -78,16 +78,17 @@ skip-if = os == "win" && debug # bug 963
 [browser_ruleview_override.js]
 [browser_ruleview_pseudo-element.js]
 [browser_ruleview_refresh-on-attribute-change_01.js]
 [browser_ruleview_refresh-on-attribute-change_02.js]
 [browser_ruleview_refresh-on-style-change.js]
 [browser_ruleview_select-and-copy-styles.js]
 [browser_ruleview_style-editor-link.js]
 [browser_ruleview_urls-clickable.js]
+[browser_ruleview_user-agent-styles.js]
 [browser_ruleview_user-property-reset.js]
 [browser_styleinspector_csslogic-content-stylesheets.js]
 [browser_styleinspector_csslogic-inherited-properties.js]
 [browser_styleinspector_csslogic-specificity.js]
 [browser_styleinspector_inplace-editor.js]
 [browser_styleinspector_output-parser.js]
 [browser_styleinspector_tooltip-background-image.js]
 [browser_styleinspector_tooltip-closes-on-new-selection.js]
diff --git a/browser/devtools/styleinspector/test/browser_ruleview_user-agent-styles.js b/browser/devtools/styleinspector/test/browser_ruleview_user-agent-styles.js
new file mode 100644
--- /dev/null
+++ b/browser/devtools/styleinspector/test/browser_ruleview_user-agent-styles.js
@@ -0,0 +1,48 @@
+/* vim: set ft=javascript ts=2 et sw=2 tw=80: */
+/* Any copyright is dedicated to the Public Domain.
+ http://creativecommons.org/publicdomain/zero/1.0/ */
+
+"use strict";
+
+// Check that user agent styles are inspectable via rule view
+
+
+  // XXX check matching styles from server with styles from frontend.
+  // Probably can't hardcode the checks here since UA styles could be different across platforms
+  // Could also run through after toggling pref to make sure there are less (and no isSystem styles)
+
+let test = asyncTest(function*() {
+
+  // let {ELEMENT_STYLE} = devtools.require("devtools/server/actors/styles");
+  // let PREF = "devtools.inspector.showUserAgentStyles";
+
+  // info("Turning the pref " + PREF + " on");
+  // Services.prefs.setBoolPref(PREF, true);
+
+  // yield addTab("data:text/html;charset=utf-8," +
+  //   "<blockquote type=cite><pre _moz_quote=true>fun <em style='color:orange'>to</em> inspect</code></blockquote>"
+  // );
+  // let {toolbox, inspector, view} = yield openRuleView();
+
+  // yield simpleInherit(inspector, view);
+
+  // info("Resetting " + PREF);
+  // Services.prefs.clearUserPref(PREF);
+
+});
+
+function* simpleInherit(inspector, view) {
+  yield selectNode("blockquote", inspector);
+
+  let entries = yield inspector.pageStyle.getApplied(inspector.selection.nodeFront, {
+    inherited: true,
+    matchedSelectors: true,
+    filter: "ua"
+  });
+
+  let elementStyle = view._elementStyle;
+  is(elementStyle.rules.length, entries.length, "Should have correct number of rules.");
+  entries.forEach((entry, i) => {
+    let elementStyleRule = elementStyle.rules[i];
+  });
+}
diff --git a/browser/locales/en-US/chrome/browser/devtools/toolbox.dtd b/browser/locales/en-US/chrome/browser/devtools/toolbox.dtd
--- a/browser/locales/en-US/chrome/browser/devtools/toolbox.dtd
+++ b/browser/locales/en-US/chrome/browser/devtools/toolbox.dtd
@@ -25,16 +25,22 @@
   -  the heading of the Inspector group in the options panel. -->
 <!ENTITY options.context.inspector "Inspector">
 
 <!-- LOCALIZATION NOTE (options.defaultColorUnit.label): This is the label for a
   -  dropdown list that controls the default color unit used in the inspector.
   -  This label is visible in the options panel. -->
 <!ENTITY options.defaultColorUnit.label "Default color unit">
 
+<!-- LOCALIZATION NOTE (options.showUserAgentStyles.label): This is the label
+  -  for the checkbox option to show user agent styles in the Inspector
+  -  panel. -->
+<!ENTITY options.showUserAgentStyles.label "Show Browser Styles">
+<!ENTITY options.showUserAgentStyles.tooltip "Turning this on will show default styles that are loaded by the browser.">
+
 <!-- LOCALIZATION NOTE (options.defaultColorUnit.accesskey): This is the access
   -  key for a dropdown list that controls the default color unit used in the
   -  inspector. This is visible in the options panel. -->
 <!ENTITY options.defaultColorUnit.accesskey "U">
 
 <!-- LOCALIZATION NOTE (options.defaultColorUnit.hex): This is used in the
   -  'Default color unit' dropdown list and is visible in the options panel. -->
 <!ENTITY options.defaultColorUnit.hex "Hex">
diff --git a/browser/themes/shared/devtools/ruleview.css b/browser/themes/shared/devtools/ruleview.css
--- a/browser/themes/shared/devtools/ruleview.css
+++ b/browser/themes/shared/devtools/ruleview.css
@@ -36,16 +36,38 @@
   text-decoration: underline;
 }
 
 .ruleview-rule,
 #noResults {
   padding: 2px 4px;
 }
 
+/* User agent styles are not editable, display them differently */
+.theme-light .ruleview-rule[uneditable=true] {
+  background: #ebeced; /* Tab Toolbar */
+}
+.theme-dark .ruleview-rule[uneditable=true] {
+  background: #252c33; /* Tab Toolbar */
+}
+.ruleview-rule[uneditable=true] :focus {
+  outline: none;
+}
+.ruleview-rule[uneditable=true] .theme-link {
+  color: #5f88b0; /* Blue - Grey */
+  text-decoration: none;
+}
+.ruleview-rule[uneditable=true] .ruleview-enableproperty {
+  visibility: hidden;
+}
+.ruleview-rule[uneditable=true] .ruleview-namecontainer > .ruleview-propertyname,
+.ruleview-rule[uneditable=true] .ruleview-propertycontainer > .ruleview-propertyvalue {
+  border-bottom-color: transparent;
+}
+
 #noResults {
   font: message-box;
   color: GrayText;
 }
 
 .ruleview-rule + .ruleview-rule {
   border-top-width: 1px;
   border-top-style: dotted;
diff --git a/toolkit/devtools/server/actors/styles.js b/toolkit/devtools/server/actors/styles.js
--- a/toolkit/devtools/server/actors/styles.js
+++ b/toolkit/devtools/server/actors/styles.js
@@ -401,17 +401,18 @@ var PageStyleActor = protocol.ActorClass
             continue;
           }
         }
 
         let ruleActor = this._styleRef(domRule);
         rules.push({
           rule: ruleActor,
           inherited: inherited,
-          pseudoElement: pseudo
+          pseudoElement: pseudo,
+          isSystem: isSystem
         });
       }
 
     }
   },
 
   /**
    * Expand Sets of rules and sheets to include all parent rules and sheets.
