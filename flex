# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  d6ea1920cbe2512c9b03ae3f5dd6846badd3f858
Bug 1198287 - Update flexbox usage for about:privatebrowsing to get rid of hardcoded localized widths;r=paolo

diff --git a/browser/components/privatebrowsing/content/aboutPrivateBrowsing.css b/browser/components/privatebrowsing/content/aboutPrivateBrowsing.css
--- a/browser/components/privatebrowsing/content/aboutPrivateBrowsing.css
+++ b/browser/components/privatebrowsing/content/aboutPrivateBrowsing.css
@@ -40,30 +40,39 @@ body[globalTpEnabled] .showGlobalTpDisab
   font-weight: 200;
   line-height: 60pt;
 }
 
 #main {
   padding: 0 2em;
   flex: 1;
   display: flex;
+  justify-content: center;
+  align-items: center;
+  max-width: 80%;
+}
+
+#main-contents {
+  display: flex;
+  flex: 1;
   flex-flow: row wrap;
   align-items: center;
-  justify-content: center;
 }
 
 .sectionHeader {
   font-size: 1.75em;
+  white-space: nowrap;
 }
 
 /* PRIVATE BROWSING SECTION */
 
 #privateBrowsingSection {
   margin: 1em;
   padding: 0 1em;
+  flex: 1;
 }
 
 ul {
   margin-bottom: 0;
   padding-inline-start: 8px;
 }
 
 li {
@@ -92,16 +101,17 @@ li {
   }
 }
 
 #list-area {
   display: flex;
   flex-direction: row;
   justify-content: flex-start;
   align-items: flex-start;
