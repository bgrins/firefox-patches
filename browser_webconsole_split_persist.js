# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  64a06fbc559d953d09d2681114b5e5ec6495416a
Bug 1243993 - Enable browser_webconsole_split_persist.js in e10s;r=me

diff --git a/devtools/client/webconsole/test/browser.ini b/devtools/client/webconsole/test/browser.ini
--- a/devtools/client/webconsole/test/browser.ini
+++ b/devtools/client/webconsole/test/browser.ini
@@ -328,17 +328,16 @@ skip-if = e10s # Bug 1042253 - webconsol
 [browser_webconsole_output_order.js]
 [browser_webconsole_property_provider.js]
 skip-if = e10s # Bug 1042253 - webconsole tests disabled with e10s
 [browser_webconsole_scratchpad_panel_link.js]
 [browser_webconsole_split.js]
 [browser_webconsole_split_escape_key.js]
 [browser_webconsole_split_focus.js]
 [browser_webconsole_split_persist.js]
-skip-if = e10s # Bug 1042253 - webconsole e10s tests (Linux debug timeout)
 [browser_webconsole_trackingprotection_errors.js]
 tags = trackingprotection
 [browser_webconsole_view_source.js]
 [browser_webconsole_reflow.js]
 [browser_webconsole_log_file_filter.js]
 [browser_webconsole_expandable_timestamps.js]
 [browser_webconsole_autocomplete_in_debugger_stackframe.js]
 [browser_webconsole_autocomplete_popup_close_on_tab_switch.js]
