# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  4fb374a2299f21def20eebea0dce3832251caea6
Bug 1243959 - e10s fixes for browser_webconsole_autocomplete_in_debugger_stackframe.js;r=linclark

diff --git a/devtools/client/webconsole/test/browser.ini b/devtools/client/webconsole/test/browser.ini
--- a/devtools/client/webconsole/test/browser.ini
+++ b/devtools/client/webconsole/test/browser.ini
@@ -345,17 +345,16 @@ skip-if = e10s # Bug 1042253 - webconsol
 skip-if = e10s # Bug 1042253 - webconsole e10s tests (Linux debug timeout)
 [browser_webconsole_trackingprotection_errors.js]
 tags = trackingprotection
 [browser_webconsole_view_source.js]
 [browser_webconsole_reflow.js]
 [browser_webconsole_log_file_filter.js]
 [browser_webconsole_expandable_timestamps.js]
 [browser_webconsole_autocomplete_in_debugger_stackframe.js]
-skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_webconsole_autocomplete_popup_close_on_tab_switch.js]
 skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_webconsole_autocomplete-properties-with-non-alphanumeric-names.js]
 [browser_console_hide_jsterm_when_devtools_chrome_enabled_false.js]
 [browser_console_history_persist.js]
 [browser_webconsole_output_01.js]
 skip-if = e10s # Bug 1042253 - webconsole e10s tests
 [browser_webconsole_output_02.js]
diff --git a/devtools/client/webconsole/test/browser_webconsole_autocomplete_in_debugger_stackframe.js b/devtools/client/webconsole/test/browser_webconsole_autocomplete_in_debugger_stackframe.js
--- a/devtools/client/webconsole/test/browser_webconsole_autocomplete_in_debugger_stackframe.js
+++ b/devtools/client/webconsole/test/browser_webconsole_autocomplete_in_debugger_stackframe.js
@@ -6,62 +6,56 @@
 // Test that makes sure web console autocomplete happens in the user-selected
 // stackframe from the js debugger.
 
 "use strict";
 
 const TEST_URI = "http://example.com/browser/devtools/client/webconsole/" +
                  "test/test-autocomplete-in-stackframe.html";
 
-var testDriver, gStackframes;
+var gStackframes;
 
