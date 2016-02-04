# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  0cae9cca480a57ac885e5bf04d11fb256a89d74d
Bug 1243995 - browser_console_optimized_out_vars.js

diff --git a/devtools/client/webconsole/test/browser.ini b/devtools/client/webconsole/test/browser.ini
--- a/devtools/client/webconsole/test/browser.ini
+++ b/devtools/client/webconsole/test/browser.ini
@@ -169,17 +169,16 @@ skip-if = e10s # Bug 1042253 - webconsol
 skip-if = e10s # Bug 1042253 - webconsole e10s tests
 [browser_console_keyboard_accessibility.js]
 [browser_console_log_inspectable_object.js]
 [browser_console_native_getters.js]
 [browser_console_navigation_marker.js]
 [browser_console_netlogging.js]
 [browser_console_nsiconsolemessage.js]
 [browser_console_optimized_out_vars.js]
-skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_console_private_browsing.js]
 skip-if = e10s # Bug 1042253 - webconsole e10s tests
 [browser_console_server_logging.js]
 [browser_console_variables_view.js]
 [browser_console_variables_view_filter.js]
 [browser_console_variables_view_dom_nodes.js]
 [browser_console_variables_view_dont_sort_non_sortable_classes_properties.js]
 [browser_console_variables_view_special_names.js]
diff --git a/devtools/client/webconsole/test/browser_console_optimized_out_vars.js b/devtools/client/webconsole/test/browser_console_optimized_out_vars.js
--- a/devtools/client/webconsole/test/browser_console_optimized_out_vars.js
+++ b/devtools/client/webconsole/test/browser_console_optimized_out_vars.js
@@ -16,21 +16,22 @@ function test() {
     let hud = yield openConsole(tab);
     let { toolbox, panel, panelWin } = yield openDebugger();
 
     let sources = panelWin.DebuggerView.Sources;
     yield panel.addBreakpoint({ actor: sources.values[0], line: 18 });
     yield ensureThreadClientState(panel, "resumed");
 
     let fetchedScopes = panelWin.once(panelWin.EVENTS.FETCHED_SCOPES);
-    let button = content.document.querySelector("button");
-    ok(button, "Button element found");
-    // Spin the event loop before causing the debuggee to pause, to allow
-    // this function to return first.
-    executeSoon(() => button.click());
+
+    // Cause the debuggee to pause
+    ContentTask.spawn(gBrowser.selectedBrowser, {}, function*() {
+      let button = content.document.querySelector("button");
+      button.click();
+    });
 
     yield fetchedScopes;
     ok(true, "Scopes were fetched");
 
     yield toolbox.selectTool("webconsole");
 
     // This is the meat of the test: evaluate the optimized out variable.
     hud.jsterm.execute("upvar");
