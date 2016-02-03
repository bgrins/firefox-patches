# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  048d7a583502de7ba755b5099bc5cf96bf3e654c
Bug 1243995 - browser_console_optimized_out_vars.js

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
