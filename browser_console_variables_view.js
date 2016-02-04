# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  c21fa64c878747e414dcbea28e7ec3efe0d59d83
Bug 1243977 - browser_console_variables_view.js

diff --git a/devtools/client/webconsole/test/browser.ini b/devtools/client/webconsole/test/browser.ini
--- a/devtools/client/webconsole/test/browser.ini
+++ b/devtools/client/webconsole/test/browser.ini
@@ -174,17 +174,16 @@ skip-if = e10s # Bug 1042253 - webconsol
 [browser_console_netlogging.js]
 [browser_console_nsiconsolemessage.js]
 [browser_console_optimized_out_vars.js]
 skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_console_private_browsing.js]
 skip-if = e10s # Bug 1042253 - webconsole e10s tests
 [browser_console_server_logging.js]
 [browser_console_variables_view.js]
-skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_console_variables_view_filter.js]
 [browser_console_variables_view_dom_nodes.js]
 [browser_console_variables_view_dont_sort_non_sortable_classes_properties.js]
 [browser_console_variables_view_special_names.js]
 [browser_console_variables_view_while_debugging.js]
 [browser_console_variables_view_while_debugging_and_inspecting.js]
 [browser_eval_in_debugger_stackframe.js]
 skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
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
