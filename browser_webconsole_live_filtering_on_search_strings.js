# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  a1bf99aba7549c1fd3c586f6beb772c747bd9351
Bug 1247962 - Get rid of CPOW in browser_webconsole_live_filtering_on_search_strings.js;r=linclark

diff --git a/devtools/client/webconsole/test/browser_webconsole_live_filtering_on_search_strings.js b/devtools/client/webconsole/test/browser_webconsole_live_filtering_on_search_strings.js
--- a/devtools/client/webconsole/test/browser_webconsole_live_filtering_on_search_strings.js
+++ b/devtools/client/webconsole/test/browser_webconsole_live_filtering_on_search_strings.js
@@ -10,21 +10,21 @@
 const TEST_URI = "http://example.com/browser/devtools/client/webconsole/" +
                  "test/test-console.html";
 
 add_task(function*() {
   yield loadTab(TEST_URI);
   let hud = yield openConsole();
   hud.jsterm.clearOutput();
 
-  let console = content.console;
-
-  for (let i = 0; i < 50; i++) {
-    console.log("http://www.example.com/ " + i);
-  }
+  ContentTask.spawn(gBrowser.selectedBrowser, {}, function*() {
+    for (let i = 0; i < 50; i++) {
+      content.console.log("http://www.example.com/ " + i);
+    }
+  });
 
   yield waitForMessages({
     webconsole: hud,
     messages: [{
       text: "http://www.example.com/ 49",
       category: CATEGORY_WEBDEV,
       severity: SEVERITY_LOG,
     }],
