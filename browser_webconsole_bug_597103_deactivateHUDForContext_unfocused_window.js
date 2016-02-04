# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  a81960ce939edd6bff30d85fa07350e780901dac
Bug 1243970 - e10s fixes for browser_webconsole_bug_597103_deactivateHUDForContext_unfocused_window.js;r=linclark

diff --git a/devtools/client/webconsole/test/browser.ini b/devtools/client/webconsole/test/browser.ini
--- a/devtools/client/webconsole/test/browser.ini
+++ b/devtools/client/webconsole/test/browser.ini
@@ -220,25 +220,23 @@ skip-if = e10s # Bug 1042253 - webconsol
 [browser_webconsole_bug_587617_output_copy.js]
 [browser_webconsole_bug_588342_document_focus.js]
 skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_webconsole_bug_588730_text_node_insertion.js]
 [browser_webconsole_bug_588967_input_expansion.js]
 [browser_webconsole_bug_589162_css_filter.js]
 [browser_webconsole_bug_592442_closing_brackets.js]
 [browser_webconsole_bug_593003_iframe_wrong_hud.js]
-skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
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
diff --git a/devtools/client/webconsole/test/browser_webconsole_bug_593003_iframe_wrong_hud.js b/devtools/client/webconsole/test/browser_webconsole_bug_593003_iframe_wrong_hud.js
--- a/devtools/client/webconsole/test/browser_webconsole_bug_593003_iframe_wrong_hud.js
+++ b/devtools/client/webconsole/test/browser_webconsole_bug_593003_iframe_wrong_hud.js
@@ -10,62 +10,57 @@ const TEST_URI = "http://example.com/bro
 
 const TEST_IFRAME_URI = "http://example.com/browser/devtools/client/" +
                         "webconsole/test/test-bug-593003-iframe-wrong-" +
                         "hud-iframe.html";
 
 const TEST_DUMMY_URI = "http://example.com/browser/devtools/client/" +
                        "webconsole/test/test-console.html";
 
-var tab1, tab2;
+add_task(function*() {
 
-function test() {
-  loadTab(TEST_URI).then(({tab}) => {
-    tab1 = tab;
+  let tab1 = (yield loadTab(TEST_URI)).tab;
+  content.console.log("FOO");
+  yield openConsole();
 
-    content.console.log("FOO");
-    openConsole().then(() => {
-      tab2 = gBrowser.addTab(TEST_DUMMY_URI);
-      gBrowser.selectedTab = tab2;
-      gBrowser.selectedBrowser.addEventListener("load", tab2Loaded, true);
-    });
-  });
+  let tab2 = (yield loadTab(TEST_DUMMY_URI)).tab;
+  yield openConsole(gBrowser.selectedTab);
+
+  info("Reloading tab 1");
+  yield reloadTab(tab1);
+
+  info("Checking for messages");
+  yield checkMessages(tab1, tab2);
+
+  info("Cleaning up");
+  yield closeConsole(tab1);
+  yield closeConsole(tab2);
+});
+
+function* reloadTab(tab) {
+  let loaded = BrowserTestUtils.browserLoaded(tab.linkedBrowser);
+  tab.linkedBrowser.reload();
+  yield loaded;
 }
 
-function tab2Loaded(aEvent) {
-  tab2.linkedBrowser.removeEventListener(aEvent.type, tab2Loaded, true);
-
-  openConsole(gBrowser.selectedTab).then(() => {
-    tab1.linkedBrowser.addEventListener("load", tab1Reloaded, true);
-    tab1.linkedBrowser.contentWindow.location.reload();
-  });
-}
-
-function tab1Reloaded(aEvent) {
-  tab1.linkedBrowser.removeEventListener(aEvent.type, tab1Reloaded, true);
-
-  let hud1 = HUDService.getHudByWindow(tab1.linkedBrowser.contentWindow);
+function* checkMessages(tab1, tab2) {
+  let hud1 = yield openConsole(tab1);
   let outputNode1 = hud1.outputNode;
 
-  waitForMessages({
+  info("Waiting for messages");
+  yield waitForMessages({
     webconsole: hud1,
     messages: [{
       text: TEST_IFRAME_URI,
       category: CATEGORY_NETWORK,
       severity: SEVERITY_LOG,
-    }],
-  }).then(() => {
-    let hud2 = HUDService.getHudByWindow(tab2.linkedBrowser.contentWindow);
-    let outputNode2 = hud2.outputNode;
+    }]
+  });
 
-    isnot(outputNode1, outputNode2,
-      "the two HUD outputNodes must be different");
+  let hud2 = yield openConsole(tab2);
+  let outputNode2 = hud2.outputNode;
 
-    let msg = "Didn't find the iframe network request in tab2";
-    testLogEntry(outputNode2, TEST_IFRAME_URI, msg, true, true);
+  isnot(outputNode1, outputNode2,
+    "the two HUD outputNodes must be different");
 
-    closeConsole(tab2).then(() => {
-      gBrowser.removeTab(tab2);
-      tab1 = tab2 = null;
-      executeSoon(finishTest);
-    });
-  });
+  let msg = "Didn't find the iframe network request in tab2";
+  testLogEntry(outputNode2, TEST_IFRAME_URI, msg, true, true);
 }
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
