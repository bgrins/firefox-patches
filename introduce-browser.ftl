# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  cb5e80a45323ccd8377a48008c34dea768bb2fbd
Bug 1579540 - Introduce browser.ftl and load it in browser.xhtml

diff --git a/browser/base/content/browser.xhtml b/browser/base/content/browser.xhtml
--- a/browser/base/content/browser.xhtml
+++ b/browser/base/content/browser.xhtml
@@ -70,16 +70,17 @@
         sizemode="normal"
         retargetdocumentfocus="urlbar-input"
         persist="screenX screenY width height sizemode"
         data-l10n-sync="true">
 
 <linkset>
   <html:link rel="localization" href="branding/brand.ftl"/>
   <html:link rel="localization" href="browser/branding/sync-brand.ftl"/>
+  <html:link rel="localization" href="browser/browser.ftl"/>
   <html:link rel="localization" href="browser/menubar.ftl"/>
 </linkset>
 
 # All JS files which are needed by browser.xhtml and other top level windows to
 # support MacOS specific features *must* go into the global-scripts.inc file so
 # that they can be shared with macWindow.inc.xul.
 #include global-scripts.inc
 
diff --git a/browser/locales/en-US/browser/browser.ftl b/browser/locales/en-US/browser/browser.ftl
new file mode 100644
--- /dev/null
+++ b/browser/locales/en-US/browser/browser.ftl
@@ -0,0 +1,3 @@
+# This Source Code Form is subject to the terms of the Mozilla Public
+# License, v. 2.0. If a copy of the MPL was not distributed with this
+# file, You can obtain one at http://mozilla.org/MPL/2.0/.