+  white-space: nowrap;
 }
 
 #list-area > div {
   margin-inline-end: 1em;
 }
 
 .list-header {
   font-weight: bold;
@@ -110,16 +120,17 @@ li {
 /* TRACKING PROTECTION SECTION */
 
 #trackingProtectionSection {
   margin: 1em;
   padding: 1em;
   text-align: center;
   border: lightgray 2px solid;
   border-radius: 10px;
+  flex: 1;
 }
 
 #tpEnabled,
 #tpDisabled {
   display: inline-block;
   margin-inline-start: 0.5em;
   border-radius: 3px;
   padding: 0.1em;
diff --git a/browser/components/privatebrowsing/content/aboutPrivateBrowsing.xhtml b/browser/components/privatebrowsing/content/aboutPrivateBrowsing.xhtml
--- a/browser/components/privatebrowsing/content/aboutPrivateBrowsing.xhtml
+++ b/browser/components/privatebrowsing/content/aboutPrivateBrowsing.xhtml
@@ -28,60 +28,60 @@
     <p class="showNormal">&aboutPrivateBrowsing.notPrivate;</p>
     <button xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul"
             id="startPrivateBrowsing"
             class="showNormal"
             label="&privatebrowsingpage.openPrivateWindow.label;"
             accesskey="&privatebrowsingpage.openPrivateWindow.accesskey;"/>
     <div id="bar" class="showPrivate">&privateBrowsing.title;</div>
     <div id="main" class="showPrivate">
-      <div id="privateBrowsingSection"
-           style="width: &aboutPrivateBrowsing.width;">
-        <div class="sectionHeader">&aboutPrivateBrowsing.title;</div>
-        <p>&aboutPrivateBrowsing.subtitle;</p>
-        <div id="list-area">
-          <div>
-            <div class="list-header">&aboutPrivateBrowsing.info.forgotten;</div>
-            <ul id="forgotten">
-              <li>&aboutPrivateBrowsing.info.history;</li>
-              <li>&aboutPrivateBrowsing.info.searches;</li>
-              <li>&aboutPrivateBrowsing.info.cookies;</li>
-              <li>&aboutPrivateBrowsing.info.temporaryFiles;</li>
-            </ul>
+      <div id="main-contents">
+        <div id="privateBrowsingSection">
+          <div class="sectionHeader">&aboutPrivateBrowsing.title;</div>
+          <p>&aboutPrivateBrowsing.subtitle;</p>
+          <div id="list-area">
+            <div>
+              <div class="list-header">&aboutPrivateBrowsing.info.forgotten;</div>
+              <ul id="forgotten">
+                <li>&aboutPrivateBrowsing.info.history;</li>
+                <li>&aboutPrivateBrowsing.info.searches;</li>
+                <li>&aboutPrivateBrowsing.info.cookies;</li>
+                <li>&aboutPrivateBrowsing.info.temporaryFiles;</li>
+              </ul>
+            </div>
+            <div>
+              <div class="list-header">&aboutPrivateBrowsing.info.kept;</div>
+              <ul id="kept">
+                <li>&aboutPrivateBrowsing.info.downloads;</li>
+                <li>&aboutPrivateBrowsing.info.bookmarks;</li>
+              </ul>
+            </div>
           </div>
-          <div>
-            <div class="list-header">&aboutPrivateBrowsing.info.kept;</div>
-            <ul id="kept">
-              <li>&aboutPrivateBrowsing.info.downloads;</li>
-              <li>&aboutPrivateBrowsing.info.bookmarks;</li>
-            </ul>
+          <p>&aboutPrivateBrowsing.note1;</p>
+          <a id="learnMore" target="_blank">&aboutPrivateBrowsing.learnMore;</a>
+        </div>
+        <div id="trackingProtectionSection">
+          <div class="sectionHeader">&trackingProtection.title;
+            <span id="tpEnabled"
+                  style="width: &trackingProtection.state.width;"
+                  class="showTpEnabled">&trackingProtection.state.enabled;</span>
+            <span id="tpDisabled"
+                  style="width: &trackingProtection.state.width;"
+                  class="showTpDisabled">&trackingProtection.state.disabled;</span>
           </div>
+          <p id="tpDiagram"/>
+          <p>&trackingProtection.description1;</p>
+          <!-- Use text links to implement plain styled buttons without an href. -->
+          <label xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul"
+                 id="disableTrackingProtection"
+                 class="text-link showTpEnabled showGlobalTpDisabled"
+                 value="&trackingProtection.disable;"/>
+          <label xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul"
+                 id="enableTrackingProtection"
+                 class="text-link showTpDisabled showGlobalTpDisabled"
+                 value="&trackingProtection.enable;"/>
+          <p id="tpStartTour"
+             class="showTpEnabled"><a id="startTour">&trackingProtection.startTour1;</a></p>
         </div>
-        <p>&aboutPrivateBrowsing.note1;</p>
-        <a id="learnMore" target="_blank">&aboutPrivateBrowsing.learnMore;</a>
-      </div>
-      <div id="trackingProtectionSection"
-           style="width: &trackingProtection.width;">
-        <div class="sectionHeader">&trackingProtection.title;
-          <span id="tpEnabled"
-                style="width: &trackingProtection.state.width;"
-                class="showTpEnabled">&trackingProtection.state.enabled;</span>
-          <span id="tpDisabled"
-                style="width: &trackingProtection.state.width;"
-                class="showTpDisabled">&trackingProtection.state.disabled;</span>
-        </div>
-        <p id="tpDiagram"/>
-        <p>&trackingProtection.description1;</p>
-        <!-- Use text links to implement plain styled buttons without an href. -->
-        <label xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul"
-               id="disableTrackingProtection"
-               class="text-link showTpEnabled showGlobalTpDisabled"
-               value="&trackingProtection.disable;"/>
-        <label xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul"
-               id="enableTrackingProtection"
-               class="text-link showTpDisabled showGlobalTpDisabled"
-               value="&trackingProtection.enable;"/>
-        <p id="tpStartTour"
-           class="showTpEnabled"><a id="startTour">&trackingProtection.startTour1;</a></p>
       </div>
     </div>
   </body>
 </html>
diff --git a/browser/locales/en-US/chrome/browser/aboutPrivateBrowsing.dtd b/browser/locales/en-US/chrome/browser/aboutPrivateBrowsing.dtd
--- a/browser/locales/en-US/chrome/browser/aboutPrivateBrowsing.dtd
+++ b/browser/locales/en-US/chrome/browser/aboutPrivateBrowsing.dtd
@@ -3,21 +3,16 @@
    - file, You can obtain one at http://mozilla.org/MPL/2.0/. -->
 
 <!ENTITY aboutPrivateBrowsing.notPrivate       "You are currently not in a private window.">
 <!ENTITY privatebrowsingpage.openPrivateWindow.label "Open a Private Window">
 <!ENTITY privatebrowsingpage.openPrivateWindow.accesskey "P">
 
 <!ENTITY privateBrowsing.title                 "Private Browsing">
 
-<!-- LOCALIZATION NOTE (aboutPrivateBrowsing.width):
-     Width of the Private Browsing section.
-     -->
-<!ENTITY aboutPrivateBrowsing.width            "25em">
-
 <!-- LOCALIZATION NOTE (aboutPrivateBrowsing.subtitle,
      aboutPrivateBrowsing.info.forgotten, aboutPrivateBrowsing.info.kept):
      These strings will be replaced by aboutPrivateBrowsing.forgotten and
      aboutPrivateBrowsing.kept when the new visual design lands (bug 1192625).
      -->
 <!ENTITY aboutPrivateBrowsing.title            "You're browsing privately">
 <!ENTITY aboutPrivateBrowsing.subtitle         "In this window, &brandShortName; will not remember any history.">
 
@@ -31,21 +26,16 @@
 <!ENTITY aboutPrivateBrowsing.kept             "&brandShortName; will keep:">
 <!ENTITY aboutPrivateBrowsing.info.kept        "Kept">
 <!ENTITY aboutPrivateBrowsing.info.downloads   "Downloads">
 <!ENTITY aboutPrivateBrowsing.info.bookmarks   "Bookmarks">
 
 <!ENTITY aboutPrivateBrowsing.note1            "Please note that your employer or Internet service provider can still track the pages you visit.">
 <!ENTITY aboutPrivateBrowsing.learnMore        "Learn More.">
 
-<!-- LOCALIZATION NOTE (trackingProtection.width):
-     Width of the Tracking Protection section. This should be enough to
-     accommodate the title as well as the enabled or disabled indicator.
-     -->
-<!ENTITY trackingProtection.width              "22em">
 <!ENTITY trackingProtection.title              "Tracking Protection">
 
 <!-- LOCALIZATION NOTE (trackingProtection.state.width):
      Width of the element representing the enabled or disabled indicator.
      -->
 <!ENTITY trackingProtection.state.width        "6ch">
 <!ENTITY trackingProtection.state.enabled      "ON">
 <!ENTITY trackingProtection.state.disabled     "OFF">
