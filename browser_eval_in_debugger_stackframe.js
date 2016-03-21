# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  64a06fbc559d953d09d2681114b5e5ec6495416a
Bug 1243991 - Enable browser_eval_in_debugger_stackframe.js in e10s;r=linclark

diff --git a/devtools/client/webconsole/test/browser.ini b/devtools/client/webconsole/test/browser.ini
--- a/devtools/client/webconsole/test/browser.ini
+++ b/devtools/client/webconsole/test/browser.ini
@@ -177,17 +177,16 @@ skip-if = e10s # Bug 1042253 - webconsol
 [browser_console_variables_view.js]
 [browser_console_variables_view_filter.js]
 [browser_console_variables_view_dom_nodes.js]
 [browser_console_variables_view_dont_sort_non_sortable_classes_properties.js]
 [browser_console_variables_view_special_names.js]
 [browser_console_variables_view_while_debugging.js]
 [browser_console_variables_view_while_debugging_and_inspecting.js]
 [browser_eval_in_debugger_stackframe.js]
-skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_eval_in_debugger_stackframe2.js]
 [browser_jsterm_inspect.js]
 skip-if = e10s && debug && (os == 'win' || os == 'mac') # Bug 1243966
 [browser_longstring_hang.js]
 [browser_output_breaks_after_console_dir_uninspectable.js]
 [browser_output_longstring_expand.js]
 [browser_repeated_messages_accuracy.js]
 [browser_result_format_as_string.js]
diff --git a/devtools/client/webconsole/test/browser_eval_in_debugger_stackframe.js b/devtools/client/webconsole/test/browser_eval_in_debugger_stackframe.js
--- a/devtools/client/webconsole/test/browser_eval_in_debugger_stackframe.js
+++ b/devtools/client/webconsole/test/browser_eval_in_debugger_stackframe.js
@@ -76,17 +76,19 @@ function onExecuteFooAndFoo2() {
   executeSoon(() => {
     gJSTerm.clearOutput();
 
     info("openDebugger");
     openDebugger().then(() => {
       gThread.addOneTimeListener("framesadded", onFramesAdded);
 
       info("firstCall()");
-      content.wrappedJSObject.firstCall();
+      ContentTask.spawn(gBrowser.selectedBrowser, {}, function*() {
+        content.wrappedJSObject.firstCall();
+      });
     });
   });
 }
 
 function onFramesAdded() {
   info("onFramesAdded, openConsole() now");
   executeSoon(() =>
     openConsole().then(() =>