-function test() {
-  requestLongerTimeout(2);
-  loadTab(TEST_URI).then(() => {
-    openConsole().then((hud) => {
-      testDriver = testCompletion(hud);
-      testDriver.next();
-    });
-  });
-}
-
-function testNext() {
-  executeSoon(function() {
-    testDriver.next();
-  });
-}
+requestLongerTimeout(2);
+add_task(function*() {
+  yield loadTab(TEST_URI);
+  let hud = yield openConsole();
+  yield testCompletion(hud);
+  gStackframes = null;
+});
 
 function* testCompletion(hud) {
   let jsterm = hud.jsterm;
   let input = jsterm.inputNode;
   let popup = jsterm.autocompletePopup;
 
   // Test that document.title gives string methods. Native getters must execute.
   input.value = "document.title.";
   input.setSelectionRange(input.value.length, input.value.length);
-  jsterm.complete(jsterm.COMPLETE_HINT_ONLY, testNext);
-  yield undefined;
+  yield new Promise(resolve => {
+    jsterm.complete(jsterm.COMPLETE_HINT_ONLY, resolve);
+  });
 
   let newItems = popup.getItems();
   ok(newItems.length > 0, "'document.title.' gave a list of suggestions");
   ok(newItems.some(function(item) {
        return item.label == "substr";
      }), "autocomplete results do contain substr");
   ok(newItems.some(function(item) {
        return item.label == "toLowerCase";
      }), "autocomplete results do contain toLowerCase");
   ok(newItems.some(function(item) {
        return item.label == "strike";
      }), "autocomplete results do contain strike");
 
   // Test if 'f' gives 'foo1' but not 'foo2' or 'foo3'
   input.value = "f";
   input.setSelectionRange(1, 1);
-  jsterm.complete(jsterm.COMPLETE_HINT_ONLY, testNext);
-  yield undefined;
+  yield new Promise(resolve => {
+    jsterm.complete(jsterm.COMPLETE_HINT_ONLY, resolve);
+  });
 
   newItems = popup.getItems();
   ok(newItems.length > 0, "'f' gave a list of suggestions");
   ok(!newItems.every(function(item) {
        return item.label != "foo1";
      }), "autocomplete results do contain foo1");
   ok(!newItems.every(function(item) {
        return item.label != "foo1Obj";
@@ -77,48 +71,56 @@ function* testCompletion(hud) {
      }), "autocomplete results do not contain foo3");
   ok(newItems.every(function(item) {
        return item.label != "foo3Obj";
      }), "autocomplete results do not contain foo3Obj");
 
   // Test if 'foo1Obj.' gives 'prop1' and 'prop2'
   input.value = "foo1Obj.";
   input.setSelectionRange(8, 8);
-  jsterm.complete(jsterm.COMPLETE_HINT_ONLY, testNext);
-  yield undefined;
+  yield new Promise(resolve => {
+    jsterm.complete(jsterm.COMPLETE_HINT_ONLY, resolve);
+  });
 
   newItems = popup.getItems();
   ok(!newItems.every(function(item) {
        return item.label != "prop1";
      }), "autocomplete results do contain prop1");
   ok(!newItems.every(function(item) {
        return item.label != "prop2";
      }), "autocomplete results do contain prop2");
 
   // Test if 'foo1Obj.prop2.' gives 'prop21'
   input.value = "foo1Obj.prop2.";
   input.setSelectionRange(14, 14);
-  jsterm.complete(jsterm.COMPLETE_HINT_ONLY, testNext);
-  yield undefined;
+  yield new Promise(resolve => {
+    jsterm.complete(jsterm.COMPLETE_HINT_ONLY, resolve);
+  });
 
   newItems = popup.getItems();
   ok(!newItems.every(function(item) {
        return item.label != "prop21";
      }), "autocomplete results do contain prop21");
 
-  info("openDebugger");
-  executeSoon(() => openDebugger().then(debuggerOpened));
-  yield undefined;
+  info("Opening Debugger");
+  let dbg = yield openDebugger();
+
+  info("Waiting for pause");
+  yield pauseDebugger(dbg);
+
+  info("Opening Console again");
+  yield openConsole();
 
   // From this point on the
   // Test if 'f' gives 'foo3' and 'foo1' but not 'foo2'
   input.value = "f";
   input.setSelectionRange(1, 1);
-  jsterm.complete(jsterm.COMPLETE_HINT_ONLY, testNext);
-  yield undefined;
+  yield new Promise(resolve => {
+    jsterm.complete(jsterm.COMPLETE_HINT_ONLY, resolve);
+  });
 
   newItems = popup.getItems();
   ok(newItems.length > 0, "'f' gave a list of suggestions");
   ok(!newItems.every(function(item) {
        return item.label != "foo3";
      }), "autocomplete results do contain foo3");
   ok(!newItems.every(function(item) {
        return item.label != "foo3Obj";
@@ -131,29 +133,29 @@ function* testCompletion(hud) {
      }), "autocomplete results do contain foo1Obj");
   ok(newItems.every(function(item) {
        return item.label != "foo2";
      }), "autocomplete results do not contain foo2");
   ok(newItems.every(function(item) {
        return item.label != "foo2Obj";
      }), "autocomplete results do not contain foo2Obj");
 
-  openDebugger().then(() => {
-    gStackframes.selectFrame(1);
+  yield openDebugger();
 
-    info("openConsole");
-    executeSoon(() => openConsole().then(() => testDriver.next()));
-  });
-  yield undefined;
+  gStackframes.selectFrame(1);
+
+  info("openConsole");
+  yield openConsole();
 
   // Test if 'f' gives 'foo2' and 'foo1' but not 'foo3'
   input.value = "f";
   input.setSelectionRange(1, 1);
-  jsterm.complete(jsterm.COMPLETE_HINT_ONLY, testNext);
-  yield undefined;
+  yield new Promise(resolve => {
+    jsterm.complete(jsterm.COMPLETE_HINT_ONLY, resolve);
+  });
 
   newItems = popup.getItems();
   ok(newItems.length > 0, "'f' gave a list of suggestions");
   ok(!newItems.every(function(item) {
        return item.label != "foo2";
      }), "autocomplete results do contain foo2");
   ok(!newItems.every(function(item) {
        return item.label != "foo2Obj";
@@ -169,75 +171,67 @@ function* testCompletion(hud) {
      }), "autocomplete results do not contain foo3");
   ok(newItems.every(function(item) {
        return item.label != "foo3Obj";
      }), "autocomplete results do not contain foo3Obj");
 
   // Test if 'foo2Obj.' gives 'prop1'
   input.value = "foo2Obj.";
   input.setSelectionRange(8, 8);
-  jsterm.complete(jsterm.COMPLETE_HINT_ONLY, testNext);
-  yield undefined;
+  yield new Promise(resolve => {
+    jsterm.complete(jsterm.COMPLETE_HINT_ONLY, resolve);
+  });
 
   newItems = popup.getItems();
   ok(!newItems.every(function(item) {
        return item.label != "prop1";
      }), "autocomplete results do contain prop1");
 
   // Test if 'foo2Obj.prop1.' gives 'prop11'
   input.value = "foo2Obj.prop1.";
   input.setSelectionRange(14, 14);
-  jsterm.complete(jsterm.COMPLETE_HINT_ONLY, testNext);
-  yield undefined;
+  yield new Promise(resolve => {
+    jsterm.complete(jsterm.COMPLETE_HINT_ONLY, resolve);
+  });
 
   newItems = popup.getItems();
   ok(!newItems.every(function(item) {
        return item.label != "prop11";
      }), "autocomplete results do contain prop11");
 
   // Test if 'foo2Obj.prop1.prop11.' gives suggestions for a string
   // i.e. 'length'
   input.value = "foo2Obj.prop1.prop11.";
   input.setSelectionRange(21, 21);
-  jsterm.complete(jsterm.COMPLETE_HINT_ONLY, testNext);
-  yield undefined;
+  yield new Promise(resolve => {
+    jsterm.complete(jsterm.COMPLETE_HINT_ONLY, resolve);
+  });
 
   newItems = popup.getItems();
   ok(!newItems.every(function(item) {
        return item.label != "length";
      }), "autocomplete results do contain length");
 
   // Test if 'foo1Obj[0].' throws no errors.
   input.value = "foo2Obj[0].";
   input.setSelectionRange(11, 11);
-  jsterm.complete(jsterm.COMPLETE_HINT_ONLY, testNext);
-  yield undefined;
+  yield new Promise(resolve => {
+    jsterm.complete(jsterm.COMPLETE_HINT_ONLY, resolve);
+  });
 
   newItems = popup.getItems();
   is(newItems.length, 0, "no items for foo2Obj[0]");
-
-  testDriver = null;
-  executeSoon(finishUp);
-  yield undefined;
 }
 
-function debuggerOpened(aResult) {
+function pauseDebugger(aResult) {
   let debuggerWin = aResult.panelWin;
   let debuggerController = debuggerWin.DebuggerController;
   let thread = debuggerController.activeThread;
   gStackframes = debuggerController.StackFrames;
+  return new Promise(resolve => {
+    thread.addOneTimeListener("framesadded", resolve);
 
-  executeSoon(() => {
-    thread.addOneTimeListener("framesadded", onFramesAdded);
     info("firstCall()");
-    content.wrappedJSObject.firstCall();
+    ContentTask.spawn(gBrowser.selectedBrowser, {}, function*() {
+      content.wrappedJSObject.firstCall();
+    });
   });
 }
-
-function onFramesAdded() {
-  info("onFramesAdded, openConsole() now");
-  executeSoon(() => openConsole().then(testNext));
-}
-
-function finishUp() {
-  testDriver = gStackframes = null;
-  finishTest();
-}
