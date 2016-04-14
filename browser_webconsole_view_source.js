# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  ea707a9243907d2e89337d01ecf7c66f5f543a86
Bug 1239566 - Get rid of CPOW in browser_webconsole_view_source.js;r=linclark

diff --git a/devtools/client/webconsole/test/browser_webconsole_view_source.js b/devtools/client/webconsole/test/browser_webconsole_view_source.js
--- a/devtools/client/webconsole/test/browser_webconsole_view_source.js
+++ b/devtools/client/webconsole/test/browser_webconsole_view_source.js
@@ -11,25 +11,28 @@
 const TEST_URI = "http://example.com/browser/devtools/client/webconsole/" +
                  "test/test-error.html";
 
 add_task(function*() {
   yield loadTab(TEST_URI);
   let hud = yield openConsole(null);
   info("console opened");
 
-  let button = content.document.querySelector("button");
-  ok(button, "we have the button on the page");
 
   // On e10s, the exception is triggered in child process
   // and is ignored by test harness
   if (!Services.appinfo.browserTabsRemoteAutostart) {
     expectUncaughtException();
   }
-  EventUtils.sendMouseEvent({ type: "click" }, button, content);
+
+  ContentTask.spawn(gBrowser.selectedBrowser, {}, function*() {
+    let button = content.document.querySelector("button");
+    ok(button, "we have the button on the page");
+    button.click();
+  });
 
   let [result] = yield waitForMessages({
     webconsole: hud,
     messages: [{
       text: "fooBazBaz is not defined",
       category: CATEGORY_JS,
       severity: SEVERITY_ERROR,
     }],
