# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  df56e046dc0a163d87aedc2a915614a3d4e053f6
Bug 1265565 - Fix eslint issues in browser_webconsole_bug_752559_ineffective_iframe_sandbox_warning.js;r=me

MozReview-Commit-ID: G054kUVXbcr

diff --git a/devtools/client/framework/test/shared-head.js b/devtools/client/framework/test/shared-head.js
--- a/devtools/client/framework/test/shared-head.js
+++ b/devtools/client/framework/test/shared-head.js
@@ -91,21 +91,18 @@ DevToolsUtils.testing = true;
 registerCleanupFunction(() => {
   DevToolsUtils.testing = false;
   Services.prefs.clearUserPref("devtools.dump.emit");
   Services.prefs.clearUserPref("devtools.toolbox.host");
   Services.prefs.clearUserPref("devtools.toolbox.previousHost");
 });
 
 registerCleanupFunction(function* cleanup() {
-  let target = TargetFactory.forTab(gBrowser.selectedTab);
-  yield gDevTools.closeToolbox(target);
-
   while (gBrowser.tabs.length > 1) {
-    gBrowser.removeCurrentTab();
+    yield closeTabAndToolbox(gBrowser.selectedTab);
   }
 });
 
 /**
  * Add a new test tab in the browser and load the given url.
  * @param {String} url The url to be loaded in the new tab
  * @return a promise that resolves to the tab object when the url is loaded
  */
diff --git a/devtools/client/webconsole/test/browser_webconsole_bug_752559_ineffective_iframe_sandbox_warning.js b/devtools/client/webconsole/test/browser_webconsole_bug_752559_ineffective_iframe_sandbox_warning.js
--- a/devtools/client/webconsole/test/browser_webconsole_bug_752559_ineffective_iframe_sandbox_warning.js
+++ b/devtools/client/webconsole/test/browser_webconsole_bug_752559_ineffective_iframe_sandbox_warning.js
@@ -3,80 +3,79 @@
 /* Any copyright is dedicated to the Public Domain.
  * http://creativecommons.org/publicdomain/zero/1.0/ */
 
 // Tests that warnings about ineffective iframe sandboxing are logged to the
 // web console when necessary (and not otherwise).
 
 "use strict";
 
-const TEST_URI_WARNING = "http://example.com/browser/devtools/client/" +
-                         "webconsole/test/test-bug-752559-ineffective-iframe-sandbox-warning0.html";
+const TEST_URI_WARNING = "http://example.com/browser/devtools/client/webconsole/test/test-bug-752559-ineffective-iframe-sandbox-warning0.html";
 const TEST_URI_NOWARNING = [
   "http://example.com/browser/devtools/client/webconsole/test/test-bug-752559-ineffective-iframe-sandbox-warning1.html",
   "http://example.com/browser/devtools/client/webconsole/test/test-bug-752559-ineffective-iframe-sandbox-warning2.html",
   "http://example.com/browser/devtools/client/webconsole/test/test-bug-752559-ineffective-iframe-sandbox-warning3.html",
   "http://example.com/browser/devtools/client/webconsole/test/test-bug-752559-ineffective-iframe-sandbox-warning4.html",
   "http://example.com/browser/devtools/client/webconsole/test/test-bug-752559-ineffective-iframe-sandbox-warning5.html"
 ];
 
 const INEFFECTIVE_IFRAME_SANDBOXING_MSG = "An iframe which has both " +
   "allow-scripts and allow-same-origin for its sandbox attribute can remove " +
   "its sandboxing.";
 const SENTINEL_MSG = "testing ineffective sandboxing message";
 
-function test() {
-  loadTab(TEST_URI_WARNING).then(() => {
-    openConsole().then((hud) => {
-      ContentTask.spawn(gBrowser.selectedBrowser, SENTINEL_MSG, function*(msg) {
-        content.console.log(msg);
-      });
-      waitForMessages({
-        webconsole: hud,
-        messages: [
-          {
-            name: "Ineffective iframe sandboxing warning displayed successfully",
-            text: INEFFECTIVE_IFRAME_SANDBOXING_MSG,
-            category: CATEGORY_SECURITY,
-            severity: SEVERITY_WARNING
-          },
-          {
-            text: SENTINEL_MSG,
-            severity: SEVERITY_LOG
-          }
-        ]
-      }).then(() => {
-        let msgs = hud.outputNode.querySelectorAll(".message[category=security]");
-        is(msgs.length, 1, "one security message");
-        testNoWarning(0);
-      });
-    });
+add_task(function* () {
+  yield testYesWarning();
+
+  for (let id = 0; id < TEST_URI_NOWARNING.length; id++) {
+    yield testNoWarning(id);
+  }
+});
+
+function* testYesWarning() {
+  yield loadTab(TEST_URI_WARNING);
+  let hud = yield openConsole();
+
+  ContentTask.spawn(gBrowser.selectedBrowser, SENTINEL_MSG, function* (msg) {
+    content.console.log(msg);
   });
+
+  yield waitForMessages({
+    webconsole: hud,
+    messages: [
+      {
+        name: "Ineffective iframe sandboxing warning displayed successfully",
+        text: INEFFECTIVE_IFRAME_SANDBOXING_MSG,
+        category: CATEGORY_SECURITY,
+        severity: SEVERITY_WARNING
+      },
+      {
+        text: SENTINEL_MSG,
+        severity: SEVERITY_LOG
+      }
+    ]
+  });
+
+  let msgs = hud.outputNode.querySelectorAll(".message[category=security]");
+  is(msgs.length, 1, "one security message");
 }
 
-function testNoWarning(id) {
-  loadTab(TEST_URI_NOWARNING[id]).then(() => {
-    openConsole().then((hud) => {
-      ContentTask.spawn(gBrowser.selectedBrowser, SENTINEL_MSG, function*(msg) {
-        content.console.log(msg);
-      });
-      waitForMessages({
-        webconsole: hud,
-        messages: [
-          {
-            text: SENTINEL_MSG,
-            severity: SEVERITY_LOG
-          }
-        ]
-      }).then(() => {
-        let msgs = hud.outputNode.querySelectorAll(".message[category=security]");
-        is(msgs.length, 0, "no security messages (case " + id + ")");
+function* testNoWarning(id) {
+  yield loadTab(TEST_URI_NOWARNING[id]);
+  let hud = yield openConsole();
 
-        id += 1;
-        if (id < TEST_URI_NOWARNING.length) {
-          testNoWarning(id);
-        } else {
-          finishTest();
-        }
-      });
-    });
+  ContentTask.spawn(gBrowser.selectedBrowser, SENTINEL_MSG, function* (msg) {
+    content.console.log(msg);
   });
+
+  yield waitForMessages({
+    webconsole: hud,
+    messages: [
+      {
+        text: SENTINEL_MSG,
+        severity: SEVERITY_LOG
+      }
+    ]
+  });
+
+  let msgs = hud.outputNode.querySelectorAll(".message[category=security]");
+  is(msgs.length, 0, "no security messages (case " + id + ")");
 }
