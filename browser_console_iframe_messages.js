# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  082700c31ecafd2b4b4b24c6c37bc7ff398e98dc
Bug 1243983 - browser_console_iframe_messages.js

diff --git a/devtools/client/webconsole/test/browser_console_iframe_messages.js b/devtools/client/webconsole/test/browser_console_iframe_messages.js
--- a/devtools/client/webconsole/test/browser_console_iframe_messages.js
+++ b/devtools/client/webconsole/test/browser_console_iframe_messages.js
@@ -50,54 +50,62 @@ const expectedMessagesAny = [
     name: "iframe 1 (repeats: 2)",
     text: "iframe 1",
     category: CATEGORY_WEBDEV,
     severity: SEVERITY_LOG,
     repeats: 2
   },
 ];
 
-function test() {
-  expectUncaughtException();
-  loadTab(TEST_URI).then(() => {
-    openConsole().then(consoleOpened);
+add_task(function*() {
+  // On e10s, the exception is triggered in child process
+  // and is ignored by test harness
+  if (!Services.appinfo.browserTabsRemoteAutostart) {
+    expectUncaughtException();
+  }
+
+  yield loadTab(TEST_URI);
+  let hud = yield openConsole();
+
+  yield testWebConsole(hud);
+  yield closeConsole();
+  info("web console closed");
+
+  // The browser console doesn't show page's console.log statements in
+  // e10s windows. See Bug 1241289.
+  if (!Services.appinfo.browserTabsRemoteAutostart) {
+    hud = yield HUDService.toggleBrowserConsole();
+    yield testBrowserConsole(hud);
+    yield closeConsole();
+  }
+});
+
+function* testWebConsole(hud) {
+  ok(hud, "web console opened");
+
+  yield waitForMessages({
+    webconsole: hud,
+    messages: expectedMessages,
+  });
+
+  info("first messages matched");
+
+  yield waitForMessages({
+      webconsole: hud,
+      messages: expectedMessagesAny,
+      matchCondition: "any",
   });
 }
 
-function consoleOpened(hud) {
-  ok(hud, "web console opened");
-
-  waitForMessages({
+function* testBrowserConsole(hud) {
+  ok(hud, "browser console opened");
+  yield waitForMessages({
     webconsole: hud,
     messages: expectedMessages,
-  }).then(() => {
-    info("first messages matched");
-    waitForMessages({
-      webconsole: hud,
-      messages: expectedMessagesAny,
-      matchCondition: "any",
-    }).then(() => {
-      closeConsole().then(onWebConsoleClose);
-    });
+  });
+
+  info("first messages matched");
+  yield waitForMessages({
+    webconsole: hud,
+    messages: expectedMessagesAny,
+    matchCondition: "any",
   });
 }
-
-function onWebConsoleClose() {
-  info("web console closed");
-  HUDService.toggleBrowserConsole().then(onBrowserConsoleOpen);
-}
-
-function onBrowserConsoleOpen(hud) {
-  ok(hud, "browser console opened");
-  waitForMessages({
-    webconsole: hud,
-    messages: expectedMessages,
-  }).then(() => {
-    info("first messages matched");
-    waitForMessages({
-      webconsole: hud,
-      messages: expectedMessagesAny,
-      matchCondition: "any",
-    }).then(() => {
-      closeConsole().then(finishTest);
-    });
-  });
-}
