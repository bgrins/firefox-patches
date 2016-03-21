# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  64a06fbc559d953d09d2681114b5e5ec6495416a
Bug 1243982 - Enable browser_webconsole_closure_inspection.js in e10s;r=linclark

diff --git a/devtools/client/webconsole/test/browser.ini b/devtools/client/webconsole/test/browser.ini
--- a/devtools/client/webconsole/test/browser.ini
+++ b/devtools/client/webconsole/test/browser.ini
@@ -289,17 +289,16 @@ skip-if = e10s && (os == 'win' || os == 
 skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_webconsole_show_subresource_security_errors.js]
 skip-if = e10s && (os == 'win' || os == 'mac') # Bug 1243987
 [browser_webconsole_cached_autocomplete.js]
 [browser_webconsole_change_font_size.js]
 [browser_webconsole_chrome.js]
 [browser_webconsole_clickable_urls.js]
 [browser_webconsole_closure_inspection.js]
-skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_webconsole_completion.js]
 [browser_webconsole_console_extras.js]
 [browser_webconsole_console_logging_api.js]
 [browser_webconsole_console_logging_workers_api.js]
 [browser_webconsole_console_trace_async.js]
 [browser_webconsole_count.js]
 [browser_webconsole_dont_navigate_on_doubleclick.js]
 [browser_webconsole_execution_scope.js]
diff --git a/devtools/client/webconsole/test/browser_webconsole_closure_inspection.js b/devtools/client/webconsole/test/browser_webconsole_closure_inspection.js
--- a/devtools/client/webconsole/test/browser_webconsole_closure_inspection.js
+++ b/devtools/client/webconsole/test/browser_webconsole_closure_inspection.js
@@ -27,19 +27,21 @@ function test() {
   }
 
   loadTab(TEST_URI).then(() => {
     openConsole().then((hud) => {
       openDebugger().then(({ toolbox, panelWin }) => {
         let deferred = promise.defer();
         fetchScopes(hud, toolbox, panelWin, deferred);
 
-        let button = content.document.querySelector("button");
-        ok(button, "button element found");
-        EventUtils.synthesizeMouseAtCenter(button, {}, content);
+        ContentTask.spawn(gBrowser.selectedBrowser, {}, function*() {
+          let button = content.document.querySelector("button");
+          ok(button, "button element found");
+          button.click();
+        });
 
         return deferred.promise;
       });
     });
   });
 }
 
 function consoleOpened(hud) {
