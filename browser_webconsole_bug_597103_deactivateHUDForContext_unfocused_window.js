# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  a7ff28d1cbdc429de42d20c22f58d14fd9000dd7
Bug 1243970 - browser_webconsole_bug_597103_deactivateHUDForContext_unfocused_window.js

diff --git a/devtools/client/webconsole/test/browser_webconsole_bug_597103_deactivateHUDForContext_unfocused_window.js b/devtools/client/webconsole/test/browser_webconsole_bug_597103_deactivateHUDForContext_unfocused_window.js
--- a/devtools/client/webconsole/test/browser_webconsole_bug_597103_deactivateHUDForContext_unfocused_window.js
+++ b/devtools/client/webconsole/test/browser_webconsole_bug_597103_deactivateHUDForContext_unfocused_window.js
@@ -80,17 +80,17 @@ function tab2Loaded(aEvent) {
 
     executeSoon(function() {
       win2.close();
       tab1 = tab2 = win1 = win2 = null;
       finishTest();
     });
   }
 
-  waitForFocus(openConsoles, tab2.linkedBrowser.contentWindow);
+  openConsoles();
 }
 
 function test() {
   loadTab(TEST_URI).then(() => {
     tab1 = gBrowser.selectedTab;
     win1 = window;
     tab1Loaded();
   });
