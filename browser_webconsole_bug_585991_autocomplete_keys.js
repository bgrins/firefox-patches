# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  bdd606bd44d5481eb7f33030512cd2adc8ea4c4d
Bug 1243992 - e10s fixes for browser_webconsole_bug_585991_autocomplete_keys.js;r=linclark

diff --git a/devtools/client/webconsole/test/browser.ini b/devtools/client/webconsole/test/browser.ini
--- a/devtools/client/webconsole/test/browser.ini
+++ b/devtools/client/webconsole/test/browser.ini
@@ -209,17 +209,16 @@ tags = mcb
 [browser_webconsole_bug_580001_closing_after_completion.js]
 [browser_webconsole_bug_580030_errors_after_page_reload.js]
 [browser_webconsole_bug_580454_timestamp_l10n.js]
 [browser_webconsole_bug_582201_duplicate_errors.js]
 [browser_webconsole_bug_583816_No_input_and_Tab_key_pressed.js]
 [browser_webconsole_bug_585237_line_limit.js]
 [browser_webconsole_bug_585956_console_trace.js]
 [browser_webconsole_bug_585991_autocomplete_keys.js]
-skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_webconsole_bug_585991_autocomplete_popup.js]
 [browser_webconsole_bug_586388_select_all.js]
 [browser_webconsole_bug_587617_output_copy.js]
 [browser_webconsole_bug_588342_document_focus.js]
 [browser_webconsole_bug_588730_text_node_insertion.js]
 [browser_webconsole_bug_588967_input_expansion.js]
 [browser_webconsole_bug_589162_css_filter.js]
 [browser_webconsole_bug_592442_closing_brackets.js]
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
