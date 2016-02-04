# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  29098be65d483e3e49430f9236c3a3fe28ea6464
Bug 1243983 - e10s fixes for browser_console_iframe_messages.js;r=linclark

diff --git a/devtools/client/webconsole/test/browser.ini b/devtools/client/webconsole/test/browser.ini
--- a/devtools/client/webconsole/test/browser.ini
+++ b/devtools/client/webconsole/test/browser.ini
@@ -161,17 +161,16 @@ skip-if = e10s # Bug 1042253 - webconsol
 [browser_console_copy_command.js]
 [browser_console_dead_objects.js]
 skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_console_copy_entire_message_context_menu.js]
 [browser_console_error_source_click.js]
 skip-if = e10s # Bug 1042253 - webconsole e10s tests
 [browser_console_filters.js]
 [browser_console_iframe_messages.js]
-skip-if = e10s # Bug 1042253 - webconsole e10s tests
 [browser_console_keyboard_accessibility.js]
 [browser_console_log_inspectable_object.js]
 [browser_console_native_getters.js]
 [browser_console_navigation_marker.js]
 [browser_console_netlogging.js]
 [browser_console_nsiconsolemessage.js]
 [browser_console_optimized_out_vars.js]
 [browser_console_private_browsing.js]
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
