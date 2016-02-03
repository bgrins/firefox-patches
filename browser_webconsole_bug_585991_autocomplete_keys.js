# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  75a2ba208d0e2889c6ce99827ae232e8b4a79457
Bug 1243992 - browser_webconsole_bug_585991_autocomplete_keys.js

diff --git a/devtools/client/webconsole/test/browser_webconsole_bug_585991_autocomplete_keys.js b/devtools/client/webconsole/test/browser_webconsole_bug_585991_autocomplete_keys.js
--- a/devtools/client/webconsole/test/browser_webconsole_bug_585991_autocomplete_keys.js
+++ b/devtools/client/webconsole/test/browser_webconsole_bug_585991_autocomplete_keys.js
@@ -250,21 +250,23 @@ function testReturnKey() {
     jsterm.setInputValue("window.foobarBug58599");
     EventUtils.synthesizeKey("1", {});
     EventUtils.synthesizeKey(".", {});
   });
 
   return deferred.promise;
 }
 
-function dontShowArrayNumbers() {
+function* dontShowArrayNumbers() {
   let deferred = promise.defer();
 
   info("dontShowArrayNumbers");
-  content.wrappedJSObject.foobarBug585991 = ["Sherlock Holmes"];
+  yield ContentTask.spawn(gBrowser.selectedBrowser, {}, function*() {
+    content.wrappedJSObject.foobarBug585991 = ["Sherlock Holmes"];
+  });
 
   jsterm = HUD.jsterm;
   popup = jsterm.autocompletePopup;
 
   popup._panel.addEventListener("popupshown", function onShown() {
     popup._panel.removeEventListener("popupshown", onShown, false);
 
     let sameItems = popup.getItems().map(function(e) {
