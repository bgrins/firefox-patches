# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  45a3c4455770b2cca8fa1830e181c947054a1042
Bug 1243968 - browser_console_variables_view_while_debugging.js

diff --git a/devtools/client/webconsole/test/browser_console_variables_view_while_debugging.js b/devtools/client/webconsole/test/browser_console_variables_view_while_debugging.js
--- a/devtools/client/webconsole/test/browser_console_variables_view_while_debugging.js
+++ b/devtools/client/webconsole/test/browser_console_variables_view_while_debugging.js
@@ -7,120 +7,96 @@
 // from the js debugger, when changing the value of a property in the variables
 // view.
 
 "use strict";
 
 const TEST_URI = "http://example.com/browser/devtools/client/webconsole/" +
                  "test/test-eval-in-stackframe.html";
 
-var gWebConsole, gJSTerm, gDebuggerWin, gThread, gDebuggerController,
-    gStackframes, gVariablesView;
+add_task(function*() {
+  yield loadTab(TEST_URI);
+  let hud = yield openConsole();
 
-function test() {
-  loadTab(TEST_URI).then(() => {
-    openConsole().then(consoleOpened);
+  let dbgPanel = yield openDebugger();
+  yield waitForFrameAdded(dbgPanel);
+  yield openConsole();
+  yield testVariablesView(hud);
+});
+
+function* waitForFrameAdded(dbgPanel) {
+  let thread = dbgPanel.panelWin.DebuggerController.activeThread;
+
+  info("Waiting for onFramesAdded");
+  yield new Promise(resolve => {
+    thread.addOneTimeListener("framesadded", resolve);
+    info("firstCall()");
+    ContentTask.spawn(gBrowser.selectedBrowser, {}, function*() {
+      content.wrappedJSObject.firstCall();
+    });
   });
 }
 
-function consoleOpened(hud) {
-  gWebConsole = hud;
-  gJSTerm = hud.jsterm;
-
-  executeSoon(() => {
-    info("openDebugger");
-    openDebugger().then(debuggerOpened);
-  });
-}
-
-function debuggerOpened(aResult) {
-  gDebuggerWin = aResult.panelWin;
-  gDebuggerController = gDebuggerWin.DebuggerController;
-  gThread = gDebuggerController.activeThread;
-  gStackframes = gDebuggerController.StackFrames;
-
-  executeSoon(() => {
-    gThread.addOneTimeListener("framesadded", onFramesAdded);
-
-    info("firstCall()");
-    content.wrappedJSObject.firstCall();
-  });
-}
-
-function onFramesAdded() {
-  info("onFramesAdded");
-
-  executeSoon(() =>
-    openConsole().then(() =>
-      gJSTerm.execute("fooObj").then(onExecuteFooObj)
-    )
-  );
-}
-
-function onExecuteFooObj(msg) {
+function* testVariablesView(hud) {
+  let jsterm = hud.jsterm;
+  let msg = yield jsterm.execute("fooObj");
   ok(msg, "output message found");
   ok(msg.textContent.includes('{ testProp2: "testValue2" }'),
                               "message text check");
 
   let anchor = msg.querySelector("a");
   ok(anchor, "object link found");
 
-  gJSTerm.once("variablesview-fetched", onFooObjFetch);
+  info("Waiting for variable view to appear");
+  let variable = yield new Promise(resolve => {
+    jsterm.once("variablesview-fetched", (e, variable) => {
+      resolve(variable);
+    });
+    executeSoon(() => EventUtils.synthesizeMouse(anchor, 2, 2, {},
+                                                 hud.iframeWindow));
+  });
 
-  executeSoon(() => EventUtils.synthesizeMouse(anchor, 2, 2, {},
-                                               gWebConsole.iframeWindow));
-}
-
-function onFooObjFetch(aEvent, aVar) {
-  gVariablesView = aVar._variablesView;
-  ok(gVariablesView, "variables view object");
-
-  findVariableViewProperties(aVar, [
+  info("Waiting for findVariableViewProperties");
+  let results = yield findVariableViewProperties(variable, [
     { name: "testProp2", value: "testValue2" },
     { name: "testProp", value: "testValue", dontMatch: true },
-  ], { webconsole: gWebConsole }).then(onTestPropFound);
-}
+  ], { webconsole: hud });
 
-function onTestPropFound(aResults) {
-  let prop = aResults[0].matchedProp;
+  let prop = results[0].matchedProp;
   ok(prop, "matched the |testProp2| property in the variables view");
 
   // Check that property value updates work and that jsterm functions can be
   // used.
-  updateVariablesViewProperty({
+  variable = yield updateVariablesViewProperty({
     property: prop,
     field: "value",
     string: "document.title + foo2 + $('p')",
-    webconsole: gWebConsole
-  }).then(onFooObjFetchAfterUpdate);
-}
+    webconsole: hud
+  });
 
-function onFooObjFetchAfterUpdate(aVar) {
   info("onFooObjFetchAfterUpdate");
-  let para = content.wrappedJSObject.document.querySelector("p");
-  let expectedValue = content.document.title + "foo2SecondCall" + para;
+  let expectedValue = yield ContentTask.spawn(gBrowser.selectedBrowser, {}, function* () {
+    let para = content.wrappedJSObject.document.querySelector("p");
+    return content.document.title + "foo2SecondCall" + para;
+  });
 
-  findVariableViewProperties(aVar, [
+  results = yield findVariableViewProperties(variable, [
     { name: "testProp2", value: expectedValue },
-  ], { webconsole: gWebConsole }).then(onUpdatedTestPropFound);
-}
+  ], { webconsole: hud });
 
-function onUpdatedTestPropFound(aResults) {
-  let prop = aResults[0].matchedProp;
+  prop = results[0].matchedProp;
   ok(prop, "matched the updated |testProp2| property value");
 
   // Check that testProp2 was updated.
-  executeSoon(() => {
-    gJSTerm.execute("fooObj.testProp2").then(onExecuteFooObjTestProp2);
+  yield new Promise(resolve => {
+    executeSoon(() => {
+      jsterm.execute("fooObj.testProp2").then(resolve);
+    });
   });
+
+  expectedValue = yield ContentTask.spawn(gBrowser.selectedBrowser, {}, function* () {
+    let para = content.wrappedJSObject.document.querySelector("p");
+    return content.document.title + "foo2SecondCall" + para;
+  });
+
+  isnot(hud.outputNode.textContent.indexOf(expectedValue), -1,
+        "fooObj.testProp2 is correct");
 }
-
-function onExecuteFooObjTestProp2() {
-  let para = content.wrappedJSObject.document.querySelector("p");
-  let expected = content.document.title + "foo2SecondCall" + para;
-
-  isnot(gWebConsole.outputNode.textContent.indexOf(expected), -1,
-        "fooObj.testProp2 is correct");
-
-  gWebConsole = gJSTerm = gDebuggerWin = gThread = gDebuggerController =
-    gStackframes = gVariablesView = null;
-  executeSoon(finishTest);
-}
