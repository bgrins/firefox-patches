# HG changeset patch
# Parent 43daa13856bea6ef5e7db97719a901f0cc64c3a0
# User Brian Grinstead <bgrinstead@mozilla.com>
Bug 935803 - Display browser styles in the rule view (UA style sheet rules)

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
+const INSPECT_UA_STYLES = false;
 
 /**
  * These regular expressions are adapted from firebug's css.js, and are
  * used to parse CSSStyleDeclaration's cssText attribute.
  */
 
 // Used to split on css line separators
 const CSS_LINE_RE = /(?:[^;\(]*(?:\([^\)]*?\))?[^;\(]*)*;?/g;
@@ -180,17 +181,18 @@ ElementStyle.prototype = {
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
+      filter: INSPECT_UA_STYLES ? "ua" : undefined,
     }).then(entries => {
       // Make sure the dummy element has been created before continuing...
       return this.dummyElementPromise.then(() => {
         if (this.populated != populated) {
           // Don't care anymore.
           return;
         }
 
@@ -396,25 +398,27 @@ ElementStyle.prototype = {
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
@@ -1645,27 +1649,29 @@ CssRuleView.prototype = {
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
@@ -1699,26 +1705,27 @@ RuleEditor.prototype = {
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
+    this.propertyList = createChild(code, "ul", {
+      class: "ruleview-propertylist"
+    });
 
-    this.element.addEventListener("mousedown", function() {
-      this.doc.defaultView.focus();
-    }.bind(this), false);
+    this.populate();
+
+    this.closeBrace = createChild(code, "div", {
+      class: "ruleview-ruleclose",
+      tabindex: "0", //this.editable ? "0" : "-1",
+      textContent: "}"
+    });
 
     this.element.addEventListener("contextmenu", event => {
       try {
         // In the sidebar we do not have this.doc.popupNode so we need to save
         // the node ourselves.
         this.doc.popupNode = event.explicitOriginalTarget;
         let win = this.doc.defaultView;
         win.focus();
@@ -1726,46 +1733,51 @@ RuleEditor.prototype = {
         this.ruleView._contextmenu.openPopupAtScreen(
           event.screenX, event.screenY, true);
 
       } catch(e) {
         console.error(e);
       }
     }, false);
 
-    this.propertyList = createChild(code, "ul", {
-      class: "ruleview-propertylist"
-    });
+    if (this.editable) {
+      code.addEventListener("click", function() {
+        let selection = this.doc.defaultView.getSelection();
+        if (selection.isCollapsed) {
+          this.newProperty();
+        }
+      }.bind(this), false);
 
-    this.populate();
+      this.element.addEventListener("mousedown", function() {
+        this.doc.defaultView.focus();
+      }.bind(this), false);
 
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
@@ -2009,92 +2021,51 @@ TextPropertyEditor.prototype = {
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
+      tabindex: "0" //this.ruleEditor.editable ? "0" : "-1",
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
+      tabindex: "0" //this.ruleEditor.editable ? "0" : "-1",
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
@@ -2109,27 +2080,74 @@ TextPropertyEditor.prototype = {
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
diff --git a/browser/themes/shared/devtools/ruleview.css b/browser/themes/shared/devtools/ruleview.css
--- a/browser/themes/shared/devtools/ruleview.css
+++ b/browser/themes/shared/devtools/ruleview.css
@@ -36,16 +36,23 @@
   text-decoration: underline;
 }
 
 .ruleview-rule,
 #noResults {
   padding: 2px 4px;
 }
 
+.ruleview-rule[uneditable=true] {
+  background: rgba(128, 128, 128, .1);
+}
+.ruleview-rule[uneditable=true] .ruleview-enableproperty {
+  visibility: hidden;
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
