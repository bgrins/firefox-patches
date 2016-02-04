# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  a81960ce939edd6bff30d85fa07350e780901dac
Bug 1243970 - e10s fixes for browser_webconsole_bug_597103_deactivateHUDForContext_unfocused_window.js;r=linclark

diff --git a/devtools/client/webconsole/test/browser.ini b/devtools/client/webconsole/test/browser.ini
--- a/devtools/client/webconsole/test/browser.ini
+++ b/devtools/client/webconsole/test/browser.ini
@@ -228,17 +228,16 @@ skip-if = e10s # Bug 1042253 - webconsol
 skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_webconsole_bug_594497_history_arrow_keys.js]
 [browser_webconsole_bug_595223_file_uri.js]
 [browser_webconsole_bug_595350_multiple_windows_and_tabs.js]
 skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_webconsole_bug_595934_message_categories.js]
 skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_webconsole_bug_597103_deactivateHUDForContext_unfocused_window.js]
-skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_webconsole_bug_597136_external_script_errors.js]
 [browser_webconsole_bug_597136_network_requests_from_chrome.js]
 [browser_webconsole_bug_597460_filter_scroll.js]
 [browser_webconsole_bug_597756_reopen_closed_tab.js]
 [browser_webconsole_bug_599725_response_headers.js]
 [browser_webconsole_bug_600183_charset.js]
 [browser_webconsole_bug_601177_log_levels.js]
 [browser_webconsole_bug_601352_scroll.js]
diff --git a/devtools/client/webconsole/test/browser_webconsole_bug_597103_deactivateHUDForContext_unfocused_window.js b/devtools/client/webconsole/test/browser_webconsole_bug_597103_deactivateHUDForContext_unfocused_window.js
--- a/devtools/client/webconsole/test/browser_webconsole_bug_597103_deactivateHUDForContext_unfocused_window.js
+++ b/devtools/client/webconsole/test/browser_webconsole_bug_597103_deactivateHUDForContext_unfocused_window.js
@@ -80,17 +80,17 @@ function tab2Loaded(aEvent) {
 
     executeSoon(function() {
       win2.close();
       tab1 = tab2 = win1 = win2 = null;
       finishTest();
     });
   }
 
-  waitForFocus(openConsoles, tab2.linkedBrowser.contentWindow);
+  openConsoles();
 }
 
 function test() {
   loadTab(TEST_URI).then(() => {
     tab1 = gBrowser.selectedTab;
     win1 = window;
     tab1Loaded();
   });
