# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  abfac0d05b79b49597d5fa8d4ce87caf4caaff8e
Bug 1243988 - Enable browser_webconsole_autocomplete_popup_close_on_tab_switch.js in e10s;r=linclark

diff --git a/devtools/client/webconsole/test/browser.ini b/devtools/client/webconsole/test/browser.ini
--- a/devtools/client/webconsole/test/browser.ini
+++ b/devtools/client/webconsole/test/browser.ini
@@ -333,17 +333,16 @@ skip-if = e10s # Bug 1042253 - webconsol
 [browser_webconsole_trackingprotection_errors.js]
 tags = trackingprotection
 [browser_webconsole_view_source.js]
 [browser_webconsole_reflow.js]
 [browser_webconsole_log_file_filter.js]
 [browser_webconsole_expandable_timestamps.js]
 [browser_webconsole_autocomplete_in_debugger_stackframe.js]
 [browser_webconsole_autocomplete_popup_close_on_tab_switch.js]
-skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_webconsole_autocomplete-properties-with-non-alphanumeric-names.js]
 [browser_console_hide_jsterm_when_devtools_chrome_enabled_false.js]
 [browser_console_history_persist.js]
 [browser_webconsole_output_01.js]
 [browser_webconsole_output_02.js]
 [browser_webconsole_output_03.js]
 [browser_webconsole_output_04.js]
 [browser_webconsole_output_05.js]
diff --git a/devtools/client/webconsole/test/browser_webconsole_autocomplete_popup_close_on_tab_switch.js b/devtools/client/webconsole/test/browser_webconsole_autocomplete_popup_close_on_tab_switch.js
--- a/devtools/client/webconsole/test/browser_webconsole_autocomplete_popup_close_on_tab_switch.js
+++ b/devtools/client/webconsole/test/browser_webconsole_autocomplete_popup_close_on_tab_switch.js
@@ -9,29 +9,19 @@
 
 const TEST_URI = "data:text/html;charset=utf-8,<p>bug 900448 - autocomplete " +
                  "popup closes on tab switch";
 
 add_task(function*() {
   yield loadTab(TEST_URI);
   let hud = yield openConsole();
   let popup = hud.jsterm.autocompletePopup;
-  let popupShown = onPopupShown(popup._panel);
+  let popupShown = once(popup._panel, "popupshown");
 
   hud.jsterm.setInputValue("sc");
   EventUtils.synthesizeKey("r", {});
 
   yield popupShown;
 
+  yield loadTab("data:text/html;charset=utf-8,<p>testing autocomplete closes")
+
   ok(!popup.isOpen, "Popup closes on tab switch");
 });
-
-function onPopupShown(panel) {
-  let finished = promise.defer();
-
-  panel.addEventListener("popupshown", function popupOpened() {
-    panel.removeEventListener("popupshown", popupOpened, false);
-    loadTab("data:text/html;charset=utf-8,<p>testing autocomplete closes")
-      .then(finished.resolve);
-  }, false);
-
-  return finished.promise;
-}
