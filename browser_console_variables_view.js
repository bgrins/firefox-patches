# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  be7baf885e40a038ee6e6db0295b6e995045511f
Bug 1243977 - browser_console_variables_view.js

diff --git a/devtools/client/webconsole/test/browser_console_variables_view.js b/devtools/client/webconsole/test/browser_console_variables_view.js
--- a/devtools/client/webconsole/test/browser_console_variables_view.js
+++ b/devtools/client/webconsole/test/browser_console_variables_view.js
@@ -103,21 +103,23 @@ function onUpdatedTestPropFound(aResults
   return updateVariablesViewProperty({
     property: prop,
     field: "name",
     string: "testUpdatedProp",
     webconsole: gWebConsole
   });
 }
 
-function onFooObjFetchAfterPropRename(aVar) {
+function* onFooObjFetchAfterPropRename(aVar) {
   info("onFooObjFetchAfterPropRename");
 
-  let para = content.wrappedJSObject.document.querySelector("p");
-  let expectedValue = content.document.title + content.location + para;
+  let expectedValue = yield ContentTask.spawn(gBrowser.selectedBrowser, {}, function* () {
+    let para = content.wrappedJSObject.document.querySelector("p");
+    return content.document.title + content.location + para;
+  });
 
   // Check that the new value is in the variables view.
   return findVariableViewProperties(aVar, [
     { name: "testUpdatedProp", value: expectedValue },
   ], { webconsole: gWebConsole });
 }
 
 function onRenamedTestPropFound(aResults) {
@@ -134,21 +136,23 @@ function onRenamedTestPropFound(aResults
   return updateVariablesViewProperty({
     property: prop,
     field: "value",
     string: "foobarzFailure()",
     webconsole: gWebConsole
   });
 }
 
-function onPropUpdateError(aVar) {
+function* onPropUpdateError(aVar) {
   info("onPropUpdateError");
 
-  let para = content.wrappedJSObject.document.querySelector("p");
-  let expectedValue = content.document.title + content.location + para;
+  let expectedValue = yield ContentTask.spawn(gBrowser.selectedBrowser, {}, function* () {
+    let para = content.wrappedJSObject.document.querySelector("p");
+    return content.document.title + content.location + para;
+  });
 
   // Make sure the property did not change.
   return findVariableViewProperties(aVar, [
     { name: "testUpdatedProp", value: expectedValue },
   ], { webconsole: gWebConsole });
 }
 
 function onRenamedTestPropFoundAgain(aResults) {
