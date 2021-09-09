# HG changeset patch
# Parent  1d13f4438c9ef7a42f23c2fbd2cbd8005245d61b

diff --git a/.hgignore b/.hgignore
--- a/.hgignore
+++ b/.hgignore
@@ -217,3 +217,5 @@ toolkit/components/certviewer/content/pa
 
 # Ignore Rust/Cargo output from running `cargo` directly for image_builder docker image
 ^taskcluster/docker/image_builder/build-image/target
+
+browser/components/companion
diff --git a/browser/actors/LinkHandlerParent.jsm b/browser/actors/LinkHandlerParent.jsm
--- a/browser/actors/LinkHandlerParent.jsm
+++ b/browser/actors/LinkHandlerParent.jsm
@@ -7,6 +7,9 @@
 const EXPORTED_SYMBOLS = ["LinkHandlerParent"];
 
 const { Services } = ChromeUtils.import("resource://gre/modules/Services.jsm");
+const { CompanionService } = ChromeUtils.import(
+  "resource:///modules/CompanionService.jsm"
+);
 
 ChromeUtils.defineModuleGetter(
   this,
@@ -143,7 +146,12 @@ class LinkHandlerParent extends JSWindow
       } catch (ex) {
         return;
       }
+    } else if (canUseForTab) {
+      CompanionService.updateTabIcon(tab, iconURI.spec);
+    } else {
+      CompanionService.updateTabRichIcon(tab, iconURI.spec);
     }
+
     if (canStoreIcon) {
       try {
         PlacesUIUtils.loadFavicon(
diff --git a/browser/base/content/browser.js b/browser/base/content/browser.js
--- a/browser/base/content/browser.js
+++ b/browser/base/content/browser.js
@@ -28,6 +28,7 @@ XPCOMUtils.defineLazyModuleGetters(this,
   BrowserWindowTracker: "resource:///modules/BrowserWindowTracker.jsm",
   CFRPageActions: "resource://activity-stream/lib/CFRPageActions.jsm",
   CharsetMenu: "resource://gre/modules/CharsetMenu.jsm",
+  CompanionService: "resource:///modules/CompanionService.jsm",
   Color: "resource://gre/modules/Color.jsm",
   ContextualIdentityService:
     "resource://gre/modules/ContextualIdentityService.jsm",
@@ -1777,6 +1778,10 @@ var gBrowserInit = {
 
     window.addEventListener("AppCommand", HandleAppCommandEvent, true);
 
+    gBrowser.addEventListener("TabOpen", event => {
+      CompanionService.onTabOpened(event.target);
+    });
+
     // These routines add message listeners. They must run before
     // loading the frame script to ensure that we don't miss any
     // message sent between when the frame script is loaded and when
@@ -5785,6 +5790,8 @@ var TabsProgressListener = {
 
     FullZoom.onLocationChange(aLocationURI, false, aBrowser);
     CaptivePortalWatcher.onLocationChange(aBrowser);
+
+    CompanionService.onLocationChange(tab, aLocationURI);
   },
 
   onLinkIconAvailable(browser, dataURI, iconURI) {
diff --git a/browser/components/BrowserGlue.jsm b/browser/components/BrowserGlue.jsm
--- a/browser/components/BrowserGlue.jsm
+++ b/browser/components/BrowserGlue.jsm
@@ -387,6 +387,19 @@ let JSWINDOWACTORS = {
     allFrames: true,
   },
 
+  Companion: {
+    parent: {
+      moduleURI: "resource:///actors/CompanionParent.jsm",
+    },
+    child: {
+      moduleURI: "resource:///actors/CompanionChild.jsm",
+      events: {
+        DOMContentLoaded: { capture: true },
+      },
+    },
+    matches: ["http://*/*", "https://*/*"],
+  },
+
   // Collects description and icon information from meta tags.
   ContentMeta: {
     parent: {
diff --git a/browser/components/moz.build b/browser/components/moz.build
--- a/browser/components/moz.build
+++ b/browser/components/moz.build
@@ -30,6 +30,7 @@ DIRS += [
     "about",
     "aboutlogins",
     "attribution",
+    "companion",
     "contextualidentity",
     "customizableui",
     "doh",
diff --git a/package-lock.json b/package-lock.json
--- a/package-lock.json
+++ b/package-lock.json
@@ -1,7 +1,3414 @@
 {
   "name": "mozilla-central",
+  "lockfileVersion": 2,
   "requires": true,
-  "lockfileVersion": 1,
+  "packages": {
+    "": {
+      "name": "mozilla-central",
+      "license": "MPL-2.0",
+      "devDependencies": {
+        "@babel/core": "7.8.3",
+        "@typescript-eslint/eslint-plugin": "^4.12.0",
+        "@typescript-eslint/parser": "^4.10.0",
+        "babel-eslint": "10.1.0",
+        "eslint": "7.8.0",
+        "eslint-config-prettier": "6.10.0",
+        "eslint-plugin-babel": "5.3.0",
+        "eslint-plugin-fetch-options": "0.0.5",
+        "eslint-plugin-file-header": "0.0.1",
+        "eslint-plugin-flowtype": "4.6.0",
+        "eslint-plugin-html": "6.0.0",
+        "eslint-plugin-import": "2.20.1",
+        "eslint-plugin-jest": "23.20.0",
+        "eslint-plugin-jsx-a11y": "6.2.3",
+        "eslint-plugin-mozilla": "file:tools/lint/eslint/eslint-plugin-mozilla",
+        "eslint-plugin-no-unsanitized": "3.1.4",
+        "eslint-plugin-prettier": "3.1.2",
+        "eslint-plugin-react": "7.18.3",
+        "eslint-plugin-spidermonkey-js": "file:tools/lint/eslint/eslint-plugin-spidermonkey-js",
+        "prettier": "1.19.1",
+        "typescript": "^4.1.3",
+        "yarn": "1.22.0"
+      }
+    },
+    "node_modules/@babel/code-frame": {
+      "version": "7.10.4",
+      "resolved": "https://registry.npmjs.org/@babel/code-frame/-/code-frame-7.10.4.tgz",
+      "integrity": "sha512-vG6SvB6oYEhvgisZNFRmRCUkLz11c7rp+tbNTynGqc6mS1d5ATd/sGyV6W0KZZnXRKMTzZDRgQT3Ou9jhpAfUg==",
+      "dev": true,
+      "dependencies": {
+        "@babel/highlight": "^7.10.4"
+      }
+    },
+    "node_modules/@babel/core": {
+      "version": "7.8.3",
+      "resolved": "https://registry.npmjs.org/@babel/core/-/core-7.8.3.tgz",
+      "integrity": "sha512-4XFkf8AwyrEG7Ziu3L2L0Cv+WyY47Tcsp70JFmpftbAA1K7YL/sgE9jh9HyNj08Y/U50ItUchpN0w6HxAoX1rA==",
+      "dev": true,
+      "dependencies": {
+        "@babel/code-frame": "^7.8.3",
+        "@babel/generator": "^7.8.3",
+        "@babel/helpers": "^7.8.3",
+        "@babel/parser": "^7.8.3",
+        "@babel/template": "^7.8.3",
+        "@babel/traverse": "^7.8.3",
+        "@babel/types": "^7.8.3",
+        "convert-source-map": "^1.7.0",
+        "debug": "^4.1.0",
+        "gensync": "^1.0.0-beta.1",
+        "json5": "^2.1.0",
+        "lodash": "^4.17.13",
+        "resolve": "^1.3.2",
+        "semver": "^5.4.1",
+        "source-map": "^0.5.0"
+      }
+    },
+    "node_modules/@babel/generator": {
+      "version": "7.10.5",
+      "resolved": "https://registry.npmjs.org/@babel/generator/-/generator-7.10.5.tgz",
+      "integrity": "sha512-3vXxr3FEW7E7lJZiWQ3bM4+v/Vyr9C+hpolQ8BGFr9Y8Ri2tFLWTixmwKBafDujO1WVah4fhZBeU1bieKdghig==",
+      "dev": true,
+      "dependencies": {
+        "@babel/types": "^7.10.5",
+        "jsesc": "^2.5.1",
+        "source-map": "^0.5.0"
+      }
+    },
+    "node_modules/@babel/helper-function-name": {
+      "version": "7.10.4",
+      "resolved": "https://registry.npmjs.org/@babel/helper-function-name/-/helper-function-name-7.10.4.tgz",
+      "integrity": "sha512-YdaSyz1n8gY44EmN7x44zBn9zQ1Ry2Y+3GTA+3vH6Mizke1Vw0aWDM66FOYEPw8//qKkmqOckrGgTYa+6sceqQ==",
+      "dev": true,
+      "dependencies": {
+        "@babel/helper-get-function-arity": "^7.10.4",
+        "@babel/template": "^7.10.4",
+        "@babel/types": "^7.10.4"
+      }
+    },
+    "node_modules/@babel/helper-get-function-arity": {
+      "version": "7.10.4",
+      "resolved": "https://registry.npmjs.org/@babel/helper-get-function-arity/-/helper-get-function-arity-7.10.4.tgz",
+      "integrity": "sha512-EkN3YDB+SRDgiIUnNgcmiD361ti+AVbL3f3Henf6dqqUyr5dMsorno0lJWJuLhDhkI5sYEpgj6y9kB8AOU1I2A==",
+      "dev": true,
+      "dependencies": {
+        "@babel/types": "^7.10.4"
+      }
+    },
+    "node_modules/@babel/helper-split-export-declaration": {
+      "version": "7.10.4",
+      "resolved": "https://registry.npmjs.org/@babel/helper-split-export-declaration/-/helper-split-export-declaration-7.10.4.tgz",
+      "integrity": "sha512-pySBTeoUff56fL5CBU2hWm9TesA4r/rOkI9DyJLvvgz09MB9YtfIYe3iBriVaYNaPe+Alua0vBIOVOLs2buWhg==",
+      "dev": true,
+      "dependencies": {
+        "@babel/types": "^7.10.4"
+      }
+    },
+    "node_modules/@babel/helper-validator-identifier": {
+      "version": "7.10.4",
+      "resolved": "https://registry.npmjs.org/@babel/helper-validator-identifier/-/helper-validator-identifier-7.10.4.tgz",
+      "integrity": "sha512-3U9y+43hz7ZM+rzG24Qe2mufW5KhvFg/NhnNph+i9mgCtdTCtMJuI1TMkrIUiK7Ix4PYlRF9I5dhqaLYA/ADXw==",
+      "dev": true
+    },
+    "node_modules/@babel/helpers": {
+      "version": "7.10.4",
+      "resolved": "https://registry.npmjs.org/@babel/helpers/-/helpers-7.10.4.tgz",
+      "integrity": "sha512-L2gX/XeUONeEbI78dXSrJzGdz4GQ+ZTA/aazfUsFaWjSe95kiCuOZ5HsXvkiw3iwF+mFHSRUfJU8t6YavocdXA==",
+      "dev": true,
+      "dependencies": {
+        "@babel/template": "^7.10.4",
+        "@babel/traverse": "^7.10.4",
+        "@babel/types": "^7.10.4"
+      }
+    },
+    "node_modules/@babel/highlight": {
+      "version": "7.10.4",
+      "resolved": "https://registry.npmjs.org/@babel/highlight/-/highlight-7.10.4.tgz",
+      "integrity": "sha512-i6rgnR/YgPEQzZZnbTHHuZdlE8qyoBNalD6F+q4vAFlcMEcqmkoG+mPqJYJCo63qPf74+Y1UZsl3l6f7/RIkmA==",
+      "dev": true,
+      "dependencies": {
+        "@babel/helper-validator-identifier": "^7.10.4",
+        "chalk": "^2.0.0",
+        "js-tokens": "^4.0.0"
+      }
+    },
+    "node_modules/@babel/parser": {
+      "version": "7.10.5",
+      "resolved": "https://registry.npmjs.org/@babel/parser/-/parser-7.10.5.tgz",
+      "integrity": "sha512-wfryxy4bE1UivvQKSQDU4/X6dr+i8bctjUjj8Zyt3DQy7NtPizJXT8M52nqpNKL+nq2PW8lxk4ZqLj0fD4B4hQ==",
+      "dev": true
+    },
+    "node_modules/@babel/runtime": {
+      "version": "7.10.5",
+      "resolved": "https://registry.npmjs.org/@babel/runtime/-/runtime-7.10.5.tgz",
+      "integrity": "sha512-otddXKhdNn7d0ptoFRHtMLa8LqDxLYwTjB4nYgM1yy5N6gU/MUf8zqyyLltCH3yAVitBzmwK4us+DD0l/MauAg==",
+      "dev": true,
+      "dependencies": {
+        "regenerator-runtime": "^0.13.4"
+      }
+    },
+    "node_modules/@babel/template": {
+      "version": "7.10.4",
+      "resolved": "https://registry.npmjs.org/@babel/template/-/template-7.10.4.tgz",
+      "integrity": "sha512-ZCjD27cGJFUB6nmCB1Enki3r+L5kJveX9pq1SvAUKoICy6CZ9yD8xO086YXdYhvNjBdnekm4ZnaP5yC8Cs/1tA==",
+      "dev": true,
+      "dependencies": {
+        "@babel/code-frame": "^7.10.4",
+        "@babel/parser": "^7.10.4",
+        "@babel/types": "^7.10.4"
+      }
+    },
+    "node_modules/@babel/traverse": {
+      "version": "7.10.5",
+      "resolved": "https://registry.npmjs.org/@babel/traverse/-/traverse-7.10.5.tgz",
+      "integrity": "sha512-yc/fyv2gUjPqzTz0WHeRJH2pv7jA9kA7mBX2tXl/x5iOE81uaVPuGPtaYk7wmkx4b67mQ7NqI8rmT2pF47KYKQ==",
+      "dev": true,
+      "dependencies": {
+        "@babel/code-frame": "^7.10.4",
+        "@babel/generator": "^7.10.5",
+        "@babel/helper-function-name": "^7.10.4",
+        "@babel/helper-split-export-declaration": "^7.10.4",
+        "@babel/parser": "^7.10.5",
+        "@babel/types": "^7.10.5",
+        "debug": "^4.1.0",
+        "globals": "^11.1.0",
+        "lodash": "^4.17.19"
+      }
+    },
+    "node_modules/@babel/types": {
+      "version": "7.10.5",
+      "resolved": "https://registry.npmjs.org/@babel/types/-/types-7.10.5.tgz",
+      "integrity": "sha512-ixV66KWfCI6GKoA/2H9v6bQdbfXEwwpOdQ8cRvb4F+eyvhlaHxWFMQB4+3d9QFJXZsiiiqVrewNV0DFEQpyT4Q==",
+      "dev": true,
+      "dependencies": {
+        "@babel/helper-validator-identifier": "^7.10.4",
+        "lodash": "^4.17.19",
+        "to-fast-properties": "^2.0.0"
+      }
+    },
+    "node_modules/@eslint/eslintrc": {
+      "version": "0.1.3",
+      "resolved": "https://registry.npmjs.org/@eslint/eslintrc/-/eslintrc-0.1.3.tgz",
+      "integrity": "sha512-4YVwPkANLeNtRjMekzux1ci8hIaH5eGKktGqR0d3LWsKNn5B2X/1Z6Trxy7jQXl9EBGE6Yj02O+t09FMeRllaA==",
+      "dev": true,
+      "dependencies": {
+        "ajv": "^6.12.4",
+        "debug": "^4.1.1",
+        "espree": "^7.3.0",
+        "globals": "^12.1.0",
+        "ignore": "^4.0.6",
+        "import-fresh": "^3.2.1",
+        "js-yaml": "^3.13.1",
+        "lodash": "^4.17.19",
+        "minimatch": "^3.0.4",
+        "strip-json-comments": "^3.1.1"
+      }
+    },
+    "node_modules/@eslint/eslintrc/node_modules/globals": {
+      "version": "12.4.0",
+      "resolved": "https://registry.npmjs.org/globals/-/globals-12.4.0.tgz",
+      "integrity": "sha512-BWICuzzDvDoH54NHKCseDanAhE3CeDorgDL5MT6LMXXj2WCnd9UC2szdk4AWLfjdgNBCXLUanXYcpBBKOSWGwg==",
+      "dev": true,
+      "dependencies": {
+        "type-fest": "^0.8.1"
+      }
+    },
+    "node_modules/@nodelib/fs.scandir": {
+      "version": "2.1.3",
+      "resolved": "https://registry.npmjs.org/@nodelib/fs.scandir/-/fs.scandir-2.1.3.tgz",
+      "integrity": "sha512-eGmwYQn3gxo4r7jdQnkrrN6bY478C3P+a/y72IJukF8LjB6ZHeB3c+Ehacj3sYeSmUXGlnA67/PmbM9CVwL7Dw==",
+      "dev": true,
+      "dependencies": {
+        "@nodelib/fs.stat": "2.0.3",
+        "run-parallel": "^1.1.9"
+      },
+      "engines": {
+        "node": ">= 8"
+      }
+    },
+    "node_modules/@nodelib/fs.stat": {
+      "version": "2.0.3",
+      "resolved": "https://registry.npmjs.org/@nodelib/fs.stat/-/fs.stat-2.0.3.tgz",
+      "integrity": "sha512-bQBFruR2TAwoevBEd/NWMoAAtNGzTRgdrqnYCc7dhzfoNvqPzLyqlEQnzZ3kVnNrSp25iyxE00/3h2fqGAGArA==",
+      "dev": true,
+      "engines": {
+        "node": ">= 8"
+      }
+    },
+    "node_modules/@nodelib/fs.walk": {
+      "version": "1.2.4",
+      "resolved": "https://registry.npmjs.org/@nodelib/fs.walk/-/fs.walk-1.2.4.tgz",
+      "integrity": "sha512-1V9XOY4rDW0rehzbrcqAmHnz8e7SKvX27gh8Gt2WgB0+pdzdiLV83p72kZPU+jvMbS1qU5mauP2iOvO8rhmurQ==",
+      "dev": true,
+      "dependencies": {
+        "@nodelib/fs.scandir": "2.1.3",
+        "fastq": "^1.6.0"
+      },
+      "engines": {
+        "node": ">= 8"
+      }
+    },
+    "node_modules/@types/color-name": {
+      "version": "1.1.1",
+      "resolved": "https://registry.npmjs.org/@types/color-name/-/color-name-1.1.1.tgz",
+      "integrity": "sha512-rr+OQyAjxze7GgWrSaJwydHStIhHq2lvY3BOC2Mj7KnzI7XK0Uw1TOOdI9lDoajEbSWLiYgoo4f1R51erQfhPQ==",
+      "dev": true
+    },
+    "node_modules/@types/json-schema": {
+      "version": "7.0.5",
+      "resolved": "https://registry.npmjs.org/@types/json-schema/-/json-schema-7.0.5.tgz",
+      "integrity": "sha512-7+2BITlgjgDhH0vvwZU/HZJVyk+2XUlvxXe8dFMedNX/aMkaOq++rMAFXc0tM7ij15QaWlbdQASBR9dihi+bDQ==",
+      "dev": true
+    },
+    "node_modules/@typescript-eslint/eslint-plugin": {
+      "version": "4.12.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/eslint-plugin/-/eslint-plugin-4.12.0.tgz",
+      "integrity": "sha512-wHKj6q8s70sO5i39H2g1gtpCXCvjVszzj6FFygneNFyIAxRvNSVz9GML7XpqrB9t7hNutXw+MHnLN/Ih6uyB8Q==",
+      "dev": true,
+      "dependencies": {
+        "@typescript-eslint/experimental-utils": "4.12.0",
+        "@typescript-eslint/scope-manager": "4.12.0",
+        "debug": "^4.1.1",
+        "functional-red-black-tree": "^1.0.1",
+        "regexpp": "^3.0.0",
+        "semver": "^7.3.2",
+        "tsutils": "^3.17.1"
+      },
+      "engines": {
+        "node": "^10.12.0 || >=12.0.0"
+      },
+      "funding": {
+        "type": "opencollective",
+        "url": "https://opencollective.com/typescript-eslint"
+      },
+      "peerDependencies": {
+        "@typescript-eslint/parser": "^4.0.0",
+        "eslint": "^5.0.0 || ^6.0.0 || ^7.0.0"
+      },
+      "peerDependenciesMeta": {
+        "typescript": {
+          "optional": true
+        }
+      }
+    },
+    "node_modules/@typescript-eslint/eslint-plugin/node_modules/@typescript-eslint/experimental-utils": {
+      "version": "4.12.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/experimental-utils/-/experimental-utils-4.12.0.tgz",
+      "integrity": "sha512-MpXZXUAvHt99c9ScXijx7i061o5HEjXltO+sbYfZAAHxv3XankQkPaNi5myy0Yh0Tyea3Hdq1pi7Vsh0GJb0fA==",
+      "dev": true,
+      "dependencies": {
+        "@types/json-schema": "^7.0.3",
+        "@typescript-eslint/scope-manager": "4.12.0",
+        "@typescript-eslint/types": "4.12.0",
+        "@typescript-eslint/typescript-estree": "4.12.0",
+        "eslint-scope": "^5.0.0",
+        "eslint-utils": "^2.0.0"
+      },
+      "engines": {
+        "node": "^10.12.0 || >=12.0.0"
+      },
+      "funding": {
+        "type": "opencollective",
+        "url": "https://opencollective.com/typescript-eslint"
+      },
+      "peerDependencies": {
+        "eslint": "*"
+      }
+    },
+    "node_modules/@typescript-eslint/eslint-plugin/node_modules/@typescript-eslint/scope-manager": {
+      "version": "4.12.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/scope-manager/-/scope-manager-4.12.0.tgz",
+      "integrity": "sha512-QVf9oCSVLte/8jvOsxmgBdOaoe2J0wtEmBr13Yz0rkBNkl5D8bfnf6G4Vhox9qqMIoG7QQoVwd2eG9DM/ge4Qg==",
+      "dev": true,
+      "dependencies": {
+        "@typescript-eslint/types": "4.12.0",
+        "@typescript-eslint/visitor-keys": "4.12.0"
+      },
+      "engines": {
+        "node": "^8.10.0 || ^10.13.0 || >=11.10.1"
+      },
+      "funding": {
+        "type": "opencollective",
+        "url": "https://opencollective.com/typescript-eslint"
+      }
+    },
+    "node_modules/@typescript-eslint/eslint-plugin/node_modules/@typescript-eslint/types": {
+      "version": "4.12.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/types/-/types-4.12.0.tgz",
+      "integrity": "sha512-N2RhGeheVLGtyy+CxRmxdsniB7sMSCfsnbh8K/+RUIXYYq3Ub5+sukRCjVE80QerrUBvuEvs4fDhz5AW/pcL6g==",
+      "dev": true,
+      "engines": {
+        "node": "^8.10.0 || ^10.13.0 || >=11.10.1"
+      },
+      "funding": {
+        "type": "opencollective",
+        "url": "https://opencollective.com/typescript-eslint"
+      }
+    },
+    "node_modules/@typescript-eslint/eslint-plugin/node_modules/@typescript-eslint/typescript-estree": {
+      "version": "4.12.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/typescript-estree/-/typescript-estree-4.12.0.tgz",
+      "integrity": "sha512-gZkFcmmp/CnzqD2RKMich2/FjBTsYopjiwJCroxqHZIY11IIoN0l5lKqcgoAPKHt33H2mAkSfvzj8i44Jm7F4w==",
+      "dev": true,
+      "dependencies": {
+        "@typescript-eslint/types": "4.12.0",
+        "@typescript-eslint/visitor-keys": "4.12.0",
+        "debug": "^4.1.1",
+        "globby": "^11.0.1",
+        "is-glob": "^4.0.1",
+        "lodash": "^4.17.15",
+        "semver": "^7.3.2",
+        "tsutils": "^3.17.1"
+      },
+      "engines": {
+        "node": "^10.12.0 || >=12.0.0"
+      },
+      "funding": {
+        "type": "opencollective",
+        "url": "https://opencollective.com/typescript-eslint"
+      },
+      "peerDependenciesMeta": {
+        "typescript": {
+          "optional": true
+        }
+      }
+    },
+    "node_modules/@typescript-eslint/eslint-plugin/node_modules/@typescript-eslint/visitor-keys": {
+      "version": "4.12.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/visitor-keys/-/visitor-keys-4.12.0.tgz",
+      "integrity": "sha512-hVpsLARbDh4B9TKYz5cLbcdMIOAoBYgFPCSP9FFS/liSF+b33gVNq8JHY3QGhHNVz85hObvL7BEYLlgx553WCw==",
+      "dev": true,
+      "dependencies": {
+        "@typescript-eslint/types": "4.12.0",
+        "eslint-visitor-keys": "^2.0.0"
+      },
+      "engines": {
+        "node": "^8.10.0 || ^10.13.0 || >=11.10.1"
+      },
+      "funding": {
+        "type": "opencollective",
+        "url": "https://opencollective.com/typescript-eslint"
+      }
+    },
+    "node_modules/@typescript-eslint/eslint-plugin/node_modules/eslint-visitor-keys": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/eslint-visitor-keys/-/eslint-visitor-keys-2.0.0.tgz",
+      "integrity": "sha512-QudtT6av5WXels9WjIM7qz1XD1cWGvX4gGXvp/zBn9nXG02D0utdU3Em2m/QjTnrsk6bBjmCygl3rmj118msQQ==",
+      "dev": true,
+      "engines": {
+        "node": ">=10"
+      }
+    },
+    "node_modules/@typescript-eslint/eslint-plugin/node_modules/semver": {
+      "version": "7.3.4",
+      "resolved": "https://registry.npmjs.org/semver/-/semver-7.3.4.tgz",
+      "integrity": "sha512-tCfb2WLjqFAtXn4KEdxIhalnRtoKFN7nAwj0B3ZXCbQloV2tq5eDbcTmT68JJD3nRJq24/XgxtQKFIpQdtvmVw==",
+      "dev": true,
+      "dependencies": {
+        "lru-cache": "^6.0.0"
+      },
+      "bin": {
+        "semver": "bin/semver.js"
+      },
+      "engines": {
+        "node": ">=10"
+      }
+    },
+    "node_modules/@typescript-eslint/experimental-utils": {
+      "version": "2.34.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/experimental-utils/-/experimental-utils-2.34.0.tgz",
+      "integrity": "sha512-eS6FTkq+wuMJ+sgtuNTtcqavWXqsflWcfBnlYhg/nS4aZ1leewkXGbvBhaapn1q6qf4M71bsR1tez5JTRMuqwA==",
+      "dev": true,
+      "dependencies": {
+        "@types/json-schema": "^7.0.3",
+        "@typescript-eslint/typescript-estree": "2.34.0",
+        "eslint-scope": "^5.0.0",
+        "eslint-utils": "^2.0.0"
+      }
+    },
+    "node_modules/@typescript-eslint/parser": {
+      "version": "4.10.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/parser/-/parser-4.10.0.tgz",
+      "integrity": "sha512-amBvUUGBMadzCW6c/qaZmfr3t9PyevcSWw7hY2FuevdZVp5QPw/K76VSQ5Sw3BxlgYCHZcK6DjIhSZK0PQNsQg==",
+      "dev": true,
+      "dependencies": {
+        "@typescript-eslint/scope-manager": "4.10.0",
+        "@typescript-eslint/types": "4.10.0",
+        "@typescript-eslint/typescript-estree": "4.10.0",
+        "debug": "^4.1.1"
+      },
+      "engines": {
+        "node": "^10.12.0 || >=12.0.0"
+      },
+      "funding": {
+        "type": "opencollective",
+        "url": "https://opencollective.com/typescript-eslint"
+      },
+      "peerDependencies": {
+        "eslint": "^5.0.0 || ^6.0.0 || ^7.0.0"
+      },
+      "peerDependenciesMeta": {
+        "typescript": {
+          "optional": true
+        }
+      }
+    },
+    "node_modules/@typescript-eslint/parser/node_modules/@typescript-eslint/typescript-estree": {
+      "version": "4.10.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/typescript-estree/-/typescript-estree-4.10.0.tgz",
+      "integrity": "sha512-mGK0YRp9TOk6ZqZ98F++bW6X5kMTzCRROJkGXH62d2azhghmq+1LNLylkGe6uGUOQzD452NOAEth5VAF6PDo5g==",
+      "dev": true,
+      "dependencies": {
+        "@typescript-eslint/types": "4.10.0",
+        "@typescript-eslint/visitor-keys": "4.10.0",
+        "debug": "^4.1.1",
+        "globby": "^11.0.1",
+        "is-glob": "^4.0.1",
+        "lodash": "^4.17.15",
+        "semver": "^7.3.2",
+        "tsutils": "^3.17.1"
+      },
+      "engines": {
+        "node": "^10.12.0 || >=12.0.0"
+      },
+      "funding": {
+        "type": "opencollective",
+        "url": "https://opencollective.com/typescript-eslint"
+      },
+      "peerDependenciesMeta": {
+        "typescript": {
+          "optional": true
+        }
+      }
+    },
+    "node_modules/@typescript-eslint/parser/node_modules/semver": {
+      "version": "7.3.4",
+      "resolved": "https://registry.npmjs.org/semver/-/semver-7.3.4.tgz",
+      "integrity": "sha512-tCfb2WLjqFAtXn4KEdxIhalnRtoKFN7nAwj0B3ZXCbQloV2tq5eDbcTmT68JJD3nRJq24/XgxtQKFIpQdtvmVw==",
+      "dev": true,
+      "dependencies": {
+        "lru-cache": "^6.0.0"
+      },
+      "bin": {
+        "semver": "bin/semver.js"
+      },
+      "engines": {
+        "node": ">=10"
+      }
+    },
+    "node_modules/@typescript-eslint/scope-manager": {
+      "version": "4.10.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/scope-manager/-/scope-manager-4.10.0.tgz",
+      "integrity": "sha512-WAPVw35P+fcnOa8DEic0tQUhoJJsgt+g6DEcz257G7vHFMwmag58EfowdVbiNcdfcV27EFR0tUBVXkDoIvfisQ==",
+      "dev": true,
+      "dependencies": {
+        "@typescript-eslint/types": "4.10.0",
+        "@typescript-eslint/visitor-keys": "4.10.0"
+      },
+      "engines": {
+        "node": "^8.10.0 || ^10.13.0 || >=11.10.1"
+      },
+      "funding": {
+        "type": "opencollective",
+        "url": "https://opencollective.com/typescript-eslint"
+      }
+    },
+    "node_modules/@typescript-eslint/types": {
+      "version": "4.10.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/types/-/types-4.10.0.tgz",
+      "integrity": "sha512-+dt5w1+Lqyd7wIPMa4XhJxUuE8+YF+vxQ6zxHyhLGHJjHiunPf0wSV8LtQwkpmAsRi1lEOoOIR30FG5S2HS33g==",
+      "dev": true,
+      "engines": {
+        "node": "^8.10.0 || ^10.13.0 || >=11.10.1"
+      },
+      "funding": {
+        "type": "opencollective",
+        "url": "https://opencollective.com/typescript-eslint"
+      }
+    },
+    "node_modules/@typescript-eslint/typescript-estree": {
+      "version": "2.34.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/typescript-estree/-/typescript-estree-2.34.0.tgz",
+      "integrity": "sha512-OMAr+nJWKdlVM9LOqCqh3pQQPwxHAN7Du8DR6dmwCrAmxtiXQnhHJ6tBNtf+cggqfo51SG/FCwnKhXCIM7hnVg==",
+      "dev": true,
+      "dependencies": {
+        "debug": "^4.1.1",
+        "eslint-visitor-keys": "^1.1.0",
+        "glob": "^7.1.6",
+        "is-glob": "^4.0.1",
+        "lodash": "^4.17.15",
+        "semver": "^7.3.2",
+        "tsutils": "^3.17.1"
+      }
+    },
+    "node_modules/@typescript-eslint/typescript-estree/node_modules/semver": {
+      "version": "7.3.2",
+      "resolved": "https://registry.npmjs.org/semver/-/semver-7.3.2.tgz",
+      "integrity": "sha512-OrOb32TeeambH6UrhtShmF7CRDqhL6/5XpPNp2DuRH6+9QLw/orhp72j87v8Qa1ScDkvrrBNpZcDejAirJmfXQ==",
+      "dev": true
+    },
+    "node_modules/@typescript-eslint/visitor-keys": {
+      "version": "4.10.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/visitor-keys/-/visitor-keys-4.10.0.tgz",
+      "integrity": "sha512-hPyz5qmDMuZWFtHZkjcCpkAKHX8vdu1G3YsCLEd25ryZgnJfj6FQuJ5/O7R+dB1ueszilJmAFMtlU4CA6se3Jg==",
+      "dev": true,
+      "dependencies": {
+        "@typescript-eslint/types": "4.10.0",
+        "eslint-visitor-keys": "^2.0.0"
+      },
+      "engines": {
+        "node": "^8.10.0 || ^10.13.0 || >=11.10.1"
+      },
+      "funding": {
+        "type": "opencollective",
+        "url": "https://opencollective.com/typescript-eslint"
+      }
+    },
+    "node_modules/@typescript-eslint/visitor-keys/node_modules/eslint-visitor-keys": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/eslint-visitor-keys/-/eslint-visitor-keys-2.0.0.tgz",
+      "integrity": "sha512-QudtT6av5WXels9WjIM7qz1XD1cWGvX4gGXvp/zBn9nXG02D0utdU3Em2m/QjTnrsk6bBjmCygl3rmj118msQQ==",
+      "dev": true,
+      "engines": {
+        "node": ">=10"
+      }
+    },
+    "node_modules/@ungap/promise-all-settled": {
+      "version": "1.1.2",
+      "resolved": "https://registry.npmjs.org/@ungap/promise-all-settled/-/promise-all-settled-1.1.2.tgz",
+      "integrity": "sha512-sL/cEvJWAnClXw0wHk85/2L0G6Sj8UB0Ctc1TEMbKSsmpRosqhwj9gWgFRZSrBr2f9tiXISwNhCPmlfqUqyb9Q==",
+      "dev": true
+    },
+    "node_modules/acorn": {
+      "version": "7.4.0",
+      "resolved": "https://registry.npmjs.org/acorn/-/acorn-7.4.0.tgz",
+      "integrity": "sha512-+G7P8jJmCHr+S+cLfQxygbWhXy+8YTVGzAkpEbcLo2mLoL7tij/VG41QSHACSf5QgYRhMZYHuNc6drJaO0Da+w==",
+      "dev": true
+    },
+    "node_modules/acorn-jsx": {
+      "version": "5.3.1",
+      "resolved": "https://registry.npmjs.org/acorn-jsx/-/acorn-jsx-5.3.1.tgz",
+      "integrity": "sha512-K0Ptm/47OKfQRpNQ2J/oIN/3QYiK6FwW+eJbILhsdxh2WTLdl+30o8aGdTbm5JbffpFFAg/g+zi1E+jvJha5ng==",
+      "dev": true
+    },
+    "node_modules/ajv": {
+      "version": "6.12.5",
+      "resolved": "https://registry.npmjs.org/ajv/-/ajv-6.12.5.tgz",
+      "integrity": "sha512-lRF8RORchjpKG50/WFf8xmg7sgCLFiYNNnqdKflk63whMQcWR5ngGjiSXkL9bjxy6B2npOK2HSMN49jEBMSkag==",
+      "dev": true,
+      "dependencies": {
+        "fast-deep-equal": "^3.1.1",
+        "fast-json-stable-stringify": "^2.0.0",
+        "json-schema-traverse": "^0.4.1",
+        "uri-js": "^4.2.2"
+      }
+    },
+    "node_modules/ansi-colors": {
+      "version": "4.1.1",
+      "resolved": "https://registry.npmjs.org/ansi-colors/-/ansi-colors-4.1.1.tgz",
+      "integrity": "sha512-JoX0apGbHaUJBNl6yF+p6JAFYZ666/hhCGKN5t9QFjbJQKUU/g8MNbFDbvfrgKXvI1QpZplPOnwIo99lX/AAmA==",
+      "dev": true
+    },
+    "node_modules/ansi-regex": {
+      "version": "5.0.0",
+      "resolved": "https://registry.npmjs.org/ansi-regex/-/ansi-regex-5.0.0.tgz",
+      "integrity": "sha512-bY6fj56OUQ0hU1KjFNDQuJFezqKdrAyFdIevADiqrWHwSlbmBNMHp5ak2f40Pm8JTFyM2mqxkG6ngkHO11f/lg==",
+      "dev": true
+    },
+    "node_modules/ansi-styles": {
+      "version": "3.2.1",
+      "resolved": "https://registry.npmjs.org/ansi-styles/-/ansi-styles-3.2.1.tgz",
+      "integrity": "sha512-VT0ZI6kZRdTh8YyJw3SMbYm/u+NqfsAxEpWO0Pf9sq8/e94WxxOpPKx9FR1FlyCtOVDNOQ+8ntlqFxiRc+r5qA==",
+      "dev": true,
+      "dependencies": {
+        "color-convert": "^1.9.0"
+      }
+    },
+    "node_modules/anymatch": {
+      "version": "3.1.1",
+      "resolved": "https://registry.npmjs.org/anymatch/-/anymatch-3.1.1.tgz",
+      "integrity": "sha512-mM8522psRCqzV+6LhomX5wgp25YVibjh8Wj23I5RPkPppSVSjyKD2A2mBJmWGa+KN7f2D6LNh9jkBCeyLktzjg==",
+      "dev": true,
+      "dependencies": {
+        "normalize-path": "^3.0.0",
+        "picomatch": "^2.0.4"
+      },
+      "engines": {
+        "node": ">= 8"
+      }
+    },
+    "node_modules/argparse": {
+      "version": "1.0.10",
+      "resolved": "https://registry.npmjs.org/argparse/-/argparse-1.0.10.tgz",
+      "integrity": "sha512-o5Roy6tNG4SL/FOkCAN6RzjiakZS25RLYFrcMttJqbdd8BWrnA+fGz57iN5Pb06pvBGvl5gQ0B48dJlslXvoTg==",
+      "dev": true,
+      "dependencies": {
+        "sprintf-js": "~1.0.2"
+      }
+    },
+    "node_modules/aria-query": {
+      "version": "3.0.0",
+      "resolved": "https://registry.npmjs.org/aria-query/-/aria-query-3.0.0.tgz",
+      "integrity": "sha1-ZbP8wcoRVajJrmTW7uKX8V1RM8w=",
+      "dev": true,
+      "dependencies": {
+        "ast-types-flow": "0.0.7",
+        "commander": "^2.11.0"
+      }
+    },
+    "node_modules/array-includes": {
+      "version": "3.1.1",
+      "resolved": "https://registry.npmjs.org/array-includes/-/array-includes-3.1.1.tgz",
+      "integrity": "sha512-c2VXaCHl7zPsvpkFsw4nxvFie4fh1ur9bpcgsVkIjqn0H/Xwdg+7fv3n2r/isyS8EBj5b06M9kHyZuIr4El6WQ==",
+      "dev": true,
+      "dependencies": {
+        "define-properties": "^1.1.3",
+        "es-abstract": "^1.17.0",
+        "is-string": "^1.0.5"
+      }
+    },
+    "node_modules/array-union": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/array-union/-/array-union-2.1.0.tgz",
+      "integrity": "sha512-HGyxoOTYUyCM6stUe6EJgnd4EoewAI7zMdfqO+kGjnlZmBDz/cR5pf8r/cR4Wq60sL/p0IkcjUEEPwS3GFrIyw==",
+      "dev": true,
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/array.prototype.flat": {
+      "version": "1.2.3",
+      "resolved": "https://registry.npmjs.org/array.prototype.flat/-/array.prototype.flat-1.2.3.tgz",
+      "integrity": "sha512-gBlRZV0VSmfPIeWfuuy56XZMvbVfbEUnOXUvt3F/eUUUSyzlgLxhEX4YAEpxNAogRGehPSnfXyPtYyKAhkzQhQ==",
+      "dev": true,
+      "dependencies": {
+        "define-properties": "^1.1.3",
+        "es-abstract": "^1.17.0-next.1"
+      }
+    },
+    "node_modules/ast-types-flow": {
+      "version": "0.0.7",
+      "resolved": "https://registry.npmjs.org/ast-types-flow/-/ast-types-flow-0.0.7.tgz",
+      "integrity": "sha1-9wtzXGvKGlycItmCw+Oef+ujva0=",
+      "dev": true
+    },
+    "node_modules/astral-regex": {
+      "version": "1.0.0",
+      "resolved": "https://registry.npmjs.org/astral-regex/-/astral-regex-1.0.0.tgz",
+      "integrity": "sha512-+Ryf6g3BKoRc7jfp7ad8tM4TtMiaWvbF/1/sQcZPkkS7ag3D5nMBCe2UfOTONtAkaG0tO0ij3C5Lwmf1EiyjHg==",
+      "dev": true
+    },
+    "node_modules/axobject-query": {
+      "version": "2.2.0",
+      "resolved": "https://registry.npmjs.org/axobject-query/-/axobject-query-2.2.0.tgz",
+      "integrity": "sha512-Td525n+iPOOyUQIeBfcASuG6uJsDOITl7Mds5gFyerkWiX7qhUTdYUBlSgNMyVqtSJqwpt1kXGLdUt6SykLMRA==",
+      "dev": true
+    },
+    "node_modules/babel-eslint": {
+      "version": "10.1.0",
+      "resolved": "https://registry.npmjs.org/babel-eslint/-/babel-eslint-10.1.0.tgz",
+      "integrity": "sha512-ifWaTHQ0ce+448CYop8AdrQiBsGrnC+bMgfyKFdi6EsPLTAWG+QfyDeM6OH+FmWnKvEq5NnBMLvlBUPKQZoDSg==",
+      "dev": true,
+      "dependencies": {
+        "@babel/code-frame": "^7.0.0",
+        "@babel/parser": "^7.7.0",
+        "@babel/traverse": "^7.7.0",
+        "@babel/types": "^7.7.0",
+        "eslint-visitor-keys": "^1.0.0",
+        "resolve": "^1.12.0"
+      }
+    },
+    "node_modules/balanced-match": {
+      "version": "1.0.0",
+      "resolved": "https://registry.npmjs.org/balanced-match/-/balanced-match-1.0.0.tgz",
+      "integrity": "sha1-ibTRmasr7kneFk6gK4nORi1xt2c=",
+      "dev": true
+    },
+    "node_modules/binary-extensions": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/binary-extensions/-/binary-extensions-2.1.0.tgz",
+      "integrity": "sha512-1Yj8h9Q+QDF5FzhMs/c9+6UntbD5MkRfRwac8DoEm9ZfUBZ7tZ55YcGVAzEe4bXsdQHEk+s9S5wsOKVdZrw0tQ==",
+      "dev": true,
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/brace-expansion": {
+      "version": "1.1.11",
+      "resolved": "https://registry.npmjs.org/brace-expansion/-/brace-expansion-1.1.11.tgz",
+      "integrity": "sha512-iCuPHDFgrHX7H2vEI/5xpz07zSHB00TpugqhmYtVmMO6518mCuRMoOYFldEBl0g187ufozdaHgWKcYFb61qGiA==",
+      "dev": true,
+      "dependencies": {
+        "balanced-match": "^1.0.0",
+        "concat-map": "0.0.1"
+      }
+    },
+    "node_modules/braces": {
+      "version": "3.0.2",
+      "resolved": "https://registry.npmjs.org/braces/-/braces-3.0.2.tgz",
+      "integrity": "sha512-b8um+L1RzM3WDSzvhm6gIz1yfTbBt6YTlcEKAvsmqCZZFw46z626lVj9j1yEPW33H5H+lBQpZMP1k8l+78Ha0A==",
+      "dev": true,
+      "dependencies": {
+        "fill-range": "^7.0.1"
+      },
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/browser-stdout": {
+      "version": "1.3.1",
+      "resolved": "https://registry.npmjs.org/browser-stdout/-/browser-stdout-1.3.1.tgz",
+      "integrity": "sha512-qhAVI1+Av2X7qelOfAIYwXONood6XlZE/fXaBSmW/T5SzLAmCgzi+eiWE7fUvbHaeNBQH13UftjpXxsfLkMpgw==",
+      "dev": true
+    },
+    "node_modules/callsites": {
+      "version": "3.1.0",
+      "resolved": "https://registry.npmjs.org/callsites/-/callsites-3.1.0.tgz",
+      "integrity": "sha512-P8BjAsXvZS+VIDUI11hHCQEv74YT67YUi5JJFNWIqL235sBmjX4+qx9Muvls5ivyNENctx46xQLQ3aTuE7ssaQ==",
+      "dev": true
+    },
+    "node_modules/camelcase": {
+      "version": "5.3.1",
+      "resolved": "https://registry.npmjs.org/camelcase/-/camelcase-5.3.1.tgz",
+      "integrity": "sha512-L28STB170nwWS63UjtlEOE3dldQApaJXZkOI1uMFfzf3rRuPegHaHesyee+YxQ+W6SvRDQV6UrdOdRiR153wJg==",
+      "dev": true,
+      "engines": {
+        "node": ">=6"
+      }
+    },
+    "node_modules/chalk": {
+      "version": "2.4.2",
+      "resolved": "https://registry.npmjs.org/chalk/-/chalk-2.4.2.tgz",
+      "integrity": "sha512-Mti+f9lpJNcwF4tWV8/OrTTtF1gZi+f8FqlyAdouralcFWFQWF2+NgCHShjkCb+IFBLq9buZwE1xckQU4peSuQ==",
+      "dev": true,
+      "dependencies": {
+        "ansi-styles": "^3.2.1",
+        "escape-string-regexp": "^1.0.5",
+        "supports-color": "^5.3.0"
+      }
+    },
+    "node_modules/chokidar": {
+      "version": "3.4.3",
+      "resolved": "https://registry.npmjs.org/chokidar/-/chokidar-3.4.3.tgz",
+      "integrity": "sha512-DtM3g7juCXQxFVSNPNByEC2+NImtBuxQQvWlHunpJIS5Ocr0lG306cC7FCi7cEA0fzmybPUIl4txBIobk1gGOQ==",
+      "dev": true,
+      "dependencies": {
+        "anymatch": "~3.1.1",
+        "braces": "~3.0.2",
+        "fsevents": "~2.1.2",
+        "glob-parent": "~5.1.0",
+        "is-binary-path": "~2.1.0",
+        "is-glob": "~4.0.1",
+        "normalize-path": "~3.0.0",
+        "readdirp": "~3.5.0"
+      },
+      "engines": {
+        "node": ">= 8.10.0"
+      },
+      "optionalDependencies": {
+        "fsevents": "~2.1.2"
+      }
+    },
+    "node_modules/cliui": {
+      "version": "5.0.0",
+      "resolved": "https://registry.npmjs.org/cliui/-/cliui-5.0.0.tgz",
+      "integrity": "sha512-PYeGSEmmHM6zvoef2w8TPzlrnNpXIjTipYK780YswmIP9vjxmd6Y2a3CB2Ks6/AU8NHjZugXvo8w3oWM2qnwXA==",
+      "dev": true,
+      "dependencies": {
+        "string-width": "^3.1.0",
+        "strip-ansi": "^5.2.0",
+        "wrap-ansi": "^5.1.0"
+      }
+    },
+    "node_modules/cliui/node_modules/ansi-regex": {
+      "version": "4.1.0",
+      "resolved": "https://registry.npmjs.org/ansi-regex/-/ansi-regex-4.1.0.tgz",
+      "integrity": "sha512-1apePfXM1UOSqw0o9IiFAovVz9M5S1Dg+4TrDwfMewQ6p/rmMueb7tWZjQ1rx4Loy1ArBggoqGpfqqdI4rondg==",
+      "dev": true,
+      "engines": {
+        "node": ">=6"
+      }
+    },
+    "node_modules/cliui/node_modules/strip-ansi": {
+      "version": "5.2.0",
+      "resolved": "https://registry.npmjs.org/strip-ansi/-/strip-ansi-5.2.0.tgz",
+      "integrity": "sha512-DuRs1gKbBqsMKIZlrffwlug8MHkcnpjs5VPmL1PAh+mA30U0DTotfDZ0d2UUsXpPmPmMMJ6W773MaA3J+lbiWA==",
+      "dev": true,
+      "dependencies": {
+        "ansi-regex": "^4.1.0"
+      },
+      "engines": {
+        "node": ">=6"
+      }
+    },
+    "node_modules/color-convert": {
+      "version": "1.9.3",
+      "resolved": "https://registry.npmjs.org/color-convert/-/color-convert-1.9.3.tgz",
+      "integrity": "sha512-QfAUtd+vFdAtFQcC8CCyYt1fYWxSqAiK2cSD6zDB8N3cpsEBAvRxp9zOGg6G/SHHJYAT88/az/IuDGALsNVbGg==",
+      "dev": true,
+      "dependencies": {
+        "color-name": "1.1.3"
+      }
+    },
+    "node_modules/color-name": {
+      "version": "1.1.3",
+      "resolved": "https://registry.npmjs.org/color-name/-/color-name-1.1.3.tgz",
+      "integrity": "sha1-p9BVi9icQveV3UIyj3QIMcpTvCU=",
+      "dev": true
+    },
+    "node_modules/commander": {
+      "version": "2.20.3",
+      "resolved": "https://registry.npmjs.org/commander/-/commander-2.20.3.tgz",
+      "integrity": "sha512-GpVkmM8vF2vQUkj2LvZmD35JxeJOLCwJ9cUkugyk2nuhbv3+mJvpLYYt+0+USMxE+oj+ey/lJEnhZw75x/OMcQ==",
+      "dev": true
+    },
+    "node_modules/concat-map": {
+      "version": "0.0.1",
+      "resolved": "https://registry.npmjs.org/concat-map/-/concat-map-0.0.1.tgz",
+      "integrity": "sha1-2Klr13/Wjfd5OnMDajug1UBdR3s=",
+      "dev": true
+    },
+    "node_modules/contains-path": {
+      "version": "0.1.0",
+      "resolved": "https://registry.npmjs.org/contains-path/-/contains-path-0.1.0.tgz",
+      "integrity": "sha1-/ozxhP9mcLa67wGp1IYaXL7EEgo=",
+      "dev": true
+    },
+    "node_modules/convert-source-map": {
+      "version": "1.7.0",
+      "resolved": "https://registry.npmjs.org/convert-source-map/-/convert-source-map-1.7.0.tgz",
+      "integrity": "sha512-4FJkXzKXEDB1snCFZlLP4gpC3JILicCpGbzG9f9G7tGqGCzETQ2hWPrcinA9oU4wtf2biUaEH5065UnMeR33oA==",
+      "dev": true,
+      "dependencies": {
+        "safe-buffer": "~5.1.1"
+      }
+    },
+    "node_modules/cross-spawn": {
+      "version": "7.0.3",
+      "resolved": "https://registry.npmjs.org/cross-spawn/-/cross-spawn-7.0.3.tgz",
+      "integrity": "sha512-iRDPJKUPVEND7dHPO8rkbOnPpyDygcDFtWjpeWNCgy8WP2rXcxXL8TskReQl6OrB2G7+UJrags1q15Fudc7G6w==",
+      "dev": true,
+      "dependencies": {
+        "path-key": "^3.1.0",
+        "shebang-command": "^2.0.0",
+        "which": "^2.0.1"
+      }
+    },
+    "node_modules/damerau-levenshtein": {
+      "version": "1.0.6",
+      "resolved": "https://registry.npmjs.org/damerau-levenshtein/-/damerau-levenshtein-1.0.6.tgz",
+      "integrity": "sha512-JVrozIeElnj3QzfUIt8tB8YMluBJom4Vw9qTPpjGYQ9fYlB3D/rb6OordUxf3xeFB35LKWs0xqcO5U6ySvBtug==",
+      "dev": true
+    },
+    "node_modules/debug": {
+      "version": "4.2.0",
+      "resolved": "https://registry.npmjs.org/debug/-/debug-4.2.0.tgz",
+      "integrity": "sha512-IX2ncY78vDTjZMFUdmsvIRFY2Cf4FnD0wRs+nQwJU8Lu99/tPFdb0VybiiMTPe3I6rQmwsqQqRBvxU+bZ/I8sg==",
+      "deprecated": "Debug versions >=3.2.0 <3.2.7 || >=4 <4.3.1 have a low-severity ReDos regression when used in a Node.js environment. It is recommended you upgrade to 3.2.7 or 4.3.1. (https://github.com/visionmedia/debug/issues/797)",
+      "dev": true,
+      "dependencies": {
+        "ms": "2.1.2"
+      },
+      "engines": {
+        "node": ">=6.0"
+      },
+      "peerDependenciesMeta": {
+        "supports-color": {
+          "optional": true
+        }
+      }
+    },
+    "node_modules/decamelize": {
+      "version": "1.2.0",
+      "resolved": "https://registry.npmjs.org/decamelize/-/decamelize-1.2.0.tgz",
+      "integrity": "sha1-9lNNFRSCabIDUue+4m9QH5oZEpA=",
+      "dev": true,
+      "engines": {
+        "node": ">=0.10.0"
+      }
+    },
+    "node_modules/deep-is": {
+      "version": "0.1.3",
+      "resolved": "https://registry.npmjs.org/deep-is/-/deep-is-0.1.3.tgz",
+      "integrity": "sha1-s2nW+128E+7PUk+RsHD+7cNXzzQ=",
+      "dev": true
+    },
+    "node_modules/define-properties": {
+      "version": "1.1.3",
+      "resolved": "https://registry.npmjs.org/define-properties/-/define-properties-1.1.3.tgz",
+      "integrity": "sha512-3MqfYKj2lLzdMSf8ZIZE/V+Zuy+BgD6f164e8K2w7dgnpKArBDerGYpM46IYYcjnkdPNMjPk9A6VFB8+3SKlXQ==",
+      "dev": true,
+      "dependencies": {
+        "object-keys": "^1.0.12"
+      }
+    },
+    "node_modules/diff": {
+      "version": "4.0.2",
+      "resolved": "https://registry.npmjs.org/diff/-/diff-4.0.2.tgz",
+      "integrity": "sha512-58lmxKSA4BNyLz+HHMUzlOEpg09FV+ev6ZMe3vJihgdxzgcwZ8VoEEPmALCZG9LmqfVoNMMKpttIYTVG6uDY7A==",
+      "dev": true,
+      "engines": {
+        "node": ">=0.3.1"
+      }
+    },
+    "node_modules/dir-glob": {
+      "version": "3.0.1",
+      "resolved": "https://registry.npmjs.org/dir-glob/-/dir-glob-3.0.1.tgz",
+      "integrity": "sha512-WkrWp9GR4KXfKGYzOLmTuGVi1UWFfws377n9cc55/tb6DuqyF6pcQ5AbiHEshaDpY9v6oaSr2XCDidGmMwdzIA==",
+      "dev": true,
+      "dependencies": {
+        "path-type": "^4.0.0"
+      },
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/dir-glob/node_modules/path-type": {
+      "version": "4.0.0",
+      "resolved": "https://registry.npmjs.org/path-type/-/path-type-4.0.0.tgz",
+      "integrity": "sha512-gDKb8aZMDeD/tZWs9P6+q0J9Mwkdl6xMV8TjnGP3qJVJ06bdMgkbBlLU8IdfOsIsFz2BW1rNVT3XuNEl8zPAvw==",
+      "dev": true,
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/doctrine": {
+      "version": "3.0.0",
+      "resolved": "https://registry.npmjs.org/doctrine/-/doctrine-3.0.0.tgz",
+      "integrity": "sha512-yS+Q5i3hBf7GBkd4KG8a7eBNNWNGLTaEwwYWUijIYM7zrlYDM0BFXHjjPWlWZ1Rg7UaddZeIDmi9jF3HmqiQ2w==",
+      "dev": true,
+      "dependencies": {
+        "esutils": "^2.0.2"
+      }
+    },
+    "node_modules/dom-serializer": {
+      "version": "0.2.2",
+      "resolved": "https://registry.npmjs.org/dom-serializer/-/dom-serializer-0.2.2.tgz",
+      "integrity": "sha512-2/xPb3ORsQ42nHYiSunXkDjPLBaEj/xTwUO4B7XCZQTRk7EBtTOPaygh10YAAh2OI1Qrp6NWfpAhzswj0ydt9g==",
+      "dev": true,
+      "dependencies": {
+        "domelementtype": "^2.0.1",
+        "entities": "^2.0.0"
+      }
+    },
+    "node_modules/dom-serializer/node_modules/domelementtype": {
+      "version": "2.0.1",
+      "resolved": "https://registry.npmjs.org/domelementtype/-/domelementtype-2.0.1.tgz",
+      "integrity": "sha512-5HOHUDsYZWV8FGWN0Njbr/Rn7f/eWSQi1v7+HsUVwXgn8nWWlL64zKDkS0n8ZmQ3mlWOMuXOnR+7Nx/5tMO5AQ==",
+      "dev": true
+    },
+    "node_modules/dom-serializer/node_modules/entities": {
+      "version": "2.0.3",
+      "resolved": "https://registry.npmjs.org/entities/-/entities-2.0.3.tgz",
+      "integrity": "sha512-MyoZ0jgnLvB2X3Lg5HqpFmn1kybDiIfEQmKzTb5apr51Rb+T3KdmMiqa70T+bhGnyv7bQ6WMj2QMHpGMmlrUYQ==",
+      "dev": true
+    },
+    "node_modules/domelementtype": {
+      "version": "1.3.1",
+      "resolved": "https://registry.npmjs.org/domelementtype/-/domelementtype-1.3.1.tgz",
+      "integrity": "sha512-BSKB+TSpMpFI/HOxCNr1O8aMOTZ8hT3pM3GQ0w/mWRmkhEDSFJkkyzz4XQsBV44BChwGkrDfMyjVD0eA2aFV3w==",
+      "dev": true
+    },
+    "node_modules/domhandler": {
+      "version": "2.4.2",
+      "resolved": "https://registry.npmjs.org/domhandler/-/domhandler-2.4.2.tgz",
+      "integrity": "sha512-JiK04h0Ht5u/80fdLMCEmV4zkNh2BcoMFBmZ/91WtYZ8qVXSKjiw7fXMgFPnHcSZgOo3XdinHvmnDUeMf5R4wA==",
+      "dev": true,
+      "dependencies": {
+        "domelementtype": "1"
+      }
+    },
+    "node_modules/domutils": {
+      "version": "1.7.0",
+      "resolved": "https://registry.npmjs.org/domutils/-/domutils-1.7.0.tgz",
+      "integrity": "sha512-Lgd2XcJ/NjEw+7tFvfKxOzCYKZsdct5lczQ2ZaQY8Djz7pfAD3Gbp8ySJWtreII/vDlMVmxwa6pHmdxIYgttDg==",
+      "dev": true,
+      "dependencies": {
+        "dom-serializer": "0",
+        "domelementtype": "1"
+      }
+    },
+    "node_modules/emoji-regex": {
+      "version": "7.0.3",
+      "resolved": "https://registry.npmjs.org/emoji-regex/-/emoji-regex-7.0.3.tgz",
+      "integrity": "sha512-CwBLREIQ7LvYFB0WyRvwhq5N5qPhc6PMjD6bYggFlI5YyDgl+0vxq5VHbMOFqLg7hfWzmu8T5Z1QofhmTIhItA==",
+      "dev": true
+    },
+    "node_modules/enquirer": {
+      "version": "2.3.6",
+      "resolved": "https://registry.npmjs.org/enquirer/-/enquirer-2.3.6.tgz",
+      "integrity": "sha512-yjNnPr315/FjS4zIsUxYguYUPP2e1NK4d7E7ZOLiyYCcbFBiTMyID+2wvm2w6+pZ/odMA7cRkjhsPbltwBOrLg==",
+      "dev": true,
+      "dependencies": {
+        "ansi-colors": "^4.1.1"
+      }
+    },
+    "node_modules/entities": {
+      "version": "1.1.2",
+      "resolved": "https://registry.npmjs.org/entities/-/entities-1.1.2.tgz",
+      "integrity": "sha512-f2LZMYl1Fzu7YSBKg+RoROelpOaNrcGmE9AZubeDfrCEia483oW4MI4VyFd5VNHIgQ/7qm1I0wUHK1eJnn2y2w==",
+      "dev": true
+    },
+    "node_modules/error-ex": {
+      "version": "1.3.2",
+      "resolved": "https://registry.npmjs.org/error-ex/-/error-ex-1.3.2.tgz",
+      "integrity": "sha512-7dFHNmqeFSEt2ZBsCriorKnn3Z2pj+fd9kmI6QoWw4//DL+icEBfc0U7qJCisqrTsKTjw4fNFy2pW9OqStD84g==",
+      "dev": true,
+      "dependencies": {
+        "is-arrayish": "^0.2.1"
+      }
+    },
+    "node_modules/es-abstract": {
+      "version": "1.17.6",
+      "resolved": "https://registry.npmjs.org/es-abstract/-/es-abstract-1.17.6.tgz",
+      "integrity": "sha512-Fr89bON3WFyUi5EvAeI48QTWX0AyekGgLA8H+c+7fbfCkJwRWRMLd8CQedNEyJuoYYhmtEqY92pgte1FAhBlhw==",
+      "dev": true,
+      "dependencies": {
+        "es-to-primitive": "^1.2.1",
+        "function-bind": "^1.1.1",
+        "has": "^1.0.3",
+        "has-symbols": "^1.0.1",
+        "is-callable": "^1.2.0",
+        "is-regex": "^1.1.0",
+        "object-inspect": "^1.7.0",
+        "object-keys": "^1.1.1",
+        "object.assign": "^4.1.0",
+        "string.prototype.trimend": "^1.0.1",
+        "string.prototype.trimstart": "^1.0.1"
+      }
+    },
+    "node_modules/es-to-primitive": {
+      "version": "1.2.1",
+      "resolved": "https://registry.npmjs.org/es-to-primitive/-/es-to-primitive-1.2.1.tgz",
+      "integrity": "sha512-QCOllgZJtaUo9miYBcLChTUaHNjJF3PYs1VidD7AwiEj1kYxKeQTctLAezAOH5ZKRH0g2IgPn6KwB4IT8iRpvA==",
+      "dev": true,
+      "dependencies": {
+        "is-callable": "^1.1.4",
+        "is-date-object": "^1.0.1",
+        "is-symbol": "^1.0.2"
+      }
+    },
+    "node_modules/escape-string-regexp": {
+      "version": "1.0.5",
+      "resolved": "https://registry.npmjs.org/escape-string-regexp/-/escape-string-regexp-1.0.5.tgz",
+      "integrity": "sha1-G2HAViGQqN/2rjuyzwIAyhMLhtQ=",
+      "dev": true
+    },
+    "node_modules/eslint": {
+      "version": "7.8.0",
+      "resolved": "https://registry.npmjs.org/eslint/-/eslint-7.8.0.tgz",
+      "integrity": "sha512-qgtVyLZqKd2ZXWnLQA4NtVbOyH56zivOAdBFWE54RFkSZjokzNrcP4Z0eVWsZ+84ByXv+jL9k/wE1ENYe8xRFw==",
+      "dev": true,
+      "dependencies": {
+        "@babel/code-frame": "^7.0.0",
+        "@eslint/eslintrc": "^0.1.0",
+        "ajv": "^6.10.0",
+        "chalk": "^4.0.0",
+        "cross-spawn": "^7.0.2",
+        "debug": "^4.0.1",
+        "doctrine": "^3.0.0",
+        "enquirer": "^2.3.5",
+        "eslint-scope": "^5.1.0",
+        "eslint-utils": "^2.1.0",
+        "eslint-visitor-keys": "^1.3.0",
+        "espree": "^7.3.0",
+        "esquery": "^1.2.0",
+        "esutils": "^2.0.2",
+        "file-entry-cache": "^5.0.1",
+        "functional-red-black-tree": "^1.0.1",
+        "glob-parent": "^5.0.0",
+        "globals": "^12.1.0",
+        "ignore": "^4.0.6",
+        "import-fresh": "^3.0.0",
+        "imurmurhash": "^0.1.4",
+        "is-glob": "^4.0.0",
+        "js-yaml": "^3.13.1",
+        "json-stable-stringify-without-jsonify": "^1.0.1",
+        "levn": "^0.4.1",
+        "lodash": "^4.17.19",
+        "minimatch": "^3.0.4",
+        "natural-compare": "^1.4.0",
+        "optionator": "^0.9.1",
+        "progress": "^2.0.0",
+        "regexpp": "^3.1.0",
+        "semver": "^7.2.1",
+        "strip-ansi": "^6.0.0",
+        "strip-json-comments": "^3.1.0",
+        "table": "^5.2.3",
+        "text-table": "^0.2.0",
+        "v8-compile-cache": "^2.0.3"
+      }
+    },
+    "node_modules/eslint-config-prettier": {
+      "version": "6.10.0",
+      "resolved": "https://registry.npmjs.org/eslint-config-prettier/-/eslint-config-prettier-6.10.0.tgz",
+      "integrity": "sha512-AtndijGte1rPILInUdHjvKEGbIV06NuvPrqlIEaEaWtbtvJh464mDeyGMdZEQMsGvC0ZVkiex1fSNcC4HAbRGg==",
+      "dev": true,
+      "dependencies": {
+        "get-stdin": "^6.0.0"
+      }
+    },
+    "node_modules/eslint-import-resolver-node": {
+      "version": "0.3.4",
+      "resolved": "https://registry.npmjs.org/eslint-import-resolver-node/-/eslint-import-resolver-node-0.3.4.tgz",
+      "integrity": "sha512-ogtf+5AB/O+nM6DIeBUNr2fuT7ot9Qg/1harBfBtaP13ekEWFQEEMP94BCB7zaNW3gyY+8SHYF00rnqYwXKWOA==",
+      "dev": true,
+      "dependencies": {
+        "debug": "^2.6.9",
+        "resolve": "^1.13.1"
+      }
+    },
+    "node_modules/eslint-import-resolver-node/node_modules/debug": {
+      "version": "2.6.9",
+      "resolved": "https://registry.npmjs.org/debug/-/debug-2.6.9.tgz",
+      "integrity": "sha512-bC7ElrdJaJnPbAP+1EotYvqZsb3ecl5wi6Bfi6BJTUcNowp6cvspg0jXznRTKDjm/E7AdgFBVeAPVMNcKGsHMA==",
+      "dev": true,
+      "dependencies": {
+        "ms": "2.0.0"
+      }
+    },
+    "node_modules/eslint-import-resolver-node/node_modules/ms": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/ms/-/ms-2.0.0.tgz",
+      "integrity": "sha1-VgiurfwAvmwpAd9fmGF4jeDVl8g=",
+      "dev": true
+    },
+    "node_modules/eslint-module-utils": {
+      "version": "2.6.0",
+      "resolved": "https://registry.npmjs.org/eslint-module-utils/-/eslint-module-utils-2.6.0.tgz",
+      "integrity": "sha512-6j9xxegbqe8/kZY8cYpcp0xhbK0EgJlg3g9mib3/miLaExuuwc3n5UEfSnU6hWMbT0FAYVvDbL9RrRgpUeQIvA==",
+      "dev": true,
+      "dependencies": {
+        "debug": "^2.6.9",
+        "pkg-dir": "^2.0.0"
+      }
+    },
+    "node_modules/eslint-module-utils/node_modules/debug": {
+      "version": "2.6.9",
+      "resolved": "https://registry.npmjs.org/debug/-/debug-2.6.9.tgz",
+      "integrity": "sha512-bC7ElrdJaJnPbAP+1EotYvqZsb3ecl5wi6Bfi6BJTUcNowp6cvspg0jXznRTKDjm/E7AdgFBVeAPVMNcKGsHMA==",
+      "dev": true,
+      "dependencies": {
+        "ms": "2.0.0"
+      }
+    },
+    "node_modules/eslint-module-utils/node_modules/ms": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/ms/-/ms-2.0.0.tgz",
+      "integrity": "sha1-VgiurfwAvmwpAd9fmGF4jeDVl8g=",
+      "dev": true
+    },
+    "node_modules/eslint-plugin-babel": {
+      "version": "5.3.0",
+      "resolved": "https://registry.npmjs.org/eslint-plugin-babel/-/eslint-plugin-babel-5.3.0.tgz",
+      "integrity": "sha512-HPuNzSPE75O+SnxHIafbW5QB45r2w78fxqwK3HmjqIUoPfPzVrq6rD+CINU3yzoDSzEhUkX07VUphbF73Lth/w==",
+      "dev": true,
+      "dependencies": {
+        "eslint-rule-composer": "^0.3.0"
+      }
+    },
+    "node_modules/eslint-plugin-fetch-options": {
+      "version": "0.0.5",
+      "resolved": "https://registry.npmjs.org/eslint-plugin-fetch-options/-/eslint-plugin-fetch-options-0.0.5.tgz",
+      "integrity": "sha512-ZMxrccsOAZ7uMQ4nMvPJLqLg6oyLF96YOEwTKWAIbDHpwWUp1raXALZom8ikKucaEnhqWSRuBWI8jBXveFwkJg==",
+      "dev": true
+    },
+    "node_modules/eslint-plugin-file-header": {
+      "version": "0.0.1",
+      "resolved": "https://registry.npmjs.org/eslint-plugin-file-header/-/eslint-plugin-file-header-0.0.1.tgz",
+      "integrity": "sha512-Xe7veqG+8s99Msd/bFm6YDBnKaufgd/oE+uOXQqpadLGZSrb3t+iW5n7c2rcBfgZ9oGjhuXIL3IsH3a+U8oVNQ==",
+      "dev": true
+    },
+    "node_modules/eslint-plugin-flowtype": {
+      "version": "4.6.0",
+      "resolved": "https://registry.npmjs.org/eslint-plugin-flowtype/-/eslint-plugin-flowtype-4.6.0.tgz",
+      "integrity": "sha512-W5hLjpFfZyZsXfo5anlu7HM970JBDqbEshAJUkeczP6BFCIfJXuiIBQXyberLRtOStT0OGPF8efeTbxlHk4LpQ==",
+      "dev": true,
+      "dependencies": {
+        "lodash": "^4.17.15"
+      }
+    },
+    "node_modules/eslint-plugin-html": {
+      "version": "6.0.0",
+      "resolved": "https://registry.npmjs.org/eslint-plugin-html/-/eslint-plugin-html-6.0.0.tgz",
+      "integrity": "sha512-PQcGippOHS+HTbQCStmH5MY1BF2MaU8qW/+Mvo/8xTa/ioeMXdSP+IiaBw2+nh0KEMfYQKuTz1Zo+vHynjwhbg==",
+      "dev": true,
+      "dependencies": {
+        "htmlparser2": "^3.10.1"
+      }
+    },
+    "node_modules/eslint-plugin-import": {
+      "version": "2.20.1",
+      "resolved": "https://registry.npmjs.org/eslint-plugin-import/-/eslint-plugin-import-2.20.1.tgz",
+      "integrity": "sha512-qQHgFOTjguR+LnYRoToeZWT62XM55MBVXObHM6SKFd1VzDcX/vqT1kAz8ssqigh5eMj8qXcRoXXGZpPP6RfdCw==",
+      "dev": true,
+      "dependencies": {
+        "array-includes": "^3.0.3",
+        "array.prototype.flat": "^1.2.1",
+        "contains-path": "^0.1.0",
+        "debug": "^2.6.9",
+        "doctrine": "1.5.0",
+        "eslint-import-resolver-node": "^0.3.2",
+        "eslint-module-utils": "^2.4.1",
+        "has": "^1.0.3",
+        "minimatch": "^3.0.4",
+        "object.values": "^1.1.0",
+        "read-pkg-up": "^2.0.0",
+        "resolve": "^1.12.0"
+      }
+    },
+    "node_modules/eslint-plugin-import/node_modules/debug": {
+      "version": "2.6.9",
+      "resolved": "https://registry.npmjs.org/debug/-/debug-2.6.9.tgz",
+      "integrity": "sha512-bC7ElrdJaJnPbAP+1EotYvqZsb3ecl5wi6Bfi6BJTUcNowp6cvspg0jXznRTKDjm/E7AdgFBVeAPVMNcKGsHMA==",
+      "dev": true,
+      "dependencies": {
+        "ms": "2.0.0"
+      }
+    },
+    "node_modules/eslint-plugin-import/node_modules/doctrine": {
+      "version": "1.5.0",
+      "resolved": "https://registry.npmjs.org/doctrine/-/doctrine-1.5.0.tgz",
+      "integrity": "sha1-N53Ocw9hZvds76TmcHoVmwLFpvo=",
+      "dev": true,
+      "dependencies": {
+        "esutils": "^2.0.2",
+        "isarray": "^1.0.0"
+      }
+    },
+    "node_modules/eslint-plugin-import/node_modules/ms": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/ms/-/ms-2.0.0.tgz",
+      "integrity": "sha1-VgiurfwAvmwpAd9fmGF4jeDVl8g=",
+      "dev": true
+    },
+    "node_modules/eslint-plugin-jest": {
+      "version": "23.20.0",
+      "resolved": "https://registry.npmjs.org/eslint-plugin-jest/-/eslint-plugin-jest-23.20.0.tgz",
+      "integrity": "sha512-+6BGQt85OREevBDWCvhqj1yYA4+BFK4XnRZSGJionuEYmcglMZYLNNBBemwzbqUAckURaHdJSBcjHPyrtypZOw==",
+      "dev": true,
+      "dependencies": {
+        "@typescript-eslint/experimental-utils": "^2.5.0"
+      }
+    },
+    "node_modules/eslint-plugin-jsx-a11y": {
+      "version": "6.2.3",
+      "resolved": "https://registry.npmjs.org/eslint-plugin-jsx-a11y/-/eslint-plugin-jsx-a11y-6.2.3.tgz",
+      "integrity": "sha512-CawzfGt9w83tyuVekn0GDPU9ytYtxyxyFZ3aSWROmnRRFQFT2BiPJd7jvRdzNDi6oLWaS2asMeYSNMjWTV4eNg==",
+      "dev": true,
+      "dependencies": {
+        "@babel/runtime": "^7.4.5",
+        "aria-query": "^3.0.0",
+        "array-includes": "^3.0.3",
+        "ast-types-flow": "^0.0.7",
+        "axobject-query": "^2.0.2",
+        "damerau-levenshtein": "^1.0.4",
+        "emoji-regex": "^7.0.2",
+        "has": "^1.0.3",
+        "jsx-ast-utils": "^2.2.1"
+      }
+    },
+    "node_modules/eslint-plugin-mozilla": {
+      "resolved": "tools/lint/eslint/eslint-plugin-mozilla",
+      "link": true
+    },
+    "node_modules/eslint-plugin-no-unsanitized": {
+      "version": "3.1.4",
+      "resolved": "https://registry.npmjs.org/eslint-plugin-no-unsanitized/-/eslint-plugin-no-unsanitized-3.1.4.tgz",
+      "integrity": "sha512-WF1+eZo2Sh+bQNjZuVNwT0dA61zuJORsLh+1Sww7+O6GOPw+WPWIIRfTWNqrmaXaDMhM4SXAqYPcNlhRMiH13g==",
+      "dev": true
+    },
+    "node_modules/eslint-plugin-prettier": {
+      "version": "3.1.2",
+      "resolved": "https://registry.npmjs.org/eslint-plugin-prettier/-/eslint-plugin-prettier-3.1.2.tgz",
+      "integrity": "sha512-GlolCC9y3XZfv3RQfwGew7NnuFDKsfI4lbvRK+PIIo23SFH+LemGs4cKwzAaRa+Mdb+lQO/STaIayno8T5sJJA==",
+      "dev": true,
+      "dependencies": {
+        "prettier-linter-helpers": "^1.0.0"
+      }
+    },
+    "node_modules/eslint-plugin-react": {
+      "version": "7.18.3",
+      "resolved": "https://registry.npmjs.org/eslint-plugin-react/-/eslint-plugin-react-7.18.3.tgz",
+      "integrity": "sha512-Bt56LNHAQCoou88s8ViKRjMB2+36XRejCQ1VoLj716KI1MoE99HpTVvIThJ0rvFmG4E4Gsq+UgToEjn+j044Bg==",
+      "dev": true,
+      "dependencies": {
+        "array-includes": "^3.1.1",
+        "doctrine": "^2.1.0",
+        "has": "^1.0.3",
+        "jsx-ast-utils": "^2.2.3",
+        "object.entries": "^1.1.1",
+        "object.fromentries": "^2.0.2",
+        "object.values": "^1.1.1",
+        "prop-types": "^15.7.2",
+        "resolve": "^1.14.2",
+        "string.prototype.matchall": "^4.0.2"
+      }
+    },
+    "node_modules/eslint-plugin-react/node_modules/doctrine": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/doctrine/-/doctrine-2.1.0.tgz",
+      "integrity": "sha512-35mSku4ZXK0vfCuHEDAwt55dg2jNajHZ1odvF+8SSr82EsZY4QmXfuWso8oEd8zRhVObSN18aM0CjSdoBX7zIw==",
+      "dev": true,
+      "dependencies": {
+        "esutils": "^2.0.2"
+      }
+    },
+    "node_modules/eslint-plugin-spidermonkey-js": {
+      "resolved": "tools/lint/eslint/eslint-plugin-spidermonkey-js",
+      "link": true
+    },
+    "node_modules/eslint-rule-composer": {
+      "version": "0.3.0",
+      "resolved": "https://registry.npmjs.org/eslint-rule-composer/-/eslint-rule-composer-0.3.0.tgz",
+      "integrity": "sha512-bt+Sh8CtDmn2OajxvNO+BX7Wn4CIWMpTRm3MaiKPCQcnnlm0CS2mhui6QaoeQugs+3Kj2ESKEEGJUdVafwhiCg==",
+      "dev": true
+    },
+    "node_modules/eslint-scope": {
+      "version": "5.1.0",
+      "resolved": "https://registry.npmjs.org/eslint-scope/-/eslint-scope-5.1.0.tgz",
+      "integrity": "sha512-iiGRvtxWqgtx5m8EyQUJihBloE4EnYeGE/bz1wSPwJE6tZuJUtHlhqDM4Xj2ukE8Dyy1+HCZ4hE0fzIVMzb58w==",
+      "dev": true,
+      "dependencies": {
+        "esrecurse": "^4.1.0",
+        "estraverse": "^4.1.1"
+      }
+    },
+    "node_modules/eslint-utils": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/eslint-utils/-/eslint-utils-2.1.0.tgz",
+      "integrity": "sha512-w94dQYoauyvlDc43XnGB8lU3Zt713vNChgt4EWwhXAP2XkBvndfxF0AgIqKOOasjPIPzj9JqgwkwbCYD0/V3Zg==",
+      "dev": true,
+      "dependencies": {
+        "eslint-visitor-keys": "^1.1.0"
+      }
+    },
+    "node_modules/eslint-visitor-keys": {
+      "version": "1.3.0",
+      "resolved": "https://registry.npmjs.org/eslint-visitor-keys/-/eslint-visitor-keys-1.3.0.tgz",
+      "integrity": "sha512-6J72N8UNa462wa/KFODt/PJ3IU60SDpC3QXC1Hjc1BXXpfL2C9R5+AU7jhe0F6GREqVMh4Juu+NY7xn+6dipUQ==",
+      "dev": true
+    },
+    "node_modules/eslint/node_modules/ansi-styles": {
+      "version": "4.2.1",
+      "resolved": "https://registry.npmjs.org/ansi-styles/-/ansi-styles-4.2.1.tgz",
+      "integrity": "sha512-9VGjrMsG1vePxcSweQsN20KY/c4zN0h9fLjqAbwbPfahM3t+NL+M9HC8xeXG2I8pX5NoamTGNuomEUFI7fcUjA==",
+      "dev": true,
+      "dependencies": {
+        "@types/color-name": "^1.1.1",
+        "color-convert": "^2.0.1"
+      }
+    },
+    "node_modules/eslint/node_modules/chalk": {
+      "version": "4.1.0",
+      "resolved": "https://registry.npmjs.org/chalk/-/chalk-4.1.0.tgz",
+      "integrity": "sha512-qwx12AxXe2Q5xQ43Ac//I6v5aXTipYrSESdOgzrN+9XjgEpyjpKuvSGaN4qE93f7TQTlerQQ8S+EQ0EyDoVL1A==",
+      "dev": true,
+      "dependencies": {
+        "ansi-styles": "^4.1.0",
+        "supports-color": "^7.1.0"
+      }
+    },
+    "node_modules/eslint/node_modules/color-convert": {
+      "version": "2.0.1",
+      "resolved": "https://registry.npmjs.org/color-convert/-/color-convert-2.0.1.tgz",
+      "integrity": "sha512-RRECPsj7iu/xb5oKYcsFHSppFNnsj/52OVTRKb4zP5onXwVF3zVmmToNcOfGC+CRDpfK/U584fMg38ZHCaElKQ==",
+      "dev": true,
+      "dependencies": {
+        "color-name": "~1.1.4"
+      }
+    },
+    "node_modules/eslint/node_modules/color-name": {
+      "version": "1.1.4",
+      "resolved": "https://registry.npmjs.org/color-name/-/color-name-1.1.4.tgz",
+      "integrity": "sha512-dOy+3AuW3a2wNbZHIuMZpTcgjGuLU/uBL/ubcZF9OXbDo8ff4O8yVp5Bf0efS8uEoYo5q4Fx7dY9OgQGXgAsQA==",
+      "dev": true
+    },
+    "node_modules/eslint/node_modules/globals": {
+      "version": "12.4.0",
+      "resolved": "https://registry.npmjs.org/globals/-/globals-12.4.0.tgz",
+      "integrity": "sha512-BWICuzzDvDoH54NHKCseDanAhE3CeDorgDL5MT6LMXXj2WCnd9UC2szdk4AWLfjdgNBCXLUanXYcpBBKOSWGwg==",
+      "dev": true,
+      "dependencies": {
+        "type-fest": "^0.8.1"
+      }
+    },
+    "node_modules/eslint/node_modules/has-flag": {
+      "version": "4.0.0",
+      "resolved": "https://registry.npmjs.org/has-flag/-/has-flag-4.0.0.tgz",
+      "integrity": "sha512-EykJT/Q1KjTWctppgIAgfSO0tKVuZUjhgMr17kqTumMl6Afv3EISleU7qZUzoXDFTAHTDC4NOoG/ZxU3EvlMPQ==",
+      "dev": true
+    },
+    "node_modules/eslint/node_modules/semver": {
+      "version": "7.3.2",
+      "resolved": "https://registry.npmjs.org/semver/-/semver-7.3.2.tgz",
+      "integrity": "sha512-OrOb32TeeambH6UrhtShmF7CRDqhL6/5XpPNp2DuRH6+9QLw/orhp72j87v8Qa1ScDkvrrBNpZcDejAirJmfXQ==",
+      "dev": true
+    },
+    "node_modules/eslint/node_modules/supports-color": {
+      "version": "7.2.0",
+      "resolved": "https://registry.npmjs.org/supports-color/-/supports-color-7.2.0.tgz",
+      "integrity": "sha512-qpCAvRl9stuOHveKsn7HncJRvv501qIacKzQlO/+Lwxc9+0q2wLyv4Dfvt80/DPn2pqOBsJdDiogXGR9+OvwRw==",
+      "dev": true,
+      "dependencies": {
+        "has-flag": "^4.0.0"
+      }
+    },
+    "node_modules/espree": {
+      "version": "7.3.0",
+      "resolved": "https://registry.npmjs.org/espree/-/espree-7.3.0.tgz",
+      "integrity": "sha512-dksIWsvKCixn1yrEXO8UosNSxaDoSYpq9reEjZSbHLpT5hpaCAKTLBwq0RHtLrIr+c0ByiYzWT8KTMRzoRCNlw==",
+      "dev": true,
+      "dependencies": {
+        "acorn": "^7.4.0",
+        "acorn-jsx": "^5.2.0",
+        "eslint-visitor-keys": "^1.3.0"
+      }
+    },
+    "node_modules/esprima": {
+      "version": "4.0.1",
+      "resolved": "https://registry.npmjs.org/esprima/-/esprima-4.0.1.tgz",
+      "integrity": "sha512-eGuFFw7Upda+g4p+QHvnW0RyTX/SVeJBDM/gCtMARO0cLuT2HcEKnTPvhjV6aGeqrCB/sbNop0Kszm0jsaWU4A==",
+      "dev": true
+    },
+    "node_modules/esquery": {
+      "version": "1.3.1",
+      "resolved": "https://registry.npmjs.org/esquery/-/esquery-1.3.1.tgz",
+      "integrity": "sha512-olpvt9QG0vniUBZspVRN6lwB7hOZoTRtT+jzR+tS4ffYx2mzbw+z0XCOk44aaLYKApNX5nMm+E+P6o25ip/DHQ==",
+      "dev": true,
+      "dependencies": {
+        "estraverse": "^5.1.0"
+      }
+    },
+    "node_modules/esquery/node_modules/estraverse": {
+      "version": "5.2.0",
+      "resolved": "https://registry.npmjs.org/estraverse/-/estraverse-5.2.0.tgz",
+      "integrity": "sha512-BxbNGGNm0RyRYvUdHpIwv9IWzeM9XClbOxwoATuFdOE7ZE6wHL+HQ5T8hoPM+zHvmKzzsEqhgy0GrQ5X13afiQ==",
+      "dev": true
+    },
+    "node_modules/esrecurse": {
+      "version": "4.2.1",
+      "resolved": "https://registry.npmjs.org/esrecurse/-/esrecurse-4.2.1.tgz",
+      "integrity": "sha512-64RBB++fIOAXPw3P9cy89qfMlvZEXZkqqJkjqqXIvzP5ezRZjW+lPWjw35UX/3EhUPFYbg5ER4JYgDw4007/DQ==",
+      "dev": true,
+      "dependencies": {
+        "estraverse": "^4.1.0"
+      }
+    },
+    "node_modules/estraverse": {
+      "version": "4.3.0",
+      "resolved": "https://registry.npmjs.org/estraverse/-/estraverse-4.3.0.tgz",
+      "integrity": "sha512-39nnKffWz8xN1BU/2c79n9nB9HDzo0niYUqx6xyqUnyoAnQyyWpOTdZEeiCch8BBu515t4wp9ZmgVfVhn9EBpw==",
+      "dev": true
+    },
+    "node_modules/esutils": {
+      "version": "2.0.3",
+      "resolved": "https://registry.npmjs.org/esutils/-/esutils-2.0.3.tgz",
+      "integrity": "sha512-kVscqXk4OCp68SZ0dkgEKVi6/8ij300KBWTJq32P/dYeWTSwK41WyTxalN1eRmA5Z9UU/LX9D7FWSmV9SAYx6g==",
+      "dev": true
+    },
+    "node_modules/fast-deep-equal": {
+      "version": "3.1.3",
+      "resolved": "https://registry.npmjs.org/fast-deep-equal/-/fast-deep-equal-3.1.3.tgz",
+      "integrity": "sha512-f3qQ9oQy9j2AhBe/H9VC91wLmKBCCU/gDOnKNAYG5hswO7BLKj09Hc5HYNz9cGI++xlpDCIgDaitVs03ATR84Q==",
+      "dev": true
+    },
+    "node_modules/fast-diff": {
+      "version": "1.2.0",
+      "resolved": "https://registry.npmjs.org/fast-diff/-/fast-diff-1.2.0.tgz",
+      "integrity": "sha512-xJuoT5+L99XlZ8twedaRf6Ax2TgQVxvgZOYoPKqZufmJib0tL2tegPBOZb1pVNgIhlqDlA0eO0c3wBvQcmzx4w==",
+      "dev": true
+    },
+    "node_modules/fast-glob": {
+      "version": "3.2.4",
+      "resolved": "https://registry.npmjs.org/fast-glob/-/fast-glob-3.2.4.tgz",
+      "integrity": "sha512-kr/Oo6PX51265qeuCYsyGypiO5uJFgBS0jksyG7FUeCyQzNwYnzrNIMR1NXfkZXsMYXYLRAHgISHBz8gQcxKHQ==",
+      "dev": true,
+      "dependencies": {
+        "@nodelib/fs.stat": "^2.0.2",
+        "@nodelib/fs.walk": "^1.2.3",
+        "glob-parent": "^5.1.0",
+        "merge2": "^1.3.0",
+        "micromatch": "^4.0.2",
+        "picomatch": "^2.2.1"
+      },
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/fast-json-stable-stringify": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/fast-json-stable-stringify/-/fast-json-stable-stringify-2.1.0.tgz",
+      "integrity": "sha512-lhd/wF+Lk98HZoTCtlVraHtfh5XYijIjalXck7saUtuanSDyLMxnHhSXEDJqHxD7msR8D0uCmqlkwjCV8xvwHw==",
+      "dev": true
+    },
+    "node_modules/fast-levenshtein": {
+      "version": "2.0.6",
+      "resolved": "https://registry.npmjs.org/fast-levenshtein/-/fast-levenshtein-2.0.6.tgz",
+      "integrity": "sha1-PYpcZog6FqMMqGQ+hR8Zuqd5eRc=",
+      "dev": true
+    },
+    "node_modules/fastq": {
+      "version": "1.9.0",
+      "resolved": "https://registry.npmjs.org/fastq/-/fastq-1.9.0.tgz",
+      "integrity": "sha512-i7FVWL8HhVY+CTkwFxkN2mk3h+787ixS5S63eb78diVRc1MCssarHq3W5cj0av7YDSwmaV928RNag+U1etRQ7w==",
+      "dev": true,
+      "dependencies": {
+        "reusify": "^1.0.4"
+      }
+    },
+    "node_modules/file-entry-cache": {
+      "version": "5.0.1",
+      "resolved": "https://registry.npmjs.org/file-entry-cache/-/file-entry-cache-5.0.1.tgz",
+      "integrity": "sha512-bCg29ictuBaKUwwArK4ouCaqDgLZcysCFLmM/Yn/FDoqndh/9vNuQfXRDvTuXKLxfD/JtZQGKFT8MGcJBK644g==",
+      "dev": true,
+      "dependencies": {
+        "flat-cache": "^2.0.1"
+      }
+    },
+    "node_modules/fill-range": {
+      "version": "7.0.1",
+      "resolved": "https://registry.npmjs.org/fill-range/-/fill-range-7.0.1.tgz",
+      "integrity": "sha512-qOo9F+dMUmC2Lcb4BbVvnKJxTPjCm+RRpe4gDuGrzkL7mEVl/djYSu2OdQ2Pa302N4oqkSg9ir6jaLWJ2USVpQ==",
+      "dev": true,
+      "dependencies": {
+        "to-regex-range": "^5.0.1"
+      },
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/find-up": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/find-up/-/find-up-2.1.0.tgz",
+      "integrity": "sha1-RdG35QbHF93UgndaK3eSCjwMV6c=",
+      "dev": true,
+      "dependencies": {
+        "locate-path": "^2.0.0"
+      }
+    },
+    "node_modules/flat": {
+      "version": "5.0.2",
+      "resolved": "https://registry.npmjs.org/flat/-/flat-5.0.2.tgz",
+      "integrity": "sha512-b6suED+5/3rTpUBdG1gupIl8MPFCAMA0QXwmljLhvCUKcUvdE4gWky9zpuGCcXHOsz4J9wPGNWq6OKpmIzz3hQ==",
+      "dev": true,
+      "bin": {
+        "flat": "cli.js"
+      }
+    },
+    "node_modules/flat-cache": {
+      "version": "2.0.1",
+      "resolved": "https://registry.npmjs.org/flat-cache/-/flat-cache-2.0.1.tgz",
+      "integrity": "sha512-LoQe6yDuUMDzQAEH8sgmh4Md6oZnc/7PjtwjNFSzveXqSHt6ka9fPBuso7IGf9Rz4uqnSnWiFH2B/zj24a5ReA==",
+      "dev": true,
+      "dependencies": {
+        "flatted": "^2.0.0",
+        "rimraf": "2.6.3",
+        "write": "1.0.3"
+      }
+    },
+    "node_modules/flatted": {
+      "version": "2.0.2",
+      "resolved": "https://registry.npmjs.org/flatted/-/flatted-2.0.2.tgz",
+      "integrity": "sha512-r5wGx7YeOwNWNlCA0wQ86zKyDLMQr+/RB8xy74M4hTphfmjlijTSSXGuH8rnvKZnfT9i+75zmd8jcKdMR4O6jA==",
+      "dev": true
+    },
+    "node_modules/fs.realpath": {
+      "version": "1.0.0",
+      "resolved": "https://registry.npmjs.org/fs.realpath/-/fs.realpath-1.0.0.tgz",
+      "integrity": "sha1-FQStJSMVjKpA20onh8sBQRmU6k8=",
+      "dev": true
+    },
+    "node_modules/function-bind": {
+      "version": "1.1.1",
+      "resolved": "https://registry.npmjs.org/function-bind/-/function-bind-1.1.1.tgz",
+      "integrity": "sha512-yIovAzMX49sF8Yl58fSCWJ5svSLuaibPxXQJFLmBObTuCr0Mf1KiPopGM9NiFjiYBCbfaa2Fh6breQ6ANVTI0A==",
+      "dev": true
+    },
+    "node_modules/functional-red-black-tree": {
+      "version": "1.0.1",
+      "resolved": "https://registry.npmjs.org/functional-red-black-tree/-/functional-red-black-tree-1.0.1.tgz",
+      "integrity": "sha1-GwqzvVU7Kg1jmdKcDj6gslIHgyc=",
+      "dev": true
+    },
+    "node_modules/gensync": {
+      "version": "1.0.0-beta.1",
+      "resolved": "https://registry.npmjs.org/gensync/-/gensync-1.0.0-beta.1.tgz",
+      "integrity": "sha512-r8EC6NO1sngH/zdD9fiRDLdcgnbayXah+mLgManTaIZJqEC1MZstmnox8KpnI2/fxQwrp5OpCOYWLp4rBl4Jcg==",
+      "dev": true
+    },
+    "node_modules/get-caller-file": {
+      "version": "2.0.5",
+      "resolved": "https://registry.npmjs.org/get-caller-file/-/get-caller-file-2.0.5.tgz",
+      "integrity": "sha512-DyFP3BM/3YHTQOCUL/w0OZHR0lpKeGrxotcHWcqNEdnltqFwXVfhEBQ94eIo34AfQpo0rGki4cyIiftY06h2Fg==",
+      "dev": true,
+      "engines": {
+        "node": "6.* || 8.* || >= 10.*"
+      }
+    },
+    "node_modules/get-stdin": {
+      "version": "6.0.0",
+      "resolved": "https://registry.npmjs.org/get-stdin/-/get-stdin-6.0.0.tgz",
+      "integrity": "sha512-jp4tHawyV7+fkkSKyvjuLZswblUtz+SQKzSWnBbii16BuZksJlU1wuBYXY75r+duh/llF1ur6oNwi+2ZzjKZ7g==",
+      "dev": true
+    },
+    "node_modules/glob": {
+      "version": "7.1.6",
+      "resolved": "https://registry.npmjs.org/glob/-/glob-7.1.6.tgz",
+      "integrity": "sha512-LwaxwyZ72Lk7vZINtNNrywX0ZuLyStrdDtabefZKAY5ZGJhVtgdznluResxNmPitE0SAO+O26sWTHeKSI2wMBA==",
+      "dev": true,
+      "dependencies": {
+        "fs.realpath": "^1.0.0",
+        "inflight": "^1.0.4",
+        "inherits": "2",
+        "minimatch": "^3.0.4",
+        "once": "^1.3.0",
+        "path-is-absolute": "^1.0.0"
+      }
+    },
+    "node_modules/glob-parent": {
+      "version": "5.1.1",
+      "resolved": "https://registry.npmjs.org/glob-parent/-/glob-parent-5.1.1.tgz",
+      "integrity": "sha512-FnI+VGOpnlGHWZxthPGR+QhR78fuiK0sNLkHQv+bL9fQi57lNNdquIbna/WrfROrolq8GK5Ek6BiMwqL/voRYQ==",
+      "dev": true,
+      "dependencies": {
+        "is-glob": "^4.0.1"
+      }
+    },
+    "node_modules/globals": {
+      "version": "11.12.0",
+      "resolved": "https://registry.npmjs.org/globals/-/globals-11.12.0.tgz",
+      "integrity": "sha512-WOBp/EEGUiIsJSp7wcv/y6MO+lV9UoncWqxuFfm8eBwzWNgyfBd6Gz+IeKQ9jCmyhoH99g15M3T+QaVHFjizVA==",
+      "dev": true
+    },
+    "node_modules/globby": {
+      "version": "11.0.1",
+      "resolved": "https://registry.npmjs.org/globby/-/globby-11.0.1.tgz",
+      "integrity": "sha512-iH9RmgwCmUJHi2z5o2l3eTtGBtXek1OYlHrbcxOYugyHLmAsZrPj43OtHThd62Buh/Vv6VyCBD2bdyWcGNQqoQ==",
+      "dev": true,
+      "dependencies": {
+        "array-union": "^2.1.0",
+        "dir-glob": "^3.0.1",
+        "fast-glob": "^3.1.1",
+        "ignore": "^5.1.4",
+        "merge2": "^1.3.0",
+        "slash": "^3.0.0"
+      },
+      "engines": {
+        "node": ">=10"
+      },
+      "funding": {
+        "url": "https://github.com/sponsors/sindresorhus"
+      }
+    },
+    "node_modules/globby/node_modules/ignore": {
+      "version": "5.1.8",
+      "resolved": "https://registry.npmjs.org/ignore/-/ignore-5.1.8.tgz",
+      "integrity": "sha512-BMpfD7PpiETpBl/A6S498BaIJ6Y/ABT93ETbby2fP00v4EbvPBXWEoaR1UBPKs3iR53pJY7EtZk5KACI57i1Uw==",
+      "dev": true,
+      "engines": {
+        "node": ">= 4"
+      }
+    },
+    "node_modules/graceful-fs": {
+      "version": "4.2.4",
+      "resolved": "https://registry.npmjs.org/graceful-fs/-/graceful-fs-4.2.4.tgz",
+      "integrity": "sha512-WjKPNJF79dtJAVniUlGGWHYGz2jWxT6VhN/4m1NdkbZ2nOsEF+cI1Edgql5zCRhs/VsQYRvrXctxktVXZUkixw==",
+      "dev": true
+    },
+    "node_modules/growl": {
+      "version": "1.10.5",
+      "resolved": "https://registry.npmjs.org/growl/-/growl-1.10.5.tgz",
+      "integrity": "sha512-qBr4OuELkhPenW6goKVXiv47US3clb3/IbuWF9KNKEijAy9oeHxU9IgzjvJhHkUzhaj7rOUD7+YGWqUjLp5oSA==",
+      "dev": true,
+      "engines": {
+        "node": ">=4.x"
+      }
+    },
+    "node_modules/has": {
+      "version": "1.0.3",
+      "resolved": "https://registry.npmjs.org/has/-/has-1.0.3.tgz",
+      "integrity": "sha512-f2dvO0VU6Oej7RkWJGrehjbzMAjFp5/VKPp5tTpWIV4JHHZK1/BxbFRtf/siA2SWTe09caDmVtYYzWEIbBS4zw==",
+      "dev": true,
+      "dependencies": {
+        "function-bind": "^1.1.1"
+      }
+    },
+    "node_modules/has-flag": {
+      "version": "3.0.0",
+      "resolved": "https://registry.npmjs.org/has-flag/-/has-flag-3.0.0.tgz",
+      "integrity": "sha1-tdRU3CGZriJWmfNGfloH87lVuv0=",
+      "dev": true
+    },
+    "node_modules/has-symbols": {
+      "version": "1.0.1",
+      "resolved": "https://registry.npmjs.org/has-symbols/-/has-symbols-1.0.1.tgz",
+      "integrity": "sha512-PLcsoqu++dmEIZB+6totNFKq/7Do+Z0u4oT0zKOJNl3lYK6vGwwu2hjHs+68OEZbTjiUE9bgOABXbP/GvrS0Kg==",
+      "dev": true
+    },
+    "node_modules/he": {
+      "version": "1.2.0",
+      "resolved": "https://registry.npmjs.org/he/-/he-1.2.0.tgz",
+      "integrity": "sha512-F/1DnUGPopORZi0ni+CvrCgHQ5FyEAHRLSApuYWMmrbSwoN2Mn/7k+Gl38gJnR7yyDZk6WLXwiGod1JOWNDKGw==",
+      "dev": true,
+      "bin": {
+        "he": "bin/he"
+      }
+    },
+    "node_modules/hosted-git-info": {
+      "version": "2.8.8",
+      "resolved": "https://registry.npmjs.org/hosted-git-info/-/hosted-git-info-2.8.8.tgz",
+      "integrity": "sha512-f/wzC2QaWBs7t9IYqB4T3sR1xviIViXJRJTWBlx2Gf3g0Xi5vI7Yy4koXQ1c9OYDGHN9sBy1DQ2AB8fqZBWhUg==",
+      "dev": true
+    },
+    "node_modules/htmlparser2": {
+      "version": "3.10.1",
+      "resolved": "https://registry.npmjs.org/htmlparser2/-/htmlparser2-3.10.1.tgz",
+      "integrity": "sha512-IgieNijUMbkDovyoKObU1DUhm1iwNYE/fuifEoEHfd1oZKZDaONBSkal7Y01shxsM49R4XaMdGez3WnF9UfiCQ==",
+      "dev": true,
+      "dependencies": {
+        "domelementtype": "^1.3.1",
+        "domhandler": "^2.3.0",
+        "domutils": "^1.5.1",
+        "entities": "^1.1.1",
+        "inherits": "^2.0.1",
+        "readable-stream": "^3.1.1"
+      }
+    },
+    "node_modules/ignore": {
+      "version": "4.0.6",
+      "resolved": "https://registry.npmjs.org/ignore/-/ignore-4.0.6.tgz",
+      "integrity": "sha512-cyFDKrqc/YdcWFniJhzI42+AzS+gNwmUzOSFcRCQYwySuBBBy/KjuxWLZ/FHEH6Moq1NizMOBWyTcv8O4OZIMg==",
+      "dev": true
+    },
+    "node_modules/import-fresh": {
+      "version": "3.2.1",
+      "resolved": "https://registry.npmjs.org/import-fresh/-/import-fresh-3.2.1.tgz",
+      "integrity": "sha512-6e1q1cnWP2RXD9/keSkxHScg508CdXqXWgWBaETNhyuBFz+kUZlKboh+ISK+bU++DmbHimVBrOz/zzPe0sZ3sQ==",
+      "dev": true,
+      "dependencies": {
+        "parent-module": "^1.0.0",
+        "resolve-from": "^4.0.0"
+      }
+    },
+    "node_modules/imurmurhash": {
+      "version": "0.1.4",
+      "resolved": "https://registry.npmjs.org/imurmurhash/-/imurmurhash-0.1.4.tgz",
+      "integrity": "sha1-khi5srkoojixPcT7a21XbyMUU+o=",
+      "dev": true
+    },
+    "node_modules/inflight": {
+      "version": "1.0.6",
+      "resolved": "https://registry.npmjs.org/inflight/-/inflight-1.0.6.tgz",
+      "integrity": "sha1-Sb1jMdfQLQwJvJEKEHW6gWW1bfk=",
+      "dev": true,
+      "dependencies": {
+        "once": "^1.3.0",
+        "wrappy": "1"
+      }
+    },
+    "node_modules/inherits": {
+      "version": "2.0.4",
+      "resolved": "https://registry.npmjs.org/inherits/-/inherits-2.0.4.tgz",
+      "integrity": "sha512-k/vGaX4/Yla3WzyMCvTQOXYeIHvqOKtnqBduzTHpzpQZzAskKMhZ2K+EnBiSM9zGSoIFeMpXKxa4dYeZIQqewQ==",
+      "dev": true
+    },
+    "node_modules/internal-slot": {
+      "version": "1.0.2",
+      "resolved": "https://registry.npmjs.org/internal-slot/-/internal-slot-1.0.2.tgz",
+      "integrity": "sha512-2cQNfwhAfJIkU4KZPkDI+Gj5yNNnbqi40W9Gge6dfnk4TocEVm00B3bdiL+JINrbGJil2TeHvM4rETGzk/f/0g==",
+      "dev": true,
+      "dependencies": {
+        "es-abstract": "^1.17.0-next.1",
+        "has": "^1.0.3",
+        "side-channel": "^1.0.2"
+      }
+    },
+    "node_modules/is-arrayish": {
+      "version": "0.2.1",
+      "resolved": "https://registry.npmjs.org/is-arrayish/-/is-arrayish-0.2.1.tgz",
+      "integrity": "sha1-d8mYQFJ6qOyxqLppe4BkWnqSap0=",
+      "dev": true
+    },
+    "node_modules/is-binary-path": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/is-binary-path/-/is-binary-path-2.1.0.tgz",
+      "integrity": "sha512-ZMERYes6pDydyuGidse7OsHxtbI7WVeUEozgR/g7rd0xUimYNlvZRE/K2MgZTjWy725IfelLeVcEM97mmtRGXw==",
+      "dev": true,
+      "dependencies": {
+        "binary-extensions": "^2.0.0"
+      },
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/is-callable": {
+      "version": "1.2.0",
+      "resolved": "https://registry.npmjs.org/is-callable/-/is-callable-1.2.0.tgz",
+      "integrity": "sha512-pyVD9AaGLxtg6srb2Ng6ynWJqkHU9bEM087AKck0w8QwDarTfNcpIYoU8x8Hv2Icm8u6kFJM18Dag8lyqGkviw==",
+      "dev": true
+    },
+    "node_modules/is-date-object": {
+      "version": "1.0.2",
+      "resolved": "https://registry.npmjs.org/is-date-object/-/is-date-object-1.0.2.tgz",
+      "integrity": "sha512-USlDT524woQ08aoZFzh3/Z6ch9Y/EWXEHQ/AaRN0SkKq4t2Jw2R2339tSXmwuVoY7LLlBCbOIlx2myP/L5zk0g==",
+      "dev": true
+    },
+    "node_modules/is-extglob": {
+      "version": "2.1.1",
+      "resolved": "https://registry.npmjs.org/is-extglob/-/is-extglob-2.1.1.tgz",
+      "integrity": "sha1-qIwCU1eR8C7TfHahueqXc8gz+MI=",
+      "dev": true
+    },
+    "node_modules/is-fullwidth-code-point": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/is-fullwidth-code-point/-/is-fullwidth-code-point-2.0.0.tgz",
+      "integrity": "sha1-o7MKXE8ZkYMWeqq5O+764937ZU8=",
+      "dev": true
+    },
+    "node_modules/is-glob": {
+      "version": "4.0.1",
+      "resolved": "https://registry.npmjs.org/is-glob/-/is-glob-4.0.1.tgz",
+      "integrity": "sha512-5G0tKtBTFImOqDnLB2hG6Bp2qcKEFduo4tZu9MT/H6NQv/ghhy30o55ufafxJ/LdH79LLs2Kfrn85TLKyA7BUg==",
+      "dev": true,
+      "dependencies": {
+        "is-extglob": "^2.1.1"
+      }
+    },
+    "node_modules/is-number": {
+      "version": "7.0.0",
+      "resolved": "https://registry.npmjs.org/is-number/-/is-number-7.0.0.tgz",
+      "integrity": "sha512-41Cifkg6e8TylSpdtTpeLVMqvSBEVzTttHvERD741+pnZ8ANv0004MRL43QKPDlK9cGvNp6NZWZUBlbGXYxxng==",
+      "dev": true,
+      "engines": {
+        "node": ">=0.12.0"
+      }
+    },
+    "node_modules/is-plain-obj": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/is-plain-obj/-/is-plain-obj-2.1.0.tgz",
+      "integrity": "sha512-YWnfyRwxL/+SsrWYfOpUtz5b3YD+nyfkHvjbcanzk8zgyO4ASD67uVMRt8k5bM4lLMDnXfriRhOpemw+NfT1eA==",
+      "dev": true,
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/is-regex": {
+      "version": "1.1.0",
+      "resolved": "https://registry.npmjs.org/is-regex/-/is-regex-1.1.0.tgz",
+      "integrity": "sha512-iI97M8KTWID2la5uYXlkbSDQIg4F6o1sYboZKKTDpnDQMLtUL86zxhgDet3Q2SriaYsyGqZ6Mn2SjbRKeLHdqw==",
+      "dev": true,
+      "dependencies": {
+        "has-symbols": "^1.0.1"
+      }
+    },
+    "node_modules/is-string": {
+      "version": "1.0.5",
+      "resolved": "https://registry.npmjs.org/is-string/-/is-string-1.0.5.tgz",
+      "integrity": "sha512-buY6VNRjhQMiF1qWDouloZlQbRhDPCebwxSjxMjxgemYT46YMd2NR0/H+fBhEfWX4A/w9TBJ+ol+okqJKFE6vQ==",
+      "dev": true
+    },
+    "node_modules/is-symbol": {
+      "version": "1.0.3",
+      "resolved": "https://registry.npmjs.org/is-symbol/-/is-symbol-1.0.3.tgz",
+      "integrity": "sha512-OwijhaRSgqvhm/0ZdAcXNZt9lYdKFpcRDT5ULUuYXPoT794UNOdU+gpT6Rzo7b4V2HUl/op6GqY894AZwv9faQ==",
+      "dev": true,
+      "dependencies": {
+        "has-symbols": "^1.0.1"
+      }
+    },
+    "node_modules/isarray": {
+      "version": "1.0.0",
+      "resolved": "https://registry.npmjs.org/isarray/-/isarray-1.0.0.tgz",
+      "integrity": "sha1-u5NdSFgsuhaMBoNJV6VKPgcSTxE=",
+      "dev": true
+    },
+    "node_modules/isexe": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/isexe/-/isexe-2.0.0.tgz",
+      "integrity": "sha1-6PvzdNxVb/iUehDcsFctYz8s+hA=",
+      "dev": true
+    },
+    "node_modules/js-tokens": {
+      "version": "4.0.0",
+      "resolved": "https://registry.npmjs.org/js-tokens/-/js-tokens-4.0.0.tgz",
+      "integrity": "sha512-RdJUflcE3cUzKiMqQgsCu06FPu9UdIJO0beYbPhHN4k6apgJtifcoCtT9bcxOpYBtpD2kCM6Sbzg4CausW/PKQ==",
+      "dev": true
+    },
+    "node_modules/js-yaml": {
+      "version": "3.14.0",
+      "resolved": "https://registry.npmjs.org/js-yaml/-/js-yaml-3.14.0.tgz",
+      "integrity": "sha512-/4IbIeHcD9VMHFqDR/gQ7EdZdLimOvW2DdcxFjdyyZ9NsbS+ccrXqVWDtab/lRl5AlUqmpBx8EhPaWR+OtY17A==",
+      "dev": true,
+      "dependencies": {
+        "argparse": "^1.0.7",
+        "esprima": "^4.0.0"
+      }
+    },
+    "node_modules/jsesc": {
+      "version": "2.5.2",
+      "resolved": "https://registry.npmjs.org/jsesc/-/jsesc-2.5.2.tgz",
+      "integrity": "sha512-OYu7XEzjkCQ3C5Ps3QIZsQfNpqoJyZZA99wd9aWd05NCtC5pWOkShK2mkL6HXQR6/Cy2lbNdPlZBpuQHXE63gA==",
+      "dev": true
+    },
+    "node_modules/json-schema-traverse": {
+      "version": "0.4.1",
+      "resolved": "https://registry.npmjs.org/json-schema-traverse/-/json-schema-traverse-0.4.1.tgz",
+      "integrity": "sha512-xbbCH5dCYU5T8LcEhhuh7HJ88HXuW3qsI3Y0zOZFKfZEHcpWiHU/Jxzk629Brsab/mMiHQti9wMP+845RPe3Vg==",
+      "dev": true
+    },
+    "node_modules/json-stable-stringify-without-jsonify": {
+      "version": "1.0.1",
+      "resolved": "https://registry.npmjs.org/json-stable-stringify-without-jsonify/-/json-stable-stringify-without-jsonify-1.0.1.tgz",
+      "integrity": "sha1-nbe1lJatPzz+8wp1FC0tkwrXJlE=",
+      "dev": true
+    },
+    "node_modules/json5": {
+      "version": "2.1.3",
+      "resolved": "https://registry.npmjs.org/json5/-/json5-2.1.3.tgz",
+      "integrity": "sha512-KXPvOm8K9IJKFM0bmdn8QXh7udDh1g/giieX0NLCaMnb4hEiVFqnop2ImTXCc5e0/oHz3LTqmHGtExn5hfMkOA==",
+      "dev": true,
+      "dependencies": {
+        "minimist": "^1.2.5"
+      }
+    },
+    "node_modules/jsx-ast-utils": {
+      "version": "2.4.1",
+      "resolved": "https://registry.npmjs.org/jsx-ast-utils/-/jsx-ast-utils-2.4.1.tgz",
+      "integrity": "sha512-z1xSldJ6imESSzOjd3NNkieVJKRlKYSOtMG8SFyCj2FIrvSaSuli/WjpBkEzCBoR9bYYYFgqJw61Xhu7Lcgk+w==",
+      "dev": true,
+      "dependencies": {
+        "array-includes": "^3.1.1",
+        "object.assign": "^4.1.0"
+      }
+    },
+    "node_modules/levn": {
+      "version": "0.4.1",
+      "resolved": "https://registry.npmjs.org/levn/-/levn-0.4.1.tgz",
+      "integrity": "sha512-+bT2uH4E5LGE7h/n3evcS/sQlJXCpIp6ym8OWJ5eV6+67Dsql/LaaT7qJBAt2rzfoa/5QBGBhxDix1dMt2kQKQ==",
+      "dev": true,
+      "dependencies": {
+        "prelude-ls": "^1.2.1",
+        "type-check": "~0.4.0"
+      }
+    },
+    "node_modules/load-json-file": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/load-json-file/-/load-json-file-2.0.0.tgz",
+      "integrity": "sha1-eUfkIUmvgNaWy/eXvKq8/h/inKg=",
+      "dev": true,
+      "dependencies": {
+        "graceful-fs": "^4.1.2",
+        "parse-json": "^2.2.0",
+        "pify": "^2.0.0",
+        "strip-bom": "^3.0.0"
+      }
+    },
+    "node_modules/locate-path": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/locate-path/-/locate-path-2.0.0.tgz",
+      "integrity": "sha1-K1aLJl7slExtnA3pw9u7ygNUzY4=",
+      "dev": true,
+      "dependencies": {
+        "p-locate": "^2.0.0",
+        "path-exists": "^3.0.0"
+      }
+    },
+    "node_modules/lodash": {
+      "version": "4.17.19",
+      "resolved": "https://registry.npmjs.org/lodash/-/lodash-4.17.19.tgz",
+      "integrity": "sha512-JNvd8XER9GQX0v2qJgsaN/mzFCNA5BRe/j8JN9d+tWyGLSodKQHKFicdwNYzWwI3wjRnaKPsGj1XkBjx/F96DQ==",
+      "dev": true
+    },
+    "node_modules/log-symbols": {
+      "version": "4.0.0",
+      "resolved": "https://registry.npmjs.org/log-symbols/-/log-symbols-4.0.0.tgz",
+      "integrity": "sha512-FN8JBzLx6CzeMrB0tg6pqlGU1wCrXW+ZXGH481kfsBqer0hToTIiHdjH4Mq8xJUbvATujKCvaREGWpGUionraA==",
+      "dev": true,
+      "dependencies": {
+        "chalk": "^4.0.0"
+      },
+      "engines": {
+        "node": ">=10"
+      }
+    },
+    "node_modules/log-symbols/node_modules/ansi-styles": {
+      "version": "4.3.0",
+      "resolved": "https://registry.npmjs.org/ansi-styles/-/ansi-styles-4.3.0.tgz",
+      "integrity": "sha512-zbB9rCJAT1rbjiVDb2hqKFHNYLxgtk8NURxZ3IZwD3F6NtxbXZQCnnSi1Lkx+IDohdPlFp222wVALIheZJQSEg==",
+      "dev": true,
+      "dependencies": {
+        "color-convert": "^2.0.1"
+      },
+      "engines": {
+        "node": ">=8"
+      },
+      "funding": {
+        "url": "https://github.com/chalk/ansi-styles?sponsor=1"
+      }
+    },
+    "node_modules/log-symbols/node_modules/chalk": {
+      "version": "4.1.0",
+      "resolved": "https://registry.npmjs.org/chalk/-/chalk-4.1.0.tgz",
+      "integrity": "sha512-qwx12AxXe2Q5xQ43Ac//I6v5aXTipYrSESdOgzrN+9XjgEpyjpKuvSGaN4qE93f7TQTlerQQ8S+EQ0EyDoVL1A==",
+      "dev": true,
+      "dependencies": {
+        "ansi-styles": "^4.1.0",
+        "supports-color": "^7.1.0"
+      },
+      "engines": {
+        "node": ">=10"
+      },
+      "funding": {
+        "url": "https://github.com/chalk/chalk?sponsor=1"
+      }
+    },
+    "node_modules/log-symbols/node_modules/color-convert": {
+      "version": "2.0.1",
+      "resolved": "https://registry.npmjs.org/color-convert/-/color-convert-2.0.1.tgz",
+      "integrity": "sha512-RRECPsj7iu/xb5oKYcsFHSppFNnsj/52OVTRKb4zP5onXwVF3zVmmToNcOfGC+CRDpfK/U584fMg38ZHCaElKQ==",
+      "dev": true,
+      "dependencies": {
+        "color-name": "~1.1.4"
+      },
+      "engines": {
+        "node": ">=7.0.0"
+      }
+    },
+    "node_modules/log-symbols/node_modules/color-name": {
+      "version": "1.1.4",
+      "resolved": "https://registry.npmjs.org/color-name/-/color-name-1.1.4.tgz",
+      "integrity": "sha512-dOy+3AuW3a2wNbZHIuMZpTcgjGuLU/uBL/ubcZF9OXbDo8ff4O8yVp5Bf0efS8uEoYo5q4Fx7dY9OgQGXgAsQA==",
+      "dev": true
+    },
+    "node_modules/log-symbols/node_modules/has-flag": {
+      "version": "4.0.0",
+      "resolved": "https://registry.npmjs.org/has-flag/-/has-flag-4.0.0.tgz",
+      "integrity": "sha512-EykJT/Q1KjTWctppgIAgfSO0tKVuZUjhgMr17kqTumMl6Afv3EISleU7qZUzoXDFTAHTDC4NOoG/ZxU3EvlMPQ==",
+      "dev": true,
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/log-symbols/node_modules/supports-color": {
+      "version": "7.2.0",
+      "resolved": "https://registry.npmjs.org/supports-color/-/supports-color-7.2.0.tgz",
+      "integrity": "sha512-qpCAvRl9stuOHveKsn7HncJRvv501qIacKzQlO/+Lwxc9+0q2wLyv4Dfvt80/DPn2pqOBsJdDiogXGR9+OvwRw==",
+      "dev": true,
+      "dependencies": {
+        "has-flag": "^4.0.0"
+      },
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/loose-envify": {
+      "version": "1.4.0",
+      "resolved": "https://registry.npmjs.org/loose-envify/-/loose-envify-1.4.0.tgz",
+      "integrity": "sha512-lyuxPGr/Wfhrlem2CL/UcnUc1zcqKAImBDzukY7Y5F/yQiNdko6+fRLevlw1HgMySw7f611UIY408EtxRSoK3Q==",
+      "dev": true,
+      "dependencies": {
+        "js-tokens": "^3.0.0 || ^4.0.0"
+      }
+    },
+    "node_modules/lru-cache": {
+      "version": "6.0.0",
+      "resolved": "https://registry.npmjs.org/lru-cache/-/lru-cache-6.0.0.tgz",
+      "integrity": "sha512-Jo6dJ04CmSjuznwJSS3pUeWmd/H0ffTlkXXgwZi+eq1UCmqQwCh+eLsYOYCwY991i2Fah4h1BEMCx4qThGbsiA==",
+      "dev": true,
+      "dependencies": {
+        "yallist": "^4.0.0"
+      },
+      "engines": {
+        "node": ">=10"
+      }
+    },
+    "node_modules/merge2": {
+      "version": "1.4.1",
+      "resolved": "https://registry.npmjs.org/merge2/-/merge2-1.4.1.tgz",
+      "integrity": "sha512-8q7VEgMJW4J8tcfVPy8g09NcQwZdbwFEqhe/WZkoIzjn/3TGDwtOCYtXGxA3O8tPzpczCCDgv+P2P5y00ZJOOg==",
+      "dev": true,
+      "engines": {
+        "node": ">= 8"
+      }
+    },
+    "node_modules/micromatch": {
+      "version": "4.0.2",
+      "resolved": "https://registry.npmjs.org/micromatch/-/micromatch-4.0.2.tgz",
+      "integrity": "sha512-y7FpHSbMUMoyPbYUSzO6PaZ6FyRnQOpHuKwbo1G+Knck95XVU4QAiKdGEnj5wwoS7PlOgthX/09u5iFJ+aYf5Q==",
+      "dev": true,
+      "dependencies": {
+        "braces": "^3.0.1",
+        "picomatch": "^2.0.5"
+      },
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/minimatch": {
+      "version": "3.0.4",
+      "resolved": "https://registry.npmjs.org/minimatch/-/minimatch-3.0.4.tgz",
+      "integrity": "sha512-yJHVQEhyqPLUTgt9B83PXu6W3rx4MvvHvSUvToogpwoGDOUQ+yDrR0HRot+yOCdCO7u4hX3pWft6kWBBcqh0UA==",
+      "dev": true,
+      "dependencies": {
+        "brace-expansion": "^1.1.7"
+      }
+    },
+    "node_modules/minimist": {
+      "version": "1.2.5",
+      "resolved": "https://registry.npmjs.org/minimist/-/minimist-1.2.5.tgz",
+      "integrity": "sha512-FM9nNUYrRBAELZQT3xeZQ7fmMOBg6nWNmJKTcgsJeaLstP/UODVpGsr5OhXhhXg6f+qtJ8uiZ+PUxkDWcgIXLw==",
+      "dev": true
+    },
+    "node_modules/mkdirp": {
+      "version": "0.5.5",
+      "resolved": "https://registry.npmjs.org/mkdirp/-/mkdirp-0.5.5.tgz",
+      "integrity": "sha512-NKmAlESf6jMGym1++R0Ra7wvhV+wFW63FaSOFPwRahvea0gMUcGUhVeAg/0BC0wiv9ih5NYPB1Wn1UEI1/L+xQ==",
+      "dev": true,
+      "dependencies": {
+        "minimist": "^1.2.5"
+      }
+    },
+    "node_modules/mocha": {
+      "version": "8.2.1",
+      "resolved": "https://registry.npmjs.org/mocha/-/mocha-8.2.1.tgz",
+      "integrity": "sha512-cuLBVfyFfFqbNR0uUKbDGXKGk+UDFe6aR4os78XIrMQpZl/nv7JYHcvP5MFIAb374b2zFXsdgEGwmzMtP0Xg8w==",
+      "dev": true,
+      "dependencies": {
+        "@ungap/promise-all-settled": "1.1.2",
+        "ansi-colors": "4.1.1",
+        "browser-stdout": "1.3.1",
+        "chokidar": "3.4.3",
+        "debug": "4.2.0",
+        "diff": "4.0.2",
+        "escape-string-regexp": "4.0.0",
+        "find-up": "5.0.0",
+        "glob": "7.1.6",
+        "growl": "1.10.5",
+        "he": "1.2.0",
+        "js-yaml": "3.14.0",
+        "log-symbols": "4.0.0",
+        "minimatch": "3.0.4",
+        "ms": "2.1.2",
+        "nanoid": "3.1.12",
+        "serialize-javascript": "5.0.1",
+        "strip-json-comments": "3.1.1",
+        "supports-color": "7.2.0",
+        "which": "2.0.2",
+        "wide-align": "1.1.3",
+        "workerpool": "6.0.2",
+        "yargs": "13.3.2",
+        "yargs-parser": "13.1.2",
+        "yargs-unparser": "2.0.0"
+      },
+      "bin": {
+        "_mocha": "bin/_mocha",
+        "mocha": "bin/mocha"
+      },
+      "engines": {
+        "node": ">= 10.12.0"
+      },
+      "funding": {
+        "type": "opencollective",
+        "url": "https://opencollective.com/mochajs"
+      }
+    },
+    "node_modules/mocha/node_modules/escape-string-regexp": {
+      "version": "4.0.0",
+      "resolved": "https://registry.npmjs.org/escape-string-regexp/-/escape-string-regexp-4.0.0.tgz",
+      "integrity": "sha512-TtpcNJ3XAzx3Gq8sWRzJaVajRs0uVxA2YAkdb1jm2YkPz4G6egUFAyA3n5vtEIZefPk5Wa4UXbKuS5fKkJWdgA==",
+      "dev": true,
+      "engines": {
+        "node": ">=10"
+      },
+      "funding": {
+        "url": "https://github.com/sponsors/sindresorhus"
+      }
+    },
+    "node_modules/mocha/node_modules/find-up": {
+      "version": "5.0.0",
+      "resolved": "https://registry.npmjs.org/find-up/-/find-up-5.0.0.tgz",
+      "integrity": "sha512-78/PXT1wlLLDgTzDs7sjq9hzz0vXD+zn+7wypEe4fXQxCmdmqfGsEPQxmiCSQI3ajFV91bVSsvNtrJRiW6nGng==",
+      "dev": true,
+      "dependencies": {
+        "locate-path": "^6.0.0",
+        "path-exists": "^4.0.0"
+      },
+      "engines": {
+        "node": ">=10"
+      },
+      "funding": {
+        "url": "https://github.com/sponsors/sindresorhus"
+      }
+    },
+    "node_modules/mocha/node_modules/has-flag": {
+      "version": "4.0.0",
+      "resolved": "https://registry.npmjs.org/has-flag/-/has-flag-4.0.0.tgz",
+      "integrity": "sha512-EykJT/Q1KjTWctppgIAgfSO0tKVuZUjhgMr17kqTumMl6Afv3EISleU7qZUzoXDFTAHTDC4NOoG/ZxU3EvlMPQ==",
+      "dev": true,
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/mocha/node_modules/locate-path": {
+      "version": "6.0.0",
+      "resolved": "https://registry.npmjs.org/locate-path/-/locate-path-6.0.0.tgz",
+      "integrity": "sha512-iPZK6eYjbxRu3uB4/WZ3EsEIMJFMqAoopl3R+zuq0UjcAm/MO6KCweDgPfP3elTztoKP3KtnVHxTn2NHBSDVUw==",
+      "dev": true,
+      "dependencies": {
+        "p-locate": "^5.0.0"
+      },
+      "engines": {
+        "node": ">=10"
+      },
+      "funding": {
+        "url": "https://github.com/sponsors/sindresorhus"
+      }
+    },
+    "node_modules/mocha/node_modules/p-limit": {
+      "version": "3.1.0",
+      "resolved": "https://registry.npmjs.org/p-limit/-/p-limit-3.1.0.tgz",
+      "integrity": "sha512-TYOanM3wGwNGsZN2cVTYPArw454xnXj5qmWF1bEoAc4+cU/ol7GVh7odevjp1FNHduHc3KZMcFduxU5Xc6uJRQ==",
+      "dev": true,
+      "dependencies": {
+        "yocto-queue": "^0.1.0"
+      },
+      "engines": {
+        "node": ">=10"
+      },
+      "funding": {
+        "url": "https://github.com/sponsors/sindresorhus"
+      }
+    },
+    "node_modules/mocha/node_modules/p-locate": {
+      "version": "5.0.0",
+      "resolved": "https://registry.npmjs.org/p-locate/-/p-locate-5.0.0.tgz",
+      "integrity": "sha512-LaNjtRWUBY++zB5nE/NwcaoMylSPk+S+ZHNB1TzdbMJMny6dynpAGt7X/tl/QYq3TIeE6nxHppbo2LGymrG5Pw==",
+      "dev": true,
+      "dependencies": {
+        "p-limit": "^3.0.2"
+      },
+      "engines": {
+        "node": ">=10"
+      },
+      "funding": {
+        "url": "https://github.com/sponsors/sindresorhus"
+      }
+    },
+    "node_modules/mocha/node_modules/path-exists": {
+      "version": "4.0.0",
+      "resolved": "https://registry.npmjs.org/path-exists/-/path-exists-4.0.0.tgz",
+      "integrity": "sha512-ak9Qy5Q7jYb2Wwcey5Fpvg2KoAc/ZIhLSLOSBmRmygPsGwkVVt0fZa0qrtMz+m6tJTAHfZQ8FnmB4MG4LWy7/w==",
+      "dev": true,
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/mocha/node_modules/supports-color": {
+      "version": "7.2.0",
+      "resolved": "https://registry.npmjs.org/supports-color/-/supports-color-7.2.0.tgz",
+      "integrity": "sha512-qpCAvRl9stuOHveKsn7HncJRvv501qIacKzQlO/+Lwxc9+0q2wLyv4Dfvt80/DPn2pqOBsJdDiogXGR9+OvwRw==",
+      "dev": true,
+      "dependencies": {
+        "has-flag": "^4.0.0"
+      },
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/ms": {
+      "version": "2.1.2",
+      "resolved": "https://registry.npmjs.org/ms/-/ms-2.1.2.tgz",
+      "integrity": "sha512-sGkPx+VjMtmA6MX27oA4FBFELFCZZ4S4XqeGOXCv68tT+jb3vk/RyaKWP0PTKyWtmLSM0b+adUTEvbs1PEaH2w==",
+      "dev": true
+    },
+    "node_modules/multi-ini": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/multi-ini/-/multi-ini-2.1.0.tgz",
+      "integrity": "sha512-ANOg4WIoTXYfqyT2Y6FGb8YQyItatdJvfAZKAfFMWT1+D3vUstocjkUQ82EduAeK69rCR6a9k0fggxRhsFnAyQ==",
+      "dev": true,
+      "dependencies": {
+        "lodash": "^4.0.0"
+      }
+    },
+    "node_modules/nanoid": {
+      "version": "3.1.12",
+      "resolved": "https://registry.npmjs.org/nanoid/-/nanoid-3.1.12.tgz",
+      "integrity": "sha512-1qstj9z5+x491jfiC4Nelk+f8XBad7LN20PmyWINJEMRSf3wcAjAWysw1qaA8z6NSKe2sjq1hRSDpBH5paCb6A==",
+      "dev": true,
+      "bin": {
+        "nanoid": "bin/nanoid.cjs"
+      },
+      "engines": {
+        "node": "^10 || ^12 || >=13.7"
+      }
+    },
+    "node_modules/natural-compare": {
+      "version": "1.4.0",
+      "resolved": "https://registry.npmjs.org/natural-compare/-/natural-compare-1.4.0.tgz",
+      "integrity": "sha1-Sr6/7tdUHywnrPspvbvRXI1bpPc=",
+      "dev": true
+    },
+    "node_modules/normalize-package-data": {
+      "version": "2.5.0",
+      "resolved": "https://registry.npmjs.org/normalize-package-data/-/normalize-package-data-2.5.0.tgz",
+      "integrity": "sha512-/5CMN3T0R4XTj4DcGaexo+roZSdSFW/0AOOTROrjxzCG1wrWXEsGbRKevjlIL+ZDE4sZlJr5ED4YW0yqmkK+eA==",
+      "dev": true,
+      "dependencies": {
+        "hosted-git-info": "^2.1.4",
+        "resolve": "^1.10.0",
+        "semver": "2 || 3 || 4 || 5",
+        "validate-npm-package-license": "^3.0.1"
+      }
+    },
+    "node_modules/normalize-path": {
+      "version": "3.0.0",
+      "resolved": "https://registry.npmjs.org/normalize-path/-/normalize-path-3.0.0.tgz",
+      "integrity": "sha512-6eZs5Ls3WtCisHWp9S2GUy8dqkpGi4BVSz3GaqiE6ezub0512ESztXUwUB6C6IKbQkY2Pnb/mD4WYojCRwcwLA==",
+      "dev": true,
+      "engines": {
+        "node": ">=0.10.0"
+      }
+    },
+    "node_modules/object-assign": {
+      "version": "4.1.1",
+      "resolved": "https://registry.npmjs.org/object-assign/-/object-assign-4.1.1.tgz",
+      "integrity": "sha1-IQmtx5ZYh8/AXLvUQsrIv7s2CGM=",
+      "dev": true
+    },
+    "node_modules/object-inspect": {
+      "version": "1.8.0",
+      "resolved": "https://registry.npmjs.org/object-inspect/-/object-inspect-1.8.0.tgz",
+      "integrity": "sha512-jLdtEOB112fORuypAyl/50VRVIBIdVQOSUUGQHzJ4xBSbit81zRarz7GThkEFZy1RceYrWYcPcBFPQwHyAc1gA==",
+      "dev": true
+    },
+    "node_modules/object-keys": {
+      "version": "1.1.1",
+      "resolved": "https://registry.npmjs.org/object-keys/-/object-keys-1.1.1.tgz",
+      "integrity": "sha512-NuAESUOUMrlIXOfHKzD6bpPu3tYt3xvjNdRIQ+FeT0lNb4K8WR70CaDxhuNguS2XG+GjkyMwOzsN5ZktImfhLA==",
+      "dev": true
+    },
+    "node_modules/object.assign": {
+      "version": "4.1.0",
+      "resolved": "https://registry.npmjs.org/object.assign/-/object.assign-4.1.0.tgz",
+      "integrity": "sha512-exHJeq6kBKj58mqGyTQ9DFvrZC/eR6OwxzoM9YRoGBqrXYonaFyGiFMuc9VZrXf7DarreEwMpurG3dd+CNyW5w==",
+      "dev": true,
+      "dependencies": {
+        "define-properties": "^1.1.2",
+        "function-bind": "^1.1.1",
+        "has-symbols": "^1.0.0",
+        "object-keys": "^1.0.11"
+      }
+    },
+    "node_modules/object.entries": {
+      "version": "1.1.2",
+      "resolved": "https://registry.npmjs.org/object.entries/-/object.entries-1.1.2.tgz",
+      "integrity": "sha512-BQdB9qKmb/HyNdMNWVr7O3+z5MUIx3aiegEIJqjMBbBf0YT9RRxTJSim4mzFqtyr7PDAHigq0N9dO0m0tRakQA==",
+      "dev": true,
+      "dependencies": {
+        "define-properties": "^1.1.3",
+        "es-abstract": "^1.17.5",
+        "has": "^1.0.3"
+      }
+    },
+    "node_modules/object.fromentries": {
+      "version": "2.0.2",
+      "resolved": "https://registry.npmjs.org/object.fromentries/-/object.fromentries-2.0.2.tgz",
+      "integrity": "sha512-r3ZiBH7MQppDJVLx6fhD618GKNG40CZYH9wgwdhKxBDDbQgjeWGGd4AtkZad84d291YxvWe7bJGuE65Anh0dxQ==",
+      "dev": true,
+      "dependencies": {
+        "define-properties": "^1.1.3",
+        "es-abstract": "^1.17.0-next.1",
+        "function-bind": "^1.1.1",
+        "has": "^1.0.3"
+      }
+    },
+    "node_modules/object.values": {
+      "version": "1.1.1",
+      "resolved": "https://registry.npmjs.org/object.values/-/object.values-1.1.1.tgz",
+      "integrity": "sha512-WTa54g2K8iu0kmS/us18jEmdv1a4Wi//BZ/DTVYEcH0XhLM5NYdpDHja3gt57VrZLcNAO2WGA+KpWsDBaHt6eA==",
+      "dev": true,
+      "dependencies": {
+        "define-properties": "^1.1.3",
+        "es-abstract": "^1.17.0-next.1",
+        "function-bind": "^1.1.1",
+        "has": "^1.0.3"
+      }
+    },
+    "node_modules/once": {
+      "version": "1.4.0",
+      "resolved": "https://registry.npmjs.org/once/-/once-1.4.0.tgz",
+      "integrity": "sha1-WDsap3WWHUsROsF9nFC6753Xa9E=",
+      "dev": true,
+      "dependencies": {
+        "wrappy": "1"
+      }
+    },
+    "node_modules/optionator": {
+      "version": "0.9.1",
+      "resolved": "https://registry.npmjs.org/optionator/-/optionator-0.9.1.tgz",
+      "integrity": "sha512-74RlY5FCnhq4jRxVUPKDaRwrVNXMqsGsiW6AJw4XK8hmtm10wC0ypZBLw5IIp85NZMr91+qd1RvvENwg7jjRFw==",
+      "dev": true,
+      "dependencies": {
+        "deep-is": "^0.1.3",
+        "fast-levenshtein": "^2.0.6",
+        "levn": "^0.4.1",
+        "prelude-ls": "^1.2.1",
+        "type-check": "^0.4.0",
+        "word-wrap": "^1.2.3"
+      }
+    },
+    "node_modules/p-limit": {
+      "version": "1.3.0",
+      "resolved": "https://registry.npmjs.org/p-limit/-/p-limit-1.3.0.tgz",
+      "integrity": "sha512-vvcXsLAJ9Dr5rQOPk7toZQZJApBl2K4J6dANSsEuh6QI41JYcsS/qhTGa9ErIUUgK3WNQoJYvylxvjqmiqEA9Q==",
+      "dev": true,
+      "dependencies": {
+        "p-try": "^1.0.0"
+      }
+    },
+    "node_modules/p-locate": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/p-locate/-/p-locate-2.0.0.tgz",
+      "integrity": "sha1-IKAQOyIqcMj9OcwuWAaA893l7EM=",
+      "dev": true,
+      "dependencies": {
+        "p-limit": "^1.1.0"
+      }
+    },
+    "node_modules/p-try": {
+      "version": "1.0.0",
+      "resolved": "https://registry.npmjs.org/p-try/-/p-try-1.0.0.tgz",
+      "integrity": "sha1-y8ec26+P1CKOE/Yh8rGiN8GyB7M=",
+      "dev": true
+    },
+    "node_modules/parent-module": {
+      "version": "1.0.1",
+      "resolved": "https://registry.npmjs.org/parent-module/-/parent-module-1.0.1.tgz",
+      "integrity": "sha512-GQ2EWRpQV8/o+Aw8YqtfZZPfNRWZYkbidE9k5rpl/hC3vtHHBfGm2Ifi6qWV+coDGkrUKZAxE3Lot5kcsRlh+g==",
+      "dev": true,
+      "dependencies": {
+        "callsites": "^3.0.0"
+      }
+    },
+    "node_modules/parse-json": {
+      "version": "2.2.0",
+      "resolved": "https://registry.npmjs.org/parse-json/-/parse-json-2.2.0.tgz",
+      "integrity": "sha1-9ID0BDTvgHQfhGkJn43qGPVaTck=",
+      "dev": true,
+      "dependencies": {
+        "error-ex": "^1.2.0"
+      }
+    },
+    "node_modules/path-exists": {
+      "version": "3.0.0",
+      "resolved": "https://registry.npmjs.org/path-exists/-/path-exists-3.0.0.tgz",
+      "integrity": "sha1-zg6+ql94yxiSXqfYENe1mwEP1RU=",
+      "dev": true
+    },
+    "node_modules/path-is-absolute": {
+      "version": "1.0.1",
+      "resolved": "https://registry.npmjs.org/path-is-absolute/-/path-is-absolute-1.0.1.tgz",
+      "integrity": "sha1-F0uSaHNVNP+8es5r9TpanhtcX18=",
+      "dev": true
+    },
+    "node_modules/path-key": {
+      "version": "3.1.1",
+      "resolved": "https://registry.npmjs.org/path-key/-/path-key-3.1.1.tgz",
+      "integrity": "sha512-ojmeN0qd+y0jszEtoY48r0Peq5dwMEkIlCOu6Q5f41lfkswXuKtYrhgoTpLnyIcHm24Uhqx+5Tqm2InSwLhE6Q==",
+      "dev": true
+    },
+    "node_modules/path-parse": {
+      "version": "1.0.6",
+      "resolved": "https://registry.npmjs.org/path-parse/-/path-parse-1.0.6.tgz",
+      "integrity": "sha512-GSmOT2EbHrINBf9SR7CDELwlJ8AENk3Qn7OikK4nFYAu3Ote2+JYNVvkpAEQm3/TLNEJFD/xZJjzyxg3KBWOzw==",
+      "dev": true
+    },
+    "node_modules/path-type": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/path-type/-/path-type-2.0.0.tgz",
+      "integrity": "sha1-8BLMuEFbcJb8LaoQVMPXI4lZTHM=",
+      "dev": true,
+      "dependencies": {
+        "pify": "^2.0.0"
+      }
+    },
+    "node_modules/picomatch": {
+      "version": "2.2.2",
+      "resolved": "https://registry.npmjs.org/picomatch/-/picomatch-2.2.2.tgz",
+      "integrity": "sha512-q0M/9eZHzmr0AulXyPwNfZjtwZ/RBZlbN3K3CErVrk50T2ASYI7Bye0EvekFY3IP1Nt2DHu0re+V2ZHIpMkuWg==",
+      "dev": true,
+      "engines": {
+        "node": ">=8.6"
+      },
+      "funding": {
+        "url": "https://github.com/sponsors/jonschlinkert"
+      }
+    },
+    "node_modules/pify": {
+      "version": "2.3.0",
+      "resolved": "https://registry.npmjs.org/pify/-/pify-2.3.0.tgz",
+      "integrity": "sha1-7RQaasBDqEnqWISY59yosVMw6Qw=",
+      "dev": true
+    },
+    "node_modules/pkg-dir": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/pkg-dir/-/pkg-dir-2.0.0.tgz",
+      "integrity": "sha1-9tXREJ4Z1j7fQo4L1X4Sd3YVM0s=",
+      "dev": true,
+      "dependencies": {
+        "find-up": "^2.1.0"
+      }
+    },
+    "node_modules/prelude-ls": {
+      "version": "1.2.1",
+      "resolved": "https://registry.npmjs.org/prelude-ls/-/prelude-ls-1.2.1.tgz",
+      "integrity": "sha512-vkcDPrRZo1QZLbn5RLGPpg/WmIQ65qoWWhcGKf/b5eplkkarX0m9z8ppCat4mlOqUsWpyNuYgO3VRyrYHSzX5g==",
+      "dev": true
+    },
+    "node_modules/prettier": {
+      "version": "1.19.1",
+      "resolved": "https://registry.npmjs.org/prettier/-/prettier-1.19.1.tgz",
+      "integrity": "sha512-s7PoyDv/II1ObgQunCbB9PdLmUcBZcnWOcxDh7O0N/UwDEsHyqkW+Qh28jW+mVuCdx7gLB0BotYI1Y6uI9iyew==",
+      "dev": true
+    },
+    "node_modules/prettier-linter-helpers": {
+      "version": "1.0.0",
+      "resolved": "https://registry.npmjs.org/prettier-linter-helpers/-/prettier-linter-helpers-1.0.0.tgz",
+      "integrity": "sha512-GbK2cP9nraSSUF9N2XwUwqfzlAFlMNYYl+ShE/V+H8a9uNl/oUqB1w2EL54Jh0OlyRSd8RfWYJ3coVS4TROP2w==",
+      "dev": true,
+      "dependencies": {
+        "fast-diff": "^1.1.2"
+      }
+    },
+    "node_modules/progress": {
+      "version": "2.0.3",
+      "resolved": "https://registry.npmjs.org/progress/-/progress-2.0.3.tgz",
+      "integrity": "sha512-7PiHtLll5LdnKIMw100I+8xJXR5gW2QwWYkT6iJva0bXitZKa/XMrSbdmg3r2Xnaidz9Qumd0VPaMrZlF9V9sA==",
+      "dev": true
+    },
+    "node_modules/prop-types": {
+      "version": "15.7.2",
+      "resolved": "https://registry.npmjs.org/prop-types/-/prop-types-15.7.2.tgz",
+      "integrity": "sha512-8QQikdH7//R2vurIJSutZ1smHYTcLpRWEOlHnzcWHmBYrOGUysKwSsrC89BCiFj3CbrfJ/nXFdJepOVrY1GCHQ==",
+      "dev": true,
+      "dependencies": {
+        "loose-envify": "^1.4.0",
+        "object-assign": "^4.1.1",
+        "react-is": "^16.8.1"
+      }
+    },
+    "node_modules/punycode": {
+      "version": "2.1.1",
+      "resolved": "https://registry.npmjs.org/punycode/-/punycode-2.1.1.tgz",
+      "integrity": "sha512-XRsRjdf+j5ml+y/6GKHPZbrF/8p2Yga0JPtdqTIY2Xe5ohJPD9saDJJLPvp9+NSBprVvevdXZybnj2cv8OEd0A==",
+      "dev": true
+    },
+    "node_modules/randombytes": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/randombytes/-/randombytes-2.1.0.tgz",
+      "integrity": "sha512-vYl3iOX+4CKUWuxGi9Ukhie6fsqXqS9FE2Zaic4tNFD2N2QQaXOMFbuKK4QmDHC0JO6B1Zp41J0LpT0oR68amQ==",
+      "dev": true,
+      "dependencies": {
+        "safe-buffer": "^5.1.0"
+      }
+    },
+    "node_modules/react-is": {
+      "version": "16.13.1",
+      "resolved": "https://registry.npmjs.org/react-is/-/react-is-16.13.1.tgz",
+      "integrity": "sha512-24e6ynE2H+OKt4kqsOvNd8kBpV65zoxbA4BVsEOB3ARVWQki/DHzaUoC5KuON/BiccDaCCTZBuOcfZs70kR8bQ==",
+      "dev": true
+    },
+    "node_modules/read-pkg": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/read-pkg/-/read-pkg-2.0.0.tgz",
+      "integrity": "sha1-jvHAYjxqbbDcZxPEv6xGMysjaPg=",
+      "dev": true,
+      "dependencies": {
+        "load-json-file": "^2.0.0",
+        "normalize-package-data": "^2.3.2",
+        "path-type": "^2.0.0"
+      }
+    },
+    "node_modules/read-pkg-up": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/read-pkg-up/-/read-pkg-up-2.0.0.tgz",
+      "integrity": "sha1-a3KoBImE4MQeeVEP1en6mbO1Sb4=",
+      "dev": true,
+      "dependencies": {
+        "find-up": "^2.0.0",
+        "read-pkg": "^2.0.0"
+      }
+    },
+    "node_modules/readable-stream": {
+      "version": "3.6.0",
+      "resolved": "https://registry.npmjs.org/readable-stream/-/readable-stream-3.6.0.tgz",
+      "integrity": "sha512-BViHy7LKeTz4oNnkcLJ+lVSL6vpiFeX6/d3oSH8zCW7UxP2onchk+vTGB143xuFjHS3deTgkKoXXymXqymiIdA==",
+      "dev": true,
+      "dependencies": {
+        "inherits": "^2.0.3",
+        "string_decoder": "^1.1.1",
+        "util-deprecate": "^1.0.1"
+      }
+    },
+    "node_modules/readdirp": {
+      "version": "3.5.0",
+      "resolved": "https://registry.npmjs.org/readdirp/-/readdirp-3.5.0.tgz",
+      "integrity": "sha512-cMhu7c/8rdhkHXWsY+osBhfSy0JikwpHK/5+imo+LpeasTF8ouErHrlYkwT0++njiyuDvc7OFY5T3ukvZ8qmFQ==",
+      "dev": true,
+      "dependencies": {
+        "picomatch": "^2.2.1"
+      },
+      "engines": {
+        "node": ">=8.10.0"
+      }
+    },
+    "node_modules/regenerator-runtime": {
+      "version": "0.13.7",
+      "resolved": "https://registry.npmjs.org/regenerator-runtime/-/regenerator-runtime-0.13.7.tgz",
+      "integrity": "sha512-a54FxoJDIr27pgf7IgeQGxmqUNYrcV338lf/6gH456HZ/PhX+5BcwHXG9ajESmwe6WRO0tAzRUrRmNONWgkrew==",
+      "dev": true
+    },
+    "node_modules/regexp.prototype.flags": {
+      "version": "1.3.0",
+      "resolved": "https://registry.npmjs.org/regexp.prototype.flags/-/regexp.prototype.flags-1.3.0.tgz",
+      "integrity": "sha512-2+Q0C5g951OlYlJz6yu5/M33IcsESLlLfsyIaLJaG4FA2r4yP8MvVMJUUP/fVBkSpbbbZlS5gynbEWLipiiXiQ==",
+      "dev": true,
+      "dependencies": {
+        "define-properties": "^1.1.3",
+        "es-abstract": "^1.17.0-next.1"
+      }
+    },
+    "node_modules/regexpp": {
+      "version": "3.1.0",
+      "resolved": "https://registry.npmjs.org/regexpp/-/regexpp-3.1.0.tgz",
+      "integrity": "sha512-ZOIzd8yVsQQA7j8GCSlPGXwg5PfmA1mrq0JP4nGhh54LaKN3xdai/vHUDu74pKwV8OxseMS65u2NImosQcSD0Q==",
+      "dev": true
+    },
+    "node_modules/require-directory": {
+      "version": "2.1.1",
+      "resolved": "https://registry.npmjs.org/require-directory/-/require-directory-2.1.1.tgz",
+      "integrity": "sha1-jGStX9MNqxyXbiNE/+f3kqam30I=",
+      "dev": true,
+      "engines": {
+        "node": ">=0.10.0"
+      }
+    },
+    "node_modules/require-main-filename": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/require-main-filename/-/require-main-filename-2.0.0.tgz",
+      "integrity": "sha512-NKN5kMDylKuldxYLSUfrbo5Tuzh4hd+2E8NPPX02mZtn1VuREQToYe/ZdlJy+J3uCpfaiGF05e7B8W0iXbQHmg==",
+      "dev": true
+    },
+    "node_modules/resolve": {
+      "version": "1.17.0",
+      "resolved": "https://registry.npmjs.org/resolve/-/resolve-1.17.0.tgz",
+      "integrity": "sha512-ic+7JYiV8Vi2yzQGFWOkiZD5Z9z7O2Zhm9XMaTxdJExKasieFCr+yXZ/WmXsckHiKl12ar0y6XiXDx3m4RHn1w==",
+      "dev": true,
+      "dependencies": {
+        "path-parse": "^1.0.6"
+      }
+    },
+    "node_modules/resolve-from": {
+      "version": "4.0.0",
+      "resolved": "https://registry.npmjs.org/resolve-from/-/resolve-from-4.0.0.tgz",
+      "integrity": "sha512-pb/MYmXstAkysRFx8piNI1tGFNQIFA3vkE3Gq4EuA1dF6gHp/+vgZqsCGJapvy8N3Q+4o7FwvquPJcnZ7RYy4g==",
+      "dev": true
+    },
+    "node_modules/reusify": {
+      "version": "1.0.4",
+      "resolved": "https://registry.npmjs.org/reusify/-/reusify-1.0.4.tgz",
+      "integrity": "sha512-U9nH88a3fc/ekCF1l0/UP1IosiuIjyTh7hBvXVMHYgVcfGvt897Xguj2UOLDeI5BG2m7/uwyaLVT6fbtCwTyzw==",
+      "dev": true,
+      "engines": {
+        "iojs": ">=1.0.0",
+        "node": ">=0.10.0"
+      }
+    },
+    "node_modules/rimraf": {
+      "version": "2.6.3",
+      "resolved": "https://registry.npmjs.org/rimraf/-/rimraf-2.6.3.tgz",
+      "integrity": "sha512-mwqeW5XsA2qAejG46gYdENaxXjx9onRNCfn7L0duuP4hCuTIi/QO7PDK07KJfp1d+izWPrzEJDcSqBa0OZQriA==",
+      "dev": true,
+      "dependencies": {
+        "glob": "^7.1.3"
+      }
+    },
+    "node_modules/run-parallel": {
+      "version": "1.1.10",
+      "resolved": "https://registry.npmjs.org/run-parallel/-/run-parallel-1.1.10.tgz",
+      "integrity": "sha512-zb/1OuZ6flOlH6tQyMPUrE3x3Ulxjlo9WIVXR4yVYi4H9UXQaeIsPbLn2R3O3vQCnDKkAl2qHiuocKKX4Tz/Sw==",
+      "dev": true,
+      "funding": [
+        {
+          "type": "github",
+          "url": "https://github.com/sponsors/feross"
+        },
+        {
+          "type": "patreon",
+          "url": "https://www.patreon.com/feross"
+        },
+        {
+          "type": "consulting",
+          "url": "https://feross.org/support"
+        }
+      ]
+    },
+    "node_modules/safe-buffer": {
+      "version": "5.1.2",
+      "resolved": "https://registry.npmjs.org/safe-buffer/-/safe-buffer-5.1.2.tgz",
+      "integrity": "sha512-Gd2UZBJDkXlY7GbJxfsE8/nvKkUEU1G38c1siN6QP6a9PT9MmHB8GnpscSmMJSoF8LOIrt8ud/wPtojys4G6+g==",
+      "dev": true
+    },
+    "node_modules/sax": {
+      "version": "1.2.4",
+      "resolved": "https://registry.npmjs.org/sax/-/sax-1.2.4.tgz",
+      "integrity": "sha512-NqVDv9TpANUjFm0N8uM5GxL36UgKi9/atZw+x7YFnQ8ckwFGKrl4xX4yWtrey3UJm5nP1kUbnYgLopqWNSRhWw==",
+      "dev": true
+    },
+    "node_modules/semver": {
+      "version": "5.7.1",
+      "resolved": "https://registry.npmjs.org/semver/-/semver-5.7.1.tgz",
+      "integrity": "sha512-sauaDf/PZdVgrLTNYHRtpXa1iRiKcaebiKQ1BJdpQlWH2lCvexQdX55snPFyK7QzpudqbCI0qXFfOasHdyNDGQ==",
+      "dev": true
+    },
+    "node_modules/serialize-javascript": {
+      "version": "5.0.1",
+      "resolved": "https://registry.npmjs.org/serialize-javascript/-/serialize-javascript-5.0.1.tgz",
+      "integrity": "sha512-SaaNal9imEO737H2c05Og0/8LUXG7EnsZyMa8MzkmuHoELfT6txuj0cMqRj6zfPKnmQ1yasR4PCJc8x+M4JSPA==",
+      "dev": true,
+      "dependencies": {
+        "randombytes": "^2.1.0"
+      }
+    },
+    "node_modules/set-blocking": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/set-blocking/-/set-blocking-2.0.0.tgz",
+      "integrity": "sha1-BF+XgtARrppoA93TgrJDkrPYkPc=",
+      "dev": true
+    },
+    "node_modules/shebang-command": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/shebang-command/-/shebang-command-2.0.0.tgz",
+      "integrity": "sha512-kHxr2zZpYtdmrN1qDjrrX/Z1rR1kG8Dx+gkpK1G4eXmvXswmcE1hTWBWYUzlraYw1/yZp6YuDY77YtvbN0dmDA==",
+      "dev": true,
+      "dependencies": {
+        "shebang-regex": "^3.0.0"
+      }
+    },
+    "node_modules/shebang-regex": {
+      "version": "3.0.0",
+      "resolved": "https://registry.npmjs.org/shebang-regex/-/shebang-regex-3.0.0.tgz",
+      "integrity": "sha512-7++dFhtcx3353uBaq8DDR4NuxBetBzC7ZQOhmTQInHEd6bSrXdiEyzCvG07Z44UYdLShWUyXt5M/yhz8ekcb1A==",
+      "dev": true
+    },
+    "node_modules/side-channel": {
+      "version": "1.0.2",
+      "resolved": "https://registry.npmjs.org/side-channel/-/side-channel-1.0.2.tgz",
+      "integrity": "sha512-7rL9YlPHg7Ancea1S96Pa8/QWb4BtXL/TZvS6B8XFetGBeuhAsfmUspK6DokBeZ64+Kj9TCNRD/30pVz1BvQNA==",
+      "dev": true,
+      "dependencies": {
+        "es-abstract": "^1.17.0-next.1",
+        "object-inspect": "^1.7.0"
+      }
+    },
+    "node_modules/slash": {
+      "version": "3.0.0",
+      "resolved": "https://registry.npmjs.org/slash/-/slash-3.0.0.tgz",
+      "integrity": "sha512-g9Q1haeby36OSStwb4ntCGGGaKsaVSjQ68fBxoQcutl5fS1vuY18H3wSt3jFyFtrkx+Kz0V1G85A4MyAdDMi2Q==",
+      "dev": true,
+      "engines": {
+        "node": ">=8"
+      }
+    },
+    "node_modules/slice-ansi": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/slice-ansi/-/slice-ansi-2.1.0.tgz",
+      "integrity": "sha512-Qu+VC3EwYLldKa1fCxuuvULvSJOKEgk9pi8dZeCVK7TqBfUNTH4sFkk4joj8afVSfAYgJoSOetjx9QWOJ5mYoQ==",
+      "dev": true,
+      "dependencies": {
+        "ansi-styles": "^3.2.0",
+        "astral-regex": "^1.0.0",
+        "is-fullwidth-code-point": "^2.0.0"
+      }
+    },
+    "node_modules/source-map": {
+      "version": "0.5.7",
+      "resolved": "https://registry.npmjs.org/source-map/-/source-map-0.5.7.tgz",
+      "integrity": "sha1-igOdLRAh0i0eoUyA2OpGi6LvP8w=",
+      "dev": true
+    },
+    "node_modules/spdx-correct": {
+      "version": "3.1.1",
+      "resolved": "https://registry.npmjs.org/spdx-correct/-/spdx-correct-3.1.1.tgz",
+      "integrity": "sha512-cOYcUWwhCuHCXi49RhFRCyJEK3iPj1Ziz9DpViV3tbZOwXD49QzIN3MpOLJNxh2qwq2lJJZaKMVw9qNi4jTC0w==",
+      "dev": true,
+      "dependencies": {
+        "spdx-expression-parse": "^3.0.0",
+        "spdx-license-ids": "^3.0.0"
+      }
+    },
+    "node_modules/spdx-exceptions": {
+      "version": "2.3.0",
+      "resolved": "https://registry.npmjs.org/spdx-exceptions/-/spdx-exceptions-2.3.0.tgz",
+      "integrity": "sha512-/tTrYOC7PPI1nUAgx34hUpqXuyJG+DTHJTnIULG4rDygi4xu/tfgmq1e1cIRwRzwZgo4NLySi+ricLkZkw4i5A==",
+      "dev": true
+    },
+    "node_modules/spdx-expression-parse": {
+      "version": "3.0.1",
+      "resolved": "https://registry.npmjs.org/spdx-expression-parse/-/spdx-expression-parse-3.0.1.tgz",
+      "integrity": "sha512-cbqHunsQWnJNE6KhVSMsMeH5H/L9EpymbzqTQ3uLwNCLZ1Q481oWaofqH7nO6V07xlXwY6PhQdQ2IedWx/ZK4Q==",
+      "dev": true,
+      "dependencies": {
+        "spdx-exceptions": "^2.1.0",
+        "spdx-license-ids": "^3.0.0"
+      }
+    },
+    "node_modules/spdx-license-ids": {
+      "version": "3.0.5",
+      "resolved": "https://registry.npmjs.org/spdx-license-ids/-/spdx-license-ids-3.0.5.tgz",
+      "integrity": "sha512-J+FWzZoynJEXGphVIS+XEh3kFSjZX/1i9gFBaWQcB+/tmpe2qUsSBABpcxqxnAxFdiUFEgAX1bjYGQvIZmoz9Q==",
+      "dev": true
+    },
+    "node_modules/sprintf-js": {
+      "version": "1.0.3",
+      "resolved": "https://registry.npmjs.org/sprintf-js/-/sprintf-js-1.0.3.tgz",
+      "integrity": "sha1-BOaSb2YolTVPPdAVIDYzuFcpfiw=",
+      "dev": true
+    },
+    "node_modules/string_decoder": {
+      "version": "1.3.0",
+      "resolved": "https://registry.npmjs.org/string_decoder/-/string_decoder-1.3.0.tgz",
+      "integrity": "sha512-hkRX8U1WjJFd8LsDJ2yQ/wWWxaopEsABU1XfkM8A+j0+85JAGppt16cr1Whg6KIbb4okU6Mql6BOj+uup/wKeA==",
+      "dev": true,
+      "dependencies": {
+        "safe-buffer": "~5.2.0"
+      }
+    },
+    "node_modules/string_decoder/node_modules/safe-buffer": {
+      "version": "5.2.1",
+      "resolved": "https://registry.npmjs.org/safe-buffer/-/safe-buffer-5.2.1.tgz",
+      "integrity": "sha512-rp3So07KcdmmKbGvgaNxQSJr7bGVSVk5S9Eq1F+ppbRo70+YeaDxkw5Dd8NPN+GD6bjnYm2VuPuCXmpuYvmCXQ==",
+      "dev": true
+    },
+    "node_modules/string-width": {
+      "version": "3.1.0",
+      "resolved": "https://registry.npmjs.org/string-width/-/string-width-3.1.0.tgz",
+      "integrity": "sha512-vafcv6KjVZKSgz06oM/H6GDBrAtz8vdhQakGjFIvNrHA6y3HCF1CInLy+QLq8dTJPQ1b+KDUqDFctkdRW44e1w==",
+      "dev": true,
+      "dependencies": {
+        "emoji-regex": "^7.0.1",
+        "is-fullwidth-code-point": "^2.0.0",
+        "strip-ansi": "^5.1.0"
+      }
+    },
+    "node_modules/string-width/node_modules/ansi-regex": {
+      "version": "4.1.0",
+      "resolved": "https://registry.npmjs.org/ansi-regex/-/ansi-regex-4.1.0.tgz",
+      "integrity": "sha512-1apePfXM1UOSqw0o9IiFAovVz9M5S1Dg+4TrDwfMewQ6p/rmMueb7tWZjQ1rx4Loy1ArBggoqGpfqqdI4rondg==",
+      "dev": true
+    },
+    "node_modules/string-width/node_modules/strip-ansi": {
+      "version": "5.2.0",
+      "resolved": "https://registry.npmjs.org/strip-ansi/-/strip-ansi-5.2.0.tgz",
+      "integrity": "sha512-DuRs1gKbBqsMKIZlrffwlug8MHkcnpjs5VPmL1PAh+mA30U0DTotfDZ0d2UUsXpPmPmMMJ6W773MaA3J+lbiWA==",
+      "dev": true,
+      "dependencies": {
+        "ansi-regex": "^4.1.0"
+      }
+    },
+    "node_modules/string.prototype.matchall": {
+      "version": "4.0.2",
+      "resolved": "https://registry.npmjs.org/string.prototype.matchall/-/string.prototype.matchall-4.0.2.tgz",
+      "integrity": "sha512-N/jp6O5fMf9os0JU3E72Qhf590RSRZU/ungsL/qJUYVTNv7hTG0P/dbPjxINVN9jpscu3nzYwKESU3P3RY5tOg==",
+      "dev": true,
+      "dependencies": {
+        "define-properties": "^1.1.3",
+        "es-abstract": "^1.17.0",
+        "has-symbols": "^1.0.1",
+        "internal-slot": "^1.0.2",
+        "regexp.prototype.flags": "^1.3.0",
+        "side-channel": "^1.0.2"
+      }
+    },
+    "node_modules/string.prototype.trimend": {
+      "version": "1.0.1",
+      "resolved": "https://registry.npmjs.org/string.prototype.trimend/-/string.prototype.trimend-1.0.1.tgz",
+      "integrity": "sha512-LRPxFUaTtpqYsTeNKaFOw3R4bxIzWOnbQ837QfBylo8jIxtcbK/A/sMV7Q+OAV/vWo+7s25pOE10KYSjaSO06g==",
+      "dev": true,
+      "dependencies": {
+        "define-properties": "^1.1.3",
+        "es-abstract": "^1.17.5"
+      }
+    },
+    "node_modules/string.prototype.trimstart": {
+      "version": "1.0.1",
+      "resolved": "https://registry.npmjs.org/string.prototype.trimstart/-/string.prototype.trimstart-1.0.1.tgz",
+      "integrity": "sha512-XxZn+QpvrBI1FOcg6dIpxUPgWCPuNXvMD72aaRaUQv1eD4e/Qy8i/hFTe0BUmD60p/QA6bh1avmuPTfNjqVWRw==",
+      "dev": true,
+      "dependencies": {
+        "define-properties": "^1.1.3",
+        "es-abstract": "^1.17.5"
+      }
+    },
+    "node_modules/strip-ansi": {
+      "version": "6.0.0",
+      "resolved": "https://registry.npmjs.org/strip-ansi/-/strip-ansi-6.0.0.tgz",
+      "integrity": "sha512-AuvKTrTfQNYNIctbR1K/YGTR1756GycPsg7b9bdV9Duqur4gv6aKqHXah67Z8ImS7WEz5QVcOtlfW2rZEugt6w==",
+      "dev": true,
+      "dependencies": {
+        "ansi-regex": "^5.0.0"
+      }
+    },
+    "node_modules/strip-bom": {
+      "version": "3.0.0",
+      "resolved": "https://registry.npmjs.org/strip-bom/-/strip-bom-3.0.0.tgz",
+      "integrity": "sha1-IzTBjpx1n3vdVv3vfprj1YjmjtM=",
+      "dev": true
+    },
+    "node_modules/strip-json-comments": {
+      "version": "3.1.1",
+      "resolved": "https://registry.npmjs.org/strip-json-comments/-/strip-json-comments-3.1.1.tgz",
+      "integrity": "sha512-6fPc+R4ihwqP6N/aIv2f1gMH8lOVtWQHoqC4yK6oSDVVocumAsfCqjkXnqiYMhmMwS/mEHLp7Vehlt3ql6lEig==",
+      "dev": true
+    },
+    "node_modules/supports-color": {
+      "version": "5.5.0",
+      "resolved": "https://registry.npmjs.org/supports-color/-/supports-color-5.5.0.tgz",
+      "integrity": "sha512-QjVjwdXIt408MIiAqCX4oUKsgU2EqAGzs2Ppkm4aQYbjm+ZEWEcW4SfFNTr4uMNZma0ey4f5lgLrkB0aX0QMow==",
+      "dev": true,
+      "dependencies": {
+        "has-flag": "^3.0.0"
+      }
+    },
+    "node_modules/table": {
+      "version": "5.4.6",
+      "resolved": "https://registry.npmjs.org/table/-/table-5.4.6.tgz",
+      "integrity": "sha512-wmEc8m4fjnob4gt5riFRtTu/6+4rSe12TpAELNSqHMfF3IqnA+CH37USM6/YR3qRZv7e56kAEAtd6nKZaxe0Ug==",
+      "dev": true,
+      "dependencies": {
+        "ajv": "^6.10.2",
+        "lodash": "^4.17.14",
+        "slice-ansi": "^2.1.0",
+        "string-width": "^3.0.0"
+      }
+    },
+    "node_modules/text-table": {
+      "version": "0.2.0",
+      "resolved": "https://registry.npmjs.org/text-table/-/text-table-0.2.0.tgz",
+      "integrity": "sha1-f17oI66AUgfACvLfSoTsP8+lcLQ=",
+      "dev": true
+    },
+    "node_modules/to-fast-properties": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/to-fast-properties/-/to-fast-properties-2.0.0.tgz",
+      "integrity": "sha1-3F5pjL0HkmW8c+A3doGk5Og/YW4=",
+      "dev": true
+    },
+    "node_modules/to-regex-range": {
+      "version": "5.0.1",
+      "resolved": "https://registry.npmjs.org/to-regex-range/-/to-regex-range-5.0.1.tgz",
+      "integrity": "sha512-65P7iz6X5yEr1cwcgvQxbbIw7Uk3gOy5dIdtZ4rDveLqhrdJP+Li/Hx6tyK0NEb+2GCyneCMJiGqrADCSNk8sQ==",
+      "dev": true,
+      "dependencies": {
+        "is-number": "^7.0.0"
+      },
+      "engines": {
+        "node": ">=8.0"
+      }
+    },
+    "node_modules/tslib": {
+      "version": "1.13.0",
+      "resolved": "https://registry.npmjs.org/tslib/-/tslib-1.13.0.tgz",
+      "integrity": "sha512-i/6DQjL8Xf3be4K/E6Wgpekn5Qasl1usyw++dAA35Ue5orEn65VIxOA+YvNNl9HV3qv70T7CNwjODHZrLwvd1Q==",
+      "dev": true
+    },
+    "node_modules/tsutils": {
+      "version": "3.17.1",
+      "resolved": "https://registry.npmjs.org/tsutils/-/tsutils-3.17.1.tgz",
+      "integrity": "sha512-kzeQ5B8H3w60nFY2g8cJIuH7JDpsALXySGtwGJ0p2LSjLgay3NdIpqq5SoOBe46bKDW2iq25irHCr8wjomUS2g==",
+      "dev": true,
+      "dependencies": {
+        "tslib": "^1.8.1"
+      }
+    },
+    "node_modules/type-check": {
+      "version": "0.4.0",
+      "resolved": "https://registry.npmjs.org/type-check/-/type-check-0.4.0.tgz",
+      "integrity": "sha512-XleUoc9uwGXqjWwXaUTZAmzMcFZ5858QA2vvx1Ur5xIcixXIP+8LnFDgRplU30us6teqdlskFfu+ae4K79Ooew==",
+      "dev": true,
+      "dependencies": {
+        "prelude-ls": "^1.2.1"
+      }
+    },
+    "node_modules/type-fest": {
+      "version": "0.8.1",
+      "resolved": "https://registry.npmjs.org/type-fest/-/type-fest-0.8.1.tgz",
+      "integrity": "sha512-4dbzIzqvjtgiM5rw1k5rEHtBANKmdudhGyBEajN01fEyhaAIhsoKNy6y7+IN93IfpFtwY9iqi7kD+xwKhQsNJA==",
+      "dev": true
+    },
+    "node_modules/typescript": {
+      "version": "4.1.3",
+      "resolved": "https://registry.npmjs.org/typescript/-/typescript-4.1.3.tgz",
+      "integrity": "sha512-B3ZIOf1IKeH2ixgHhj6la6xdwR9QrLC5d1VKeCSY4tvkqhF2eqd9O7txNlS0PO3GrBAFIdr3L1ndNwteUbZLYg==",
+      "dev": true,
+      "bin": {
+        "tsc": "bin/tsc",
+        "tsserver": "bin/tsserver"
+      },
+      "engines": {
+        "node": ">=4.2.0"
+      }
+    },
+    "node_modules/uri-js": {
+      "version": "4.4.0",
+      "resolved": "https://registry.npmjs.org/uri-js/-/uri-js-4.4.0.tgz",
+      "integrity": "sha512-B0yRTzYdUCCn9n+F4+Gh4yIDtMQcaJsmYBDsTSG8g/OejKBodLQ2IHfN3bM7jUsRXndopT7OIXWdYqc1fjmV6g==",
+      "dev": true,
+      "dependencies": {
+        "punycode": "^2.1.0"
+      }
+    },
+    "node_modules/util-deprecate": {
+      "version": "1.0.2",
+      "resolved": "https://registry.npmjs.org/util-deprecate/-/util-deprecate-1.0.2.tgz",
+      "integrity": "sha1-RQ1Nyfpw3nMnYvvS1KKJgUGaDM8=",
+      "dev": true
+    },
+    "node_modules/v8-compile-cache": {
+      "version": "2.1.1",
+      "resolved": "https://registry.npmjs.org/v8-compile-cache/-/v8-compile-cache-2.1.1.tgz",
+      "integrity": "sha512-8OQ9CL+VWyt3JStj7HX7/ciTL2V3Rl1Wf5OL+SNTm0yK1KvtReVulksyeRnCANHHuUxHlQig+JJDlUhBt1NQDQ==",
+      "dev": true
+    },
+    "node_modules/validate-npm-package-license": {
+      "version": "3.0.4",
+      "resolved": "https://registry.npmjs.org/validate-npm-package-license/-/validate-npm-package-license-3.0.4.tgz",
+      "integrity": "sha512-DpKm2Ui/xN7/HQKCtpZxoRWBhZ9Z0kqtygG8XCgNQ8ZlDnxuQmWhj566j8fN4Cu3/JmbhsDo7fcAJq4s9h27Ew==",
+      "dev": true,
+      "dependencies": {
+        "spdx-correct": "^3.0.0",
+        "spdx-expression-parse": "^3.0.0"
+      }
+    },
+    "node_modules/which": {
+      "version": "2.0.2",
+      "resolved": "https://registry.npmjs.org/which/-/which-2.0.2.tgz",
+      "integrity": "sha512-BLI3Tl1TW3Pvl70l3yq3Y64i+awpwXqsGBYWkkqMtnbXgrMD+yj7rhW0kuEDxzJaYXGjEW5ogapKNMEKNMjibA==",
+      "dev": true,
+      "dependencies": {
+        "isexe": "^2.0.0"
+      }
+    },
+    "node_modules/which-module": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/which-module/-/which-module-2.0.0.tgz",
+      "integrity": "sha1-2e8H3Od7mQK4o6j6SzHD4/fm6Ho=",
+      "dev": true
+    },
+    "node_modules/wide-align": {
+      "version": "1.1.3",
+      "resolved": "https://registry.npmjs.org/wide-align/-/wide-align-1.1.3.tgz",
+      "integrity": "sha512-QGkOQc8XL6Bt5PwnsExKBPuMKBxnGxWWW3fU55Xt4feHozMUhdUMaBCk290qpm/wG5u/RSKzwdAC4i51YigihA==",
+      "dev": true,
+      "dependencies": {
+        "string-width": "^1.0.2 || 2"
+      }
+    },
+    "node_modules/wide-align/node_modules/ansi-regex": {
+      "version": "3.0.0",
+      "resolved": "https://registry.npmjs.org/ansi-regex/-/ansi-regex-3.0.0.tgz",
+      "integrity": "sha1-7QMXwyIGT3lGbAKWa922Bas32Zg=",
+      "dev": true,
+      "engines": {
+        "node": ">=4"
+      }
+    },
+    "node_modules/wide-align/node_modules/string-width": {
+      "version": "2.1.1",
+      "resolved": "https://registry.npmjs.org/string-width/-/string-width-2.1.1.tgz",
+      "integrity": "sha512-nOqH59deCq9SRHlxq1Aw85Jnt4w6KvLKqWVik6oA9ZklXLNIOlqg4F2yrT1MVaTjAqvVwdfeZ7w7aCvJD7ugkw==",
+      "dev": true,
+      "dependencies": {
+        "is-fullwidth-code-point": "^2.0.0",
+        "strip-ansi": "^4.0.0"
+      },
+      "engines": {
+        "node": ">=4"
+      }
+    },
+    "node_modules/wide-align/node_modules/strip-ansi": {
+      "version": "4.0.0",
+      "resolved": "https://registry.npmjs.org/strip-ansi/-/strip-ansi-4.0.0.tgz",
+      "integrity": "sha1-qEeQIusaw2iocTibY1JixQXuNo8=",
+      "dev": true,
+      "dependencies": {
+        "ansi-regex": "^3.0.0"
+      },
+      "engines": {
+        "node": ">=4"
+      }
+    },
+    "node_modules/word-wrap": {
+      "version": "1.2.3",
+      "resolved": "https://registry.npmjs.org/word-wrap/-/word-wrap-1.2.3.tgz",
+      "integrity": "sha512-Hz/mrNwitNRh/HUAtM/VT/5VH+ygD6DV7mYKZAtHOrbs8U7lvPS6xf7EJKMF0uW1KJCl0H701g3ZGus+muE5vQ==",
+      "dev": true
+    },
+    "node_modules/workerpool": {
+      "version": "6.0.2",
+      "resolved": "https://registry.npmjs.org/workerpool/-/workerpool-6.0.2.tgz",
+      "integrity": "sha512-DSNyvOpFKrNusaaUwk+ej6cBj1bmhLcBfj80elGk+ZIo5JSkq+unB1dLKEOcNfJDZgjGICfhQ0Q5TbP0PvF4+Q==",
+      "dev": true
+    },
+    "node_modules/wrap-ansi": {
+      "version": "5.1.0",
+      "resolved": "https://registry.npmjs.org/wrap-ansi/-/wrap-ansi-5.1.0.tgz",
+      "integrity": "sha512-QC1/iN/2/RPVJ5jYK8BGttj5z83LmSKmvbvrXPNCLZSEb32KKVDJDl/MOt2N01qU2H/FkzEa9PKto1BqDjtd7Q==",
+      "dev": true,
+      "dependencies": {
+        "ansi-styles": "^3.2.0",
+        "string-width": "^3.0.0",
+        "strip-ansi": "^5.0.0"
+      },
+      "engines": {
+        "node": ">=6"
+      }
+    },
+    "node_modules/wrap-ansi/node_modules/ansi-regex": {
+      "version": "4.1.0",
+      "resolved": "https://registry.npmjs.org/ansi-regex/-/ansi-regex-4.1.0.tgz",
+      "integrity": "sha512-1apePfXM1UOSqw0o9IiFAovVz9M5S1Dg+4TrDwfMewQ6p/rmMueb7tWZjQ1rx4Loy1ArBggoqGpfqqdI4rondg==",
+      "dev": true,
+      "engines": {
+        "node": ">=6"
+      }
+    },
+    "node_modules/wrap-ansi/node_modules/strip-ansi": {
+      "version": "5.2.0",
+      "resolved": "https://registry.npmjs.org/strip-ansi/-/strip-ansi-5.2.0.tgz",
+      "integrity": "sha512-DuRs1gKbBqsMKIZlrffwlug8MHkcnpjs5VPmL1PAh+mA30U0DTotfDZ0d2UUsXpPmPmMMJ6W773MaA3J+lbiWA==",
+      "dev": true,
+      "dependencies": {
+        "ansi-regex": "^4.1.0"
+      },
+      "engines": {
+        "node": ">=6"
+      }
+    },
+    "node_modules/wrappy": {
+      "version": "1.0.2",
+      "resolved": "https://registry.npmjs.org/wrappy/-/wrappy-1.0.2.tgz",
+      "integrity": "sha1-tSQ9jz7BqjXxNkYFvA0QNuMKtp8=",
+      "dev": true
+    },
+    "node_modules/write": {
+      "version": "1.0.3",
+      "resolved": "https://registry.npmjs.org/write/-/write-1.0.3.tgz",
+      "integrity": "sha512-/lg70HAjtkUgWPVZhZcm+T4hkL8Zbtp1nFNOn3lRrxnlv50SRBv7cR7RqR+GMsd3hUXy9hWBo4CHTbFTcOYwig==",
+      "dev": true,
+      "dependencies": {
+        "mkdirp": "^0.5.1"
+      }
+    },
+    "node_modules/y18n": {
+      "version": "4.0.1",
+      "resolved": "https://registry.npmjs.org/y18n/-/y18n-4.0.1.tgz",
+      "integrity": "sha512-wNcy4NvjMYL8gogWWYAO7ZFWFfHcbdbE57tZO8e4cbpj8tfUcwrwqSl3ad8HxpYWCdXcJUCeKKZS62Av1affwQ==",
+      "dev": true
+    },
+    "node_modules/yallist": {
+      "version": "4.0.0",
+      "resolved": "https://registry.npmjs.org/yallist/-/yallist-4.0.0.tgz",
+      "integrity": "sha512-3wdGidZyq5PB084XLES5TpOSRA3wjXAlIWMhum2kRcv/41Sn2emQ0dycQW4uZXLejwKvg6EsvbdlVL+FYEct7A==",
+      "dev": true
+    },
+    "node_modules/yargs": {
+      "version": "13.3.2",
+      "resolved": "https://registry.npmjs.org/yargs/-/yargs-13.3.2.tgz",
+      "integrity": "sha512-AX3Zw5iPruN5ie6xGRIDgqkT+ZhnRlZMLMHAs8tg7nRruy2Nb+i5o9bwghAogtM08q1dpr2LVoS8KSTMYpWXUw==",
+      "dev": true,
+      "dependencies": {
+        "cliui": "^5.0.0",
+        "find-up": "^3.0.0",
+        "get-caller-file": "^2.0.1",
+        "require-directory": "^2.1.1",
+        "require-main-filename": "^2.0.0",
+        "set-blocking": "^2.0.0",
+        "string-width": "^3.0.0",
+        "which-module": "^2.0.0",
+        "y18n": "^4.0.0",
+        "yargs-parser": "^13.1.2"
+      }
+    },
+    "node_modules/yargs-parser": {
+      "version": "13.1.2",
+      "resolved": "https://registry.npmjs.org/yargs-parser/-/yargs-parser-13.1.2.tgz",
+      "integrity": "sha512-3lbsNRf/j+A4QuSZfDRA7HRSfWrzO0YjqTJd5kjAq37Zep1CEgaYmrH9Q3GwPiB9cHyd1Y1UwggGhJGoxipbzg==",
+      "dev": true,
+      "dependencies": {
+        "camelcase": "^5.0.0",
+        "decamelize": "^1.2.0"
+      }
+    },
+    "node_modules/yargs-unparser": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/yargs-unparser/-/yargs-unparser-2.0.0.tgz",
+      "integrity": "sha512-7pRTIA9Qc1caZ0bZ6RYRGbHJthJWuakf+WmHK0rVeLkNrrGhfoabBNdue6kdINI6r4if7ocq9aD/n7xwKOdzOA==",
+      "dev": true,
+      "dependencies": {
+        "camelcase": "^6.0.0",
+        "decamelize": "^4.0.0",
+        "flat": "^5.0.2",
+        "is-plain-obj": "^2.1.0"
+      },
+      "engines": {
+        "node": ">=10"
+      }
+    },
+    "node_modules/yargs-unparser/node_modules/camelcase": {
+      "version": "6.2.0",
+      "resolved": "https://registry.npmjs.org/camelcase/-/camelcase-6.2.0.tgz",
+      "integrity": "sha512-c7wVvbw3f37nuobQNtgsgG9POC9qMbNuMQmTCqZv23b6MIz0fcYpBiOlv9gEN/hdLdnZTDQhg6e9Dq5M1vKvfg==",
+      "dev": true,
+      "engines": {
+        "node": ">=10"
+      },
+      "funding": {
+        "url": "https://github.com/sponsors/sindresorhus"
+      }
+    },
+    "node_modules/yargs-unparser/node_modules/decamelize": {
+      "version": "4.0.0",
+      "resolved": "https://registry.npmjs.org/decamelize/-/decamelize-4.0.0.tgz",
+      "integrity": "sha512-9iE1PgSik9HeIIw2JO94IidnE3eBoQrFJ3w7sFuzSX4DpmZ3v5sZpUiV5Swcf6mQEF+Y0ru8Neo+p+nyh2J+hQ==",
+      "dev": true,
+      "engines": {
+        "node": ">=10"
+      },
+      "funding": {
+        "url": "https://github.com/sponsors/sindresorhus"
+      }
+    },
+    "node_modules/yargs/node_modules/find-up": {
+      "version": "3.0.0",
+      "resolved": "https://registry.npmjs.org/find-up/-/find-up-3.0.0.tgz",
+      "integrity": "sha512-1yD6RmLI1XBfxugvORwlck6f75tYL+iR0jqwsOrOxMZyGYqUuDhJ0l4AXdO1iX/FTs9cBAMEk1gWSEx1kSbylg==",
+      "dev": true,
+      "dependencies": {
+        "locate-path": "^3.0.0"
+      },
+      "engines": {
+        "node": ">=6"
+      }
+    },
+    "node_modules/yargs/node_modules/locate-path": {
+      "version": "3.0.0",
+      "resolved": "https://registry.npmjs.org/locate-path/-/locate-path-3.0.0.tgz",
+      "integrity": "sha512-7AO748wWnIhNqAuaty2ZWHkQHRSNfPVIsPIfwEOWO22AmaoVrWavlOcMR5nzTLNYvp36X220/maaRsrec1G65A==",
+      "dev": true,
+      "dependencies": {
+        "p-locate": "^3.0.0",
+        "path-exists": "^3.0.0"
+      },
+      "engines": {
+        "node": ">=6"
+      }
+    },
+    "node_modules/yargs/node_modules/p-limit": {
+      "version": "2.3.0",
+      "resolved": "https://registry.npmjs.org/p-limit/-/p-limit-2.3.0.tgz",
+      "integrity": "sha512-//88mFWSJx8lxCzwdAABTJL2MyWB12+eIY7MDL2SqLmAkeKU9qxRvWuSyTjm3FUmpBEMuFfckAIqEaVGUDxb6w==",
+      "dev": true,
+      "dependencies": {
+        "p-try": "^2.0.0"
+      },
+      "engines": {
+        "node": ">=6"
+      },
+      "funding": {
+        "url": "https://github.com/sponsors/sindresorhus"
+      }
+    },
+    "node_modules/yargs/node_modules/p-locate": {
+      "version": "3.0.0",
+      "resolved": "https://registry.npmjs.org/p-locate/-/p-locate-3.0.0.tgz",
+      "integrity": "sha512-x+12w/To+4GFfgJhBEpiDcLozRJGegY+Ei7/z0tSLkMmxGZNybVMSfWj9aJn8Z5Fc7dBUNJOOVgPv2H7IwulSQ==",
+      "dev": true,
+      "dependencies": {
+        "p-limit": "^2.0.0"
+      },
+      "engines": {
+        "node": ">=6"
+      }
+    },
+    "node_modules/yargs/node_modules/p-try": {
+      "version": "2.2.0",
+      "resolved": "https://registry.npmjs.org/p-try/-/p-try-2.2.0.tgz",
+      "integrity": "sha512-R4nPAVTAU0B9D35/Gk3uJf/7XYbQcyohSKdvAxIRSNghFl4e71hVoGnBNQz9cWaXxO2I10KTC+3jMdvvoKw6dQ==",
+      "dev": true,
+      "engines": {
+        "node": ">=6"
+      }
+    },
+    "node_modules/yarn": {
+      "version": "1.22.0",
+      "resolved": "https://registry.npmjs.org/yarn/-/yarn-1.22.0.tgz",
+      "integrity": "sha512-KMHP/Jq53jZKTY9iTUt3dIVl/be6UPs2INo96+BnZHLKxYNTfwMmlgHTaMWyGZoO74RI4AIFvnWhYrXq2USJkg==",
+      "dev": true
+    },
+    "node_modules/yocto-queue": {
+      "version": "0.1.0",
+      "resolved": "https://registry.npmjs.org/yocto-queue/-/yocto-queue-0.1.0.tgz",
+      "integrity": "sha512-rVksvsnNCdJ/ohGc6xgPwyN8eheCxsiLM8mxuE/t/mOVqJewPuO1miLpTHQiRgTKCLexL4MeAFVagts7HmNZ2Q==",
+      "dev": true,
+      "engines": {
+        "node": ">=10"
+      },
+      "funding": {
+        "url": "https://github.com/sponsors/sindresorhus"
+      }
+    },
+    "tools/lint/eslint/eslint-plugin-mozilla": {
+      "version": "2.9.1",
+      "dev": true,
+      "license": "MPL-2.0",
+      "dependencies": {
+        "@babel/core": "7.8.3",
+        "babel-eslint": "10.1.0",
+        "eslint-scope": "5.1.0",
+        "eslint-visitor-keys": "1.3.0",
+        "estraverse": "4.3.0",
+        "htmlparser2": "3.10.1",
+        "multi-ini": "2.1.0",
+        "sax": "1.2.4"
+      },
+      "devDependencies": {
+        "eslint": "7.8.0",
+        "mocha": "^8.1.3"
+      },
+      "engines": {
+        "node": ">=6.9.1"
+      },
+      "peerDependencies": {
+        "eslint": "^7.8.0",
+        "eslint-config-prettier": "^6.0.0",
+        "eslint-plugin-fetch-options": "^0.0.5",
+        "eslint-plugin-html": "^6.0.0",
+        "eslint-plugin-no-unsanitized": "^3.1.4",
+        "eslint-plugin-prettier": "^3.0.1",
+        "prettier": "^1.17.0"
+      }
+    },
+    "tools/lint/eslint/eslint-plugin-spidermonkey-js": {
+      "version": "0.1.1",
+      "dev": true,
+      "license": "MPL-2.0",
+      "devDependencies": {},
+      "engines": {
+        "node": ">=6.9.1"
+      }
+    }
+  },
   "dependencies": {
     "@babel/code-frame": {
       "version": "7.10.4",
@@ -186,6 +3593,32 @@
         }
       }
     },
+    "@nodelib/fs.scandir": {
+      "version": "2.1.3",
+      "resolved": "https://registry.npmjs.org/@nodelib/fs.scandir/-/fs.scandir-2.1.3.tgz",
+      "integrity": "sha512-eGmwYQn3gxo4r7jdQnkrrN6bY478C3P+a/y72IJukF8LjB6ZHeB3c+Ehacj3sYeSmUXGlnA67/PmbM9CVwL7Dw==",
+      "dev": true,
+      "requires": {
+        "@nodelib/fs.stat": "2.0.3",
+        "run-parallel": "^1.1.9"
+      }
+    },
+    "@nodelib/fs.stat": {
+      "version": "2.0.3",
+      "resolved": "https://registry.npmjs.org/@nodelib/fs.stat/-/fs.stat-2.0.3.tgz",
+      "integrity": "sha512-bQBFruR2TAwoevBEd/NWMoAAtNGzTRgdrqnYCc7dhzfoNvqPzLyqlEQnzZ3kVnNrSp25iyxE00/3h2fqGAGArA==",
+      "dev": true
+    },
+    "@nodelib/fs.walk": {
+      "version": "1.2.4",
+      "resolved": "https://registry.npmjs.org/@nodelib/fs.walk/-/fs.walk-1.2.4.tgz",
+      "integrity": "sha512-1V9XOY4rDW0rehzbrcqAmHnz8e7SKvX27gh8Gt2WgB0+pdzdiLV83p72kZPU+jvMbS1qU5mauP2iOvO8rhmurQ==",
+      "dev": true,
+      "requires": {
+        "@nodelib/fs.scandir": "2.1.3",
+        "fastq": "^1.6.0"
+      }
+    },
     "@types/color-name": {
       "version": "1.1.1",
       "resolved": "https://registry.npmjs.org/@types/color-name/-/color-name-1.1.1.tgz",
@@ -198,6 +3631,94 @@
       "integrity": "sha512-7+2BITlgjgDhH0vvwZU/HZJVyk+2XUlvxXe8dFMedNX/aMkaOq++rMAFXc0tM7ij15QaWlbdQASBR9dihi+bDQ==",
       "dev": true
     },
+    "@typescript-eslint/eslint-plugin": {
+      "version": "4.12.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/eslint-plugin/-/eslint-plugin-4.12.0.tgz",
+      "integrity": "sha512-wHKj6q8s70sO5i39H2g1gtpCXCvjVszzj6FFygneNFyIAxRvNSVz9GML7XpqrB9t7hNutXw+MHnLN/Ih6uyB8Q==",
+      "dev": true,
+      "requires": {
+        "@typescript-eslint/experimental-utils": "4.12.0",
+        "@typescript-eslint/scope-manager": "4.12.0",
+        "debug": "^4.1.1",
+        "functional-red-black-tree": "^1.0.1",
+        "regexpp": "^3.0.0",
+        "semver": "^7.3.2",
+        "tsutils": "^3.17.1"
+      },
+      "dependencies": {
+        "@typescript-eslint/experimental-utils": {
+          "version": "4.12.0",
+          "resolved": "https://registry.npmjs.org/@typescript-eslint/experimental-utils/-/experimental-utils-4.12.0.tgz",
+          "integrity": "sha512-MpXZXUAvHt99c9ScXijx7i061o5HEjXltO+sbYfZAAHxv3XankQkPaNi5myy0Yh0Tyea3Hdq1pi7Vsh0GJb0fA==",
+          "dev": true,
+          "requires": {
+            "@types/json-schema": "^7.0.3",
+            "@typescript-eslint/scope-manager": "4.12.0",
+            "@typescript-eslint/types": "4.12.0",
+            "@typescript-eslint/typescript-estree": "4.12.0",
+            "eslint-scope": "^5.0.0",
+            "eslint-utils": "^2.0.0"
+          }
+        },
+        "@typescript-eslint/scope-manager": {
+          "version": "4.12.0",
+          "resolved": "https://registry.npmjs.org/@typescript-eslint/scope-manager/-/scope-manager-4.12.0.tgz",
+          "integrity": "sha512-QVf9oCSVLte/8jvOsxmgBdOaoe2J0wtEmBr13Yz0rkBNkl5D8bfnf6G4Vhox9qqMIoG7QQoVwd2eG9DM/ge4Qg==",
+          "dev": true,
+          "requires": {
+            "@typescript-eslint/types": "4.12.0",
+            "@typescript-eslint/visitor-keys": "4.12.0"
+          }
+        },
+        "@typescript-eslint/types": {
+          "version": "4.12.0",
+          "resolved": "https://registry.npmjs.org/@typescript-eslint/types/-/types-4.12.0.tgz",
+          "integrity": "sha512-N2RhGeheVLGtyy+CxRmxdsniB7sMSCfsnbh8K/+RUIXYYq3Ub5+sukRCjVE80QerrUBvuEvs4fDhz5AW/pcL6g==",
+          "dev": true
+        },
+        "@typescript-eslint/typescript-estree": {
+          "version": "4.12.0",
+          "resolved": "https://registry.npmjs.org/@typescript-eslint/typescript-estree/-/typescript-estree-4.12.0.tgz",
+          "integrity": "sha512-gZkFcmmp/CnzqD2RKMich2/FjBTsYopjiwJCroxqHZIY11IIoN0l5lKqcgoAPKHt33H2mAkSfvzj8i44Jm7F4w==",
+          "dev": true,
+          "requires": {
+            "@typescript-eslint/types": "4.12.0",
+            "@typescript-eslint/visitor-keys": "4.12.0",
+            "debug": "^4.1.1",
+            "globby": "^11.0.1",
+            "is-glob": "^4.0.1",
+            "lodash": "^4.17.15",
+            "semver": "^7.3.2",
+            "tsutils": "^3.17.1"
+          }
+        },
+        "@typescript-eslint/visitor-keys": {
+          "version": "4.12.0",
+          "resolved": "https://registry.npmjs.org/@typescript-eslint/visitor-keys/-/visitor-keys-4.12.0.tgz",
+          "integrity": "sha512-hVpsLARbDh4B9TKYz5cLbcdMIOAoBYgFPCSP9FFS/liSF+b33gVNq8JHY3QGhHNVz85hObvL7BEYLlgx553WCw==",
+          "dev": true,
+          "requires": {
+            "@typescript-eslint/types": "4.12.0",
+            "eslint-visitor-keys": "^2.0.0"
+          }
+        },
+        "eslint-visitor-keys": {
+          "version": "2.0.0",
+          "resolved": "https://registry.npmjs.org/eslint-visitor-keys/-/eslint-visitor-keys-2.0.0.tgz",
+          "integrity": "sha512-QudtT6av5WXels9WjIM7qz1XD1cWGvX4gGXvp/zBn9nXG02D0utdU3Em2m/QjTnrsk6bBjmCygl3rmj118msQQ==",
+          "dev": true
+        },
+        "semver": {
+          "version": "7.3.4",
+          "resolved": "https://registry.npmjs.org/semver/-/semver-7.3.4.tgz",
+          "integrity": "sha512-tCfb2WLjqFAtXn4KEdxIhalnRtoKFN7nAwj0B3ZXCbQloV2tq5eDbcTmT68JJD3nRJq24/XgxtQKFIpQdtvmVw==",
+          "dev": true,
+          "requires": {
+            "lru-cache": "^6.0.0"
+          }
+        }
+      }
+    },
     "@typescript-eslint/experimental-utils": {
       "version": "2.34.0",
       "resolved": "https://registry.npmjs.org/@typescript-eslint/experimental-utils/-/experimental-utils-2.34.0.tgz",
@@ -210,6 +3731,61 @@
         "eslint-utils": "^2.0.0"
       }
     },
+    "@typescript-eslint/parser": {
+      "version": "4.10.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/parser/-/parser-4.10.0.tgz",
+      "integrity": "sha512-amBvUUGBMadzCW6c/qaZmfr3t9PyevcSWw7hY2FuevdZVp5QPw/K76VSQ5Sw3BxlgYCHZcK6DjIhSZK0PQNsQg==",
+      "dev": true,
+      "requires": {
+        "@typescript-eslint/scope-manager": "4.10.0",
+        "@typescript-eslint/types": "4.10.0",
+        "@typescript-eslint/typescript-estree": "4.10.0",
+        "debug": "^4.1.1"
+      },
+      "dependencies": {
+        "@typescript-eslint/typescript-estree": {
+          "version": "4.10.0",
+          "resolved": "https://registry.npmjs.org/@typescript-eslint/typescript-estree/-/typescript-estree-4.10.0.tgz",
+          "integrity": "sha512-mGK0YRp9TOk6ZqZ98F++bW6X5kMTzCRROJkGXH62d2azhghmq+1LNLylkGe6uGUOQzD452NOAEth5VAF6PDo5g==",
+          "dev": true,
+          "requires": {
+            "@typescript-eslint/types": "4.10.0",
+            "@typescript-eslint/visitor-keys": "4.10.0",
+            "debug": "^4.1.1",
+            "globby": "^11.0.1",
+            "is-glob": "^4.0.1",
+            "lodash": "^4.17.15",
+            "semver": "^7.3.2",
+            "tsutils": "^3.17.1"
+          }
+        },
+        "semver": {
+          "version": "7.3.4",
+          "resolved": "https://registry.npmjs.org/semver/-/semver-7.3.4.tgz",
+          "integrity": "sha512-tCfb2WLjqFAtXn4KEdxIhalnRtoKFN7nAwj0B3ZXCbQloV2tq5eDbcTmT68JJD3nRJq24/XgxtQKFIpQdtvmVw==",
+          "dev": true,
+          "requires": {
+            "lru-cache": "^6.0.0"
+          }
+        }
+      }
+    },
+    "@typescript-eslint/scope-manager": {
+      "version": "4.10.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/scope-manager/-/scope-manager-4.10.0.tgz",
+      "integrity": "sha512-WAPVw35P+fcnOa8DEic0tQUhoJJsgt+g6DEcz257G7vHFMwmag58EfowdVbiNcdfcV27EFR0tUBVXkDoIvfisQ==",
+      "dev": true,
+      "requires": {
+        "@typescript-eslint/types": "4.10.0",
+        "@typescript-eslint/visitor-keys": "4.10.0"
+      }
+    },
+    "@typescript-eslint/types": {
+      "version": "4.10.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/types/-/types-4.10.0.tgz",
+      "integrity": "sha512-+dt5w1+Lqyd7wIPMa4XhJxUuE8+YF+vxQ6zxHyhLGHJjHiunPf0wSV8LtQwkpmAsRi1lEOoOIR30FG5S2HS33g==",
+      "dev": true
+    },
     "@typescript-eslint/typescript-estree": {
       "version": "2.34.0",
       "resolved": "https://registry.npmjs.org/@typescript-eslint/typescript-estree/-/typescript-estree-2.34.0.tgz",
@@ -233,6 +3809,30 @@
         }
       }
     },
+    "@typescript-eslint/visitor-keys": {
+      "version": "4.10.0",
+      "resolved": "https://registry.npmjs.org/@typescript-eslint/visitor-keys/-/visitor-keys-4.10.0.tgz",
+      "integrity": "sha512-hPyz5qmDMuZWFtHZkjcCpkAKHX8vdu1G3YsCLEd25ryZgnJfj6FQuJ5/O7R+dB1ueszilJmAFMtlU4CA6se3Jg==",
+      "dev": true,
+      "requires": {
+        "@typescript-eslint/types": "4.10.0",
+        "eslint-visitor-keys": "^2.0.0"
+      },
+      "dependencies": {
+        "eslint-visitor-keys": {
+          "version": "2.0.0",
+          "resolved": "https://registry.npmjs.org/eslint-visitor-keys/-/eslint-visitor-keys-2.0.0.tgz",
+          "integrity": "sha512-QudtT6av5WXels9WjIM7qz1XD1cWGvX4gGXvp/zBn9nXG02D0utdU3Em2m/QjTnrsk6bBjmCygl3rmj118msQQ==",
+          "dev": true
+        }
+      }
+    },
+    "@ungap/promise-all-settled": {
+      "version": "1.1.2",
+      "resolved": "https://registry.npmjs.org/@ungap/promise-all-settled/-/promise-all-settled-1.1.2.tgz",
+      "integrity": "sha512-sL/cEvJWAnClXw0wHk85/2L0G6Sj8UB0Ctc1TEMbKSsmpRosqhwj9gWgFRZSrBr2f9tiXISwNhCPmlfqUqyb9Q==",
+      "dev": true
+    },
     "acorn": {
       "version": "7.4.0",
       "resolved": "https://registry.npmjs.org/acorn/-/acorn-7.4.0.tgz",
@@ -278,6 +3878,16 @@
         "color-convert": "^1.9.0"
       }
     },
+    "anymatch": {
+      "version": "3.1.1",
+      "resolved": "https://registry.npmjs.org/anymatch/-/anymatch-3.1.1.tgz",
+      "integrity": "sha512-mM8522psRCqzV+6LhomX5wgp25YVibjh8Wj23I5RPkPppSVSjyKD2A2mBJmWGa+KN7f2D6LNh9jkBCeyLktzjg==",
+      "dev": true,
+      "requires": {
+        "normalize-path": "^3.0.0",
+        "picomatch": "^2.0.4"
+      }
+    },
     "argparse": {
       "version": "1.0.10",
       "resolved": "https://registry.npmjs.org/argparse/-/argparse-1.0.10.tgz",
@@ -308,6 +3918,12 @@
         "is-string": "^1.0.5"
       }
     },
+    "array-union": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/array-union/-/array-union-2.1.0.tgz",
+      "integrity": "sha512-HGyxoOTYUyCM6stUe6EJgnd4EoewAI7zMdfqO+kGjnlZmBDz/cR5pf8r/cR4Wq60sL/p0IkcjUEEPwS3GFrIyw==",
+      "dev": true
+    },
     "array.prototype.flat": {
       "version": "1.2.3",
       "resolved": "https://registry.npmjs.org/array.prototype.flat/-/array.prototype.flat-1.2.3.tgz",
@@ -356,6 +3972,12 @@
       "integrity": "sha1-ibTRmasr7kneFk6gK4nORi1xt2c=",
       "dev": true
     },
+    "binary-extensions": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/binary-extensions/-/binary-extensions-2.1.0.tgz",
+      "integrity": "sha512-1Yj8h9Q+QDF5FzhMs/c9+6UntbD5MkRfRwac8DoEm9ZfUBZ7tZ55YcGVAzEe4bXsdQHEk+s9S5wsOKVdZrw0tQ==",
+      "dev": true
+    },
     "brace-expansion": {
       "version": "1.1.11",
       "resolved": "https://registry.npmjs.org/brace-expansion/-/brace-expansion-1.1.11.tgz",
@@ -366,12 +3988,33 @@
         "concat-map": "0.0.1"
       }
     },
+    "braces": {
+      "version": "3.0.2",
+      "resolved": "https://registry.npmjs.org/braces/-/braces-3.0.2.tgz",
+      "integrity": "sha512-b8um+L1RzM3WDSzvhm6gIz1yfTbBt6YTlcEKAvsmqCZZFw46z626lVj9j1yEPW33H5H+lBQpZMP1k8l+78Ha0A==",
+      "dev": true,
+      "requires": {
+        "fill-range": "^7.0.1"
+      }
+    },
+    "browser-stdout": {
+      "version": "1.3.1",
+      "resolved": "https://registry.npmjs.org/browser-stdout/-/browser-stdout-1.3.1.tgz",
+      "integrity": "sha512-qhAVI1+Av2X7qelOfAIYwXONood6XlZE/fXaBSmW/T5SzLAmCgzi+eiWE7fUvbHaeNBQH13UftjpXxsfLkMpgw==",
+      "dev": true
+    },
     "callsites": {
       "version": "3.1.0",
       "resolved": "https://registry.npmjs.org/callsites/-/callsites-3.1.0.tgz",
       "integrity": "sha512-P8BjAsXvZS+VIDUI11hHCQEv74YT67YUi5JJFNWIqL235sBmjX4+qx9Muvls5ivyNENctx46xQLQ3aTuE7ssaQ==",
       "dev": true
     },
+    "camelcase": {
+      "version": "5.3.1",
+      "resolved": "https://registry.npmjs.org/camelcase/-/camelcase-5.3.1.tgz",
+      "integrity": "sha512-L28STB170nwWS63UjtlEOE3dldQApaJXZkOI1uMFfzf3rRuPegHaHesyee+YxQ+W6SvRDQV6UrdOdRiR153wJg==",
+      "dev": true
+    },
     "chalk": {
       "version": "2.4.2",
       "resolved": "https://registry.npmjs.org/chalk/-/chalk-2.4.2.tgz",
@@ -383,6 +4026,50 @@
         "supports-color": "^5.3.0"
       }
     },
+    "chokidar": {
+      "version": "3.4.3",
+      "resolved": "https://registry.npmjs.org/chokidar/-/chokidar-3.4.3.tgz",
+      "integrity": "sha512-DtM3g7juCXQxFVSNPNByEC2+NImtBuxQQvWlHunpJIS5Ocr0lG306cC7FCi7cEA0fzmybPUIl4txBIobk1gGOQ==",
+      "dev": true,
+      "requires": {
+        "anymatch": "~3.1.1",
+        "braces": "~3.0.2",
+        "fsevents": "~2.1.2",
+        "glob-parent": "~5.1.0",
+        "is-binary-path": "~2.1.0",
+        "is-glob": "~4.0.1",
+        "normalize-path": "~3.0.0",
+        "readdirp": "~3.5.0"
+      }
+    },
+    "cliui": {
+      "version": "5.0.0",
+      "resolved": "https://registry.npmjs.org/cliui/-/cliui-5.0.0.tgz",
+      "integrity": "sha512-PYeGSEmmHM6zvoef2w8TPzlrnNpXIjTipYK780YswmIP9vjxmd6Y2a3CB2Ks6/AU8NHjZugXvo8w3oWM2qnwXA==",
+      "dev": true,
+      "requires": {
+        "string-width": "^3.1.0",
+        "strip-ansi": "^5.2.0",
+        "wrap-ansi": "^5.1.0"
+      },
+      "dependencies": {
+        "ansi-regex": {
+          "version": "4.1.0",
+          "resolved": "https://registry.npmjs.org/ansi-regex/-/ansi-regex-4.1.0.tgz",
+          "integrity": "sha512-1apePfXM1UOSqw0o9IiFAovVz9M5S1Dg+4TrDwfMewQ6p/rmMueb7tWZjQ1rx4Loy1ArBggoqGpfqqdI4rondg==",
+          "dev": true
+        },
+        "strip-ansi": {
+          "version": "5.2.0",
+          "resolved": "https://registry.npmjs.org/strip-ansi/-/strip-ansi-5.2.0.tgz",
+          "integrity": "sha512-DuRs1gKbBqsMKIZlrffwlug8MHkcnpjs5VPmL1PAh+mA30U0DTotfDZ0d2UUsXpPmPmMMJ6W773MaA3J+lbiWA==",
+          "dev": true,
+          "requires": {
+            "ansi-regex": "^4.1.0"
+          }
+        }
+      }
+    },
     "color-convert": {
       "version": "1.9.3",
       "resolved": "https://registry.npmjs.org/color-convert/-/color-convert-1.9.3.tgz",
@@ -443,13 +4130,19 @@
       "dev": true
     },
     "debug": {
-      "version": "4.1.1",
-      "resolved": "https://registry.npmjs.org/debug/-/debug-4.1.1.tgz",
-      "integrity": "sha512-pYAIzeRo8J6KPEaJ0VWOh5Pzkbw/RetuzehGM7QRRX5he4fPHx2rdKMB256ehJCkX+XRQm16eZLqLNS8RSZXZw==",
+      "version": "4.2.0",
+      "resolved": "https://registry.npmjs.org/debug/-/debug-4.2.0.tgz",
+      "integrity": "sha512-IX2ncY78vDTjZMFUdmsvIRFY2Cf4FnD0wRs+nQwJU8Lu99/tPFdb0VybiiMTPe3I6rQmwsqQqRBvxU+bZ/I8sg==",
       "dev": true,
       "requires": {
-        "ms": "^2.1.1"
-      }
+        "ms": "2.1.2"
+      }
+    },
+    "decamelize": {
+      "version": "1.2.0",
+      "resolved": "https://registry.npmjs.org/decamelize/-/decamelize-1.2.0.tgz",
+      "integrity": "sha1-9lNNFRSCabIDUue+4m9QH5oZEpA=",
+      "dev": true
     },
     "deep-is": {
       "version": "0.1.3",
@@ -466,6 +4159,29 @@
         "object-keys": "^1.0.12"
       }
     },
+    "diff": {
+      "version": "4.0.2",
+      "resolved": "https://registry.npmjs.org/diff/-/diff-4.0.2.tgz",
+      "integrity": "sha512-58lmxKSA4BNyLz+HHMUzlOEpg09FV+ev6ZMe3vJihgdxzgcwZ8VoEEPmALCZG9LmqfVoNMMKpttIYTVG6uDY7A==",
+      "dev": true
+    },
+    "dir-glob": {
+      "version": "3.0.1",
+      "resolved": "https://registry.npmjs.org/dir-glob/-/dir-glob-3.0.1.tgz",
+      "integrity": "sha512-WkrWp9GR4KXfKGYzOLmTuGVi1UWFfws377n9cc55/tb6DuqyF6pcQ5AbiHEshaDpY9v6oaSr2XCDidGmMwdzIA==",
+      "dev": true,
+      "requires": {
+        "path-type": "^4.0.0"
+      },
+      "dependencies": {
+        "path-type": {
+          "version": "4.0.0",
+          "resolved": "https://registry.npmjs.org/path-type/-/path-type-4.0.0.tgz",
+          "integrity": "sha512-gDKb8aZMDeD/tZWs9P6+q0J9Mwkdl6xMV8TjnGP3qJVJ06bdMgkbBlLU8IdfOsIsFz2BW1rNVT3XuNEl8zPAvw==",
+          "dev": true
+        }
+      }
+    },
     "doctrine": {
       "version": "3.0.0",
       "resolved": "https://registry.npmjs.org/doctrine/-/doctrine-3.0.0.tgz",
@@ -879,14 +4595,15 @@
     },
     "eslint-plugin-mozilla": {
       "version": "file:tools/lint/eslint/eslint-plugin-mozilla",
-      "dev": true,
       "requires": {
         "@babel/core": "7.8.3",
         "babel-eslint": "10.1.0",
+        "eslint": "7.8.0",
         "eslint-scope": "5.1.0",
         "eslint-visitor-keys": "1.3.0",
         "estraverse": "4.3.0",
         "htmlparser2": "3.10.1",
+        "mocha": "^8.1.3",
         "multi-ini": "2.1.0",
         "sax": "1.2.4"
       }
@@ -936,8 +4653,7 @@
       }
     },
     "eslint-plugin-spidermonkey-js": {
-      "version": "file:tools/lint/eslint/eslint-plugin-spidermonkey-js",
-      "dev": true
+      "version": "file:tools/lint/eslint/eslint-plugin-spidermonkey-js"
     },
     "eslint-rule-composer": {
       "version": "0.3.0",
@@ -1037,6 +4753,20 @@
       "integrity": "sha512-xJuoT5+L99XlZ8twedaRf6Ax2TgQVxvgZOYoPKqZufmJib0tL2tegPBOZb1pVNgIhlqDlA0eO0c3wBvQcmzx4w==",
       "dev": true
     },
+    "fast-glob": {
+      "version": "3.2.4",
+      "resolved": "https://registry.npmjs.org/fast-glob/-/fast-glob-3.2.4.tgz",
+      "integrity": "sha512-kr/Oo6PX51265qeuCYsyGypiO5uJFgBS0jksyG7FUeCyQzNwYnzrNIMR1NXfkZXsMYXYLRAHgISHBz8gQcxKHQ==",
+      "dev": true,
+      "requires": {
+        "@nodelib/fs.stat": "^2.0.2",
+        "@nodelib/fs.walk": "^1.2.3",
+        "glob-parent": "^5.1.0",
+        "merge2": "^1.3.0",
+        "micromatch": "^4.0.2",
+        "picomatch": "^2.2.1"
+      }
+    },
     "fast-json-stable-stringify": {
       "version": "2.1.0",
       "resolved": "https://registry.npmjs.org/fast-json-stable-stringify/-/fast-json-stable-stringify-2.1.0.tgz",
@@ -1049,6 +4779,15 @@
       "integrity": "sha1-PYpcZog6FqMMqGQ+hR8Zuqd5eRc=",
       "dev": true
     },
+    "fastq": {
+      "version": "1.9.0",
+      "resolved": "https://registry.npmjs.org/fastq/-/fastq-1.9.0.tgz",
+      "integrity": "sha512-i7FVWL8HhVY+CTkwFxkN2mk3h+787ixS5S63eb78diVRc1MCssarHq3W5cj0av7YDSwmaV928RNag+U1etRQ7w==",
+      "dev": true,
+      "requires": {
+        "reusify": "^1.0.4"
+      }
+    },
     "file-entry-cache": {
       "version": "5.0.1",
       "resolved": "https://registry.npmjs.org/file-entry-cache/-/file-entry-cache-5.0.1.tgz",
@@ -1058,6 +4797,15 @@
         "flat-cache": "^2.0.1"
       }
     },
+    "fill-range": {
+      "version": "7.0.1",
+      "resolved": "https://registry.npmjs.org/fill-range/-/fill-range-7.0.1.tgz",
+      "integrity": "sha512-qOo9F+dMUmC2Lcb4BbVvnKJxTPjCm+RRpe4gDuGrzkL7mEVl/djYSu2OdQ2Pa302N4oqkSg9ir6jaLWJ2USVpQ==",
+      "dev": true,
+      "requires": {
+        "to-regex-range": "^5.0.1"
+      }
+    },
     "find-up": {
       "version": "2.1.0",
       "resolved": "https://registry.npmjs.org/find-up/-/find-up-2.1.0.tgz",
@@ -1067,6 +4815,12 @@
         "locate-path": "^2.0.0"
       }
     },
+    "flat": {
+      "version": "5.0.2",
+      "resolved": "https://registry.npmjs.org/flat/-/flat-5.0.2.tgz",
+      "integrity": "sha512-b6suED+5/3rTpUBdG1gupIl8MPFCAMA0QXwmljLhvCUKcUvdE4gWky9zpuGCcXHOsz4J9wPGNWq6OKpmIzz3hQ==",
+      "dev": true
+    },
     "flat-cache": {
       "version": "2.0.1",
       "resolved": "https://registry.npmjs.org/flat-cache/-/flat-cache-2.0.1.tgz",
@@ -1108,6 +4862,12 @@
       "integrity": "sha512-r8EC6NO1sngH/zdD9fiRDLdcgnbayXah+mLgManTaIZJqEC1MZstmnox8KpnI2/fxQwrp5OpCOYWLp4rBl4Jcg==",
       "dev": true
     },
+    "get-caller-file": {
+      "version": "2.0.5",
+      "resolved": "https://registry.npmjs.org/get-caller-file/-/get-caller-file-2.0.5.tgz",
+      "integrity": "sha512-DyFP3BM/3YHTQOCUL/w0OZHR0lpKeGrxotcHWcqNEdnltqFwXVfhEBQ94eIo34AfQpo0rGki4cyIiftY06h2Fg==",
+      "dev": true
+    },
     "get-stdin": {
       "version": "6.0.0",
       "resolved": "https://registry.npmjs.org/get-stdin/-/get-stdin-6.0.0.tgz",
@@ -1143,12 +4903,40 @@
       "integrity": "sha512-WOBp/EEGUiIsJSp7wcv/y6MO+lV9UoncWqxuFfm8eBwzWNgyfBd6Gz+IeKQ9jCmyhoH99g15M3T+QaVHFjizVA==",
       "dev": true
     },
+    "globby": {
+      "version": "11.0.1",
+      "resolved": "https://registry.npmjs.org/globby/-/globby-11.0.1.tgz",
+      "integrity": "sha512-iH9RmgwCmUJHi2z5o2l3eTtGBtXek1OYlHrbcxOYugyHLmAsZrPj43OtHThd62Buh/Vv6VyCBD2bdyWcGNQqoQ==",
+      "dev": true,
+      "requires": {
+        "array-union": "^2.1.0",
+        "dir-glob": "^3.0.1",
+        "fast-glob": "^3.1.1",
+        "ignore": "^5.1.4",
+        "merge2": "^1.3.0",
+        "slash": "^3.0.0"
+      },
+      "dependencies": {
+        "ignore": {
+          "version": "5.1.8",
+          "resolved": "https://registry.npmjs.org/ignore/-/ignore-5.1.8.tgz",
+          "integrity": "sha512-BMpfD7PpiETpBl/A6S498BaIJ6Y/ABT93ETbby2fP00v4EbvPBXWEoaR1UBPKs3iR53pJY7EtZk5KACI57i1Uw==",
+          "dev": true
+        }
+      }
+    },
     "graceful-fs": {
       "version": "4.2.4",
       "resolved": "https://registry.npmjs.org/graceful-fs/-/graceful-fs-4.2.4.tgz",
       "integrity": "sha512-WjKPNJF79dtJAVniUlGGWHYGz2jWxT6VhN/4m1NdkbZ2nOsEF+cI1Edgql5zCRhs/VsQYRvrXctxktVXZUkixw==",
       "dev": true
     },
+    "growl": {
+      "version": "1.10.5",
+      "resolved": "https://registry.npmjs.org/growl/-/growl-1.10.5.tgz",
+      "integrity": "sha512-qBr4OuELkhPenW6goKVXiv47US3clb3/IbuWF9KNKEijAy9oeHxU9IgzjvJhHkUzhaj7rOUD7+YGWqUjLp5oSA==",
+      "dev": true
+    },
     "has": {
       "version": "1.0.3",
       "resolved": "https://registry.npmjs.org/has/-/has-1.0.3.tgz",
@@ -1170,6 +4958,12 @@
       "integrity": "sha512-PLcsoqu++dmEIZB+6totNFKq/7Do+Z0u4oT0zKOJNl3lYK6vGwwu2hjHs+68OEZbTjiUE9bgOABXbP/GvrS0Kg==",
       "dev": true
     },
+    "he": {
+      "version": "1.2.0",
+      "resolved": "https://registry.npmjs.org/he/-/he-1.2.0.tgz",
+      "integrity": "sha512-F/1DnUGPopORZi0ni+CvrCgHQ5FyEAHRLSApuYWMmrbSwoN2Mn/7k+Gl38gJnR7yyDZk6WLXwiGod1JOWNDKGw==",
+      "dev": true
+    },
     "hosted-git-info": {
       "version": "2.8.8",
       "resolved": "https://registry.npmjs.org/hosted-git-info/-/hosted-git-info-2.8.8.tgz",
@@ -1245,6 +5039,15 @@
       "integrity": "sha1-d8mYQFJ6qOyxqLppe4BkWnqSap0=",
       "dev": true
     },
+    "is-binary-path": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/is-binary-path/-/is-binary-path-2.1.0.tgz",
+      "integrity": "sha512-ZMERYes6pDydyuGidse7OsHxtbI7WVeUEozgR/g7rd0xUimYNlvZRE/K2MgZTjWy725IfelLeVcEM97mmtRGXw==",
+      "dev": true,
+      "requires": {
+        "binary-extensions": "^2.0.0"
+      }
+    },
     "is-callable": {
       "version": "1.2.0",
       "resolved": "https://registry.npmjs.org/is-callable/-/is-callable-1.2.0.tgz",
@@ -1278,6 +5081,18 @@
         "is-extglob": "^2.1.1"
       }
     },
+    "is-number": {
+      "version": "7.0.0",
+      "resolved": "https://registry.npmjs.org/is-number/-/is-number-7.0.0.tgz",
+      "integrity": "sha512-41Cifkg6e8TylSpdtTpeLVMqvSBEVzTttHvERD741+pnZ8ANv0004MRL43QKPDlK9cGvNp6NZWZUBlbGXYxxng==",
+      "dev": true
+    },
+    "is-plain-obj": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/is-plain-obj/-/is-plain-obj-2.1.0.tgz",
+      "integrity": "sha512-YWnfyRwxL/+SsrWYfOpUtz5b3YD+nyfkHvjbcanzk8zgyO4ASD67uVMRt8k5bM4lLMDnXfriRhOpemw+NfT1eA==",
+      "dev": true
+    },
     "is-regex": {
       "version": "1.1.0",
       "resolved": "https://registry.npmjs.org/is-regex/-/is-regex-1.1.0.tgz",
@@ -1405,6 +5220,66 @@
       "integrity": "sha512-JNvd8XER9GQX0v2qJgsaN/mzFCNA5BRe/j8JN9d+tWyGLSodKQHKFicdwNYzWwI3wjRnaKPsGj1XkBjx/F96DQ==",
       "dev": true
     },
+    "log-symbols": {
+      "version": "4.0.0",
+      "resolved": "https://registry.npmjs.org/log-symbols/-/log-symbols-4.0.0.tgz",
+      "integrity": "sha512-FN8JBzLx6CzeMrB0tg6pqlGU1wCrXW+ZXGH481kfsBqer0hToTIiHdjH4Mq8xJUbvATujKCvaREGWpGUionraA==",
+      "dev": true,
+      "requires": {
+        "chalk": "^4.0.0"
+      },
+      "dependencies": {
+        "ansi-styles": {
+          "version": "4.3.0",
+          "resolved": "https://registry.npmjs.org/ansi-styles/-/ansi-styles-4.3.0.tgz",
+          "integrity": "sha512-zbB9rCJAT1rbjiVDb2hqKFHNYLxgtk8NURxZ3IZwD3F6NtxbXZQCnnSi1Lkx+IDohdPlFp222wVALIheZJQSEg==",
+          "dev": true,
+          "requires": {
+            "color-convert": "^2.0.1"
+          }
+        },
+        "chalk": {
+          "version": "4.1.0",
+          "resolved": "https://registry.npmjs.org/chalk/-/chalk-4.1.0.tgz",
+          "integrity": "sha512-qwx12AxXe2Q5xQ43Ac//I6v5aXTipYrSESdOgzrN+9XjgEpyjpKuvSGaN4qE93f7TQTlerQQ8S+EQ0EyDoVL1A==",
+          "dev": true,
+          "requires": {
+            "ansi-styles": "^4.1.0",
+            "supports-color": "^7.1.0"
+          }
+        },
+        "color-convert": {
+          "version": "2.0.1",
+          "resolved": "https://registry.npmjs.org/color-convert/-/color-convert-2.0.1.tgz",
+          "integrity": "sha512-RRECPsj7iu/xb5oKYcsFHSppFNnsj/52OVTRKb4zP5onXwVF3zVmmToNcOfGC+CRDpfK/U584fMg38ZHCaElKQ==",
+          "dev": true,
+          "requires": {
+            "color-name": "~1.1.4"
+          }
+        },
+        "color-name": {
+          "version": "1.1.4",
+          "resolved": "https://registry.npmjs.org/color-name/-/color-name-1.1.4.tgz",
+          "integrity": "sha512-dOy+3AuW3a2wNbZHIuMZpTcgjGuLU/uBL/ubcZF9OXbDo8ff4O8yVp5Bf0efS8uEoYo5q4Fx7dY9OgQGXgAsQA==",
+          "dev": true
+        },
+        "has-flag": {
+          "version": "4.0.0",
+          "resolved": "https://registry.npmjs.org/has-flag/-/has-flag-4.0.0.tgz",
+          "integrity": "sha512-EykJT/Q1KjTWctppgIAgfSO0tKVuZUjhgMr17kqTumMl6Afv3EISleU7qZUzoXDFTAHTDC4NOoG/ZxU3EvlMPQ==",
+          "dev": true
+        },
+        "supports-color": {
+          "version": "7.2.0",
+          "resolved": "https://registry.npmjs.org/supports-color/-/supports-color-7.2.0.tgz",
+          "integrity": "sha512-qpCAvRl9stuOHveKsn7HncJRvv501qIacKzQlO/+Lwxc9+0q2wLyv4Dfvt80/DPn2pqOBsJdDiogXGR9+OvwRw==",
+          "dev": true,
+          "requires": {
+            "has-flag": "^4.0.0"
+          }
+        }
+      }
+    },
     "loose-envify": {
       "version": "1.4.0",
       "resolved": "https://registry.npmjs.org/loose-envify/-/loose-envify-1.4.0.tgz",
@@ -1414,6 +5289,31 @@
         "js-tokens": "^3.0.0 || ^4.0.0"
       }
     },
+    "lru-cache": {
+      "version": "6.0.0",
+      "resolved": "https://registry.npmjs.org/lru-cache/-/lru-cache-6.0.0.tgz",
+      "integrity": "sha512-Jo6dJ04CmSjuznwJSS3pUeWmd/H0ffTlkXXgwZi+eq1UCmqQwCh+eLsYOYCwY991i2Fah4h1BEMCx4qThGbsiA==",
+      "dev": true,
+      "requires": {
+        "yallist": "^4.0.0"
+      }
+    },
+    "merge2": {
+      "version": "1.4.1",
+      "resolved": "https://registry.npmjs.org/merge2/-/merge2-1.4.1.tgz",
+      "integrity": "sha512-8q7VEgMJW4J8tcfVPy8g09NcQwZdbwFEqhe/WZkoIzjn/3TGDwtOCYtXGxA3O8tPzpczCCDgv+P2P5y00ZJOOg==",
+      "dev": true
+    },
+    "micromatch": {
+      "version": "4.0.2",
+      "resolved": "https://registry.npmjs.org/micromatch/-/micromatch-4.0.2.tgz",
+      "integrity": "sha512-y7FpHSbMUMoyPbYUSzO6PaZ6FyRnQOpHuKwbo1G+Knck95XVU4QAiKdGEnj5wwoS7PlOgthX/09u5iFJ+aYf5Q==",
+      "dev": true,
+      "requires": {
+        "braces": "^3.0.1",
+        "picomatch": "^2.0.5"
+      }
+    },
     "minimatch": {
       "version": "3.0.4",
       "resolved": "https://registry.npmjs.org/minimatch/-/minimatch-3.0.4.tgz",
@@ -1438,6 +5338,105 @@
         "minimist": "^1.2.5"
       }
     },
+    "mocha": {
+      "version": "8.2.1",
+      "resolved": "https://registry.npmjs.org/mocha/-/mocha-8.2.1.tgz",
+      "integrity": "sha512-cuLBVfyFfFqbNR0uUKbDGXKGk+UDFe6aR4os78XIrMQpZl/nv7JYHcvP5MFIAb374b2zFXsdgEGwmzMtP0Xg8w==",
+      "dev": true,
+      "requires": {
+        "@ungap/promise-all-settled": "1.1.2",
+        "ansi-colors": "4.1.1",
+        "browser-stdout": "1.3.1",
+        "chokidar": "3.4.3",
+        "debug": "4.2.0",
+        "diff": "4.0.2",
+        "escape-string-regexp": "4.0.0",
+        "find-up": "5.0.0",
+        "glob": "7.1.6",
+        "growl": "1.10.5",
+        "he": "1.2.0",
+        "js-yaml": "3.14.0",
+        "log-symbols": "4.0.0",
+        "minimatch": "3.0.4",
+        "ms": "2.1.2",
+        "nanoid": "3.1.12",
+        "serialize-javascript": "5.0.1",
+        "strip-json-comments": "3.1.1",
+        "supports-color": "7.2.0",
+        "which": "2.0.2",
+        "wide-align": "1.1.3",
+        "workerpool": "6.0.2",
+        "yargs": "13.3.2",
+        "yargs-parser": "13.1.2",
+        "yargs-unparser": "2.0.0"
+      },
+      "dependencies": {
+        "escape-string-regexp": {
+          "version": "4.0.0",
+          "resolved": "https://registry.npmjs.org/escape-string-regexp/-/escape-string-regexp-4.0.0.tgz",
+          "integrity": "sha512-TtpcNJ3XAzx3Gq8sWRzJaVajRs0uVxA2YAkdb1jm2YkPz4G6egUFAyA3n5vtEIZefPk5Wa4UXbKuS5fKkJWdgA==",
+          "dev": true
+        },
+        "find-up": {
+          "version": "5.0.0",
+          "resolved": "https://registry.npmjs.org/find-up/-/find-up-5.0.0.tgz",
+          "integrity": "sha512-78/PXT1wlLLDgTzDs7sjq9hzz0vXD+zn+7wypEe4fXQxCmdmqfGsEPQxmiCSQI3ajFV91bVSsvNtrJRiW6nGng==",
+          "dev": true,
+          "requires": {
+            "locate-path": "^6.0.0",
+            "path-exists": "^4.0.0"
+          }
+        },
+        "has-flag": {
+          "version": "4.0.0",
+          "resolved": "https://registry.npmjs.org/has-flag/-/has-flag-4.0.0.tgz",
+          "integrity": "sha512-EykJT/Q1KjTWctppgIAgfSO0tKVuZUjhgMr17kqTumMl6Afv3EISleU7qZUzoXDFTAHTDC4NOoG/ZxU3EvlMPQ==",
+          "dev": true
+        },
+        "locate-path": {
+          "version": "6.0.0",
+          "resolved": "https://registry.npmjs.org/locate-path/-/locate-path-6.0.0.tgz",
+          "integrity": "sha512-iPZK6eYjbxRu3uB4/WZ3EsEIMJFMqAoopl3R+zuq0UjcAm/MO6KCweDgPfP3elTztoKP3KtnVHxTn2NHBSDVUw==",
+          "dev": true,
+          "requires": {
+            "p-locate": "^5.0.0"
+          }
+        },
+        "p-limit": {
+          "version": "3.1.0",
+          "resolved": "https://registry.npmjs.org/p-limit/-/p-limit-3.1.0.tgz",
+          "integrity": "sha512-TYOanM3wGwNGsZN2cVTYPArw454xnXj5qmWF1bEoAc4+cU/ol7GVh7odevjp1FNHduHc3KZMcFduxU5Xc6uJRQ==",
+          "dev": true,
+          "requires": {
+            "yocto-queue": "^0.1.0"
+          }
+        },
+        "p-locate": {
+          "version": "5.0.0",
+          "resolved": "https://registry.npmjs.org/p-locate/-/p-locate-5.0.0.tgz",
+          "integrity": "sha512-LaNjtRWUBY++zB5nE/NwcaoMylSPk+S+ZHNB1TzdbMJMny6dynpAGt7X/tl/QYq3TIeE6nxHppbo2LGymrG5Pw==",
+          "dev": true,
+          "requires": {
+            "p-limit": "^3.0.2"
+          }
+        },
+        "path-exists": {
+          "version": "4.0.0",
+          "resolved": "https://registry.npmjs.org/path-exists/-/path-exists-4.0.0.tgz",
+          "integrity": "sha512-ak9Qy5Q7jYb2Wwcey5Fpvg2KoAc/ZIhLSLOSBmRmygPsGwkVVt0fZa0qrtMz+m6tJTAHfZQ8FnmB4MG4LWy7/w==",
+          "dev": true
+        },
+        "supports-color": {
+          "version": "7.2.0",
+          "resolved": "https://registry.npmjs.org/supports-color/-/supports-color-7.2.0.tgz",
+          "integrity": "sha512-qpCAvRl9stuOHveKsn7HncJRvv501qIacKzQlO/+Lwxc9+0q2wLyv4Dfvt80/DPn2pqOBsJdDiogXGR9+OvwRw==",
+          "dev": true,
+          "requires": {
+            "has-flag": "^4.0.0"
+          }
+        }
+      }
+    },
     "ms": {
       "version": "2.1.2",
       "resolved": "https://registry.npmjs.org/ms/-/ms-2.1.2.tgz",
@@ -1453,6 +5452,12 @@
         "lodash": "^4.0.0"
       }
     },
+    "nanoid": {
+      "version": "3.1.12",
+      "resolved": "https://registry.npmjs.org/nanoid/-/nanoid-3.1.12.tgz",
+      "integrity": "sha512-1qstj9z5+x491jfiC4Nelk+f8XBad7LN20PmyWINJEMRSf3wcAjAWysw1qaA8z6NSKe2sjq1hRSDpBH5paCb6A==",
+      "dev": true
+    },
     "natural-compare": {
       "version": "1.4.0",
       "resolved": "https://registry.npmjs.org/natural-compare/-/natural-compare-1.4.0.tgz",
@@ -1471,6 +5476,12 @@
         "validate-npm-package-license": "^3.0.1"
       }
     },
+    "normalize-path": {
+      "version": "3.0.0",
+      "resolved": "https://registry.npmjs.org/normalize-path/-/normalize-path-3.0.0.tgz",
+      "integrity": "sha512-6eZs5Ls3WtCisHWp9S2GUy8dqkpGi4BVSz3GaqiE6ezub0512ESztXUwUB6C6IKbQkY2Pnb/mD4WYojCRwcwLA==",
+      "dev": true
+    },
     "object-assign": {
       "version": "4.1.1",
       "resolved": "https://registry.npmjs.org/object-assign/-/object-assign-4.1.1.tgz",
@@ -1634,6 +5645,12 @@
         "pify": "^2.0.0"
       }
     },
+    "picomatch": {
+      "version": "2.2.2",
+      "resolved": "https://registry.npmjs.org/picomatch/-/picomatch-2.2.2.tgz",
+      "integrity": "sha512-q0M/9eZHzmr0AulXyPwNfZjtwZ/RBZlbN3K3CErVrk50T2ASYI7Bye0EvekFY3IP1Nt2DHu0re+V2ZHIpMkuWg==",
+      "dev": true
+    },
     "pify": {
       "version": "2.3.0",
       "resolved": "https://registry.npmjs.org/pify/-/pify-2.3.0.tgz",
@@ -1693,6 +5710,15 @@
       "integrity": "sha512-XRsRjdf+j5ml+y/6GKHPZbrF/8p2Yga0JPtdqTIY2Xe5ohJPD9saDJJLPvp9+NSBprVvevdXZybnj2cv8OEd0A==",
       "dev": true
     },
+    "randombytes": {
+      "version": "2.1.0",
+      "resolved": "https://registry.npmjs.org/randombytes/-/randombytes-2.1.0.tgz",
+      "integrity": "sha512-vYl3iOX+4CKUWuxGi9Ukhie6fsqXqS9FE2Zaic4tNFD2N2QQaXOMFbuKK4QmDHC0JO6B1Zp41J0LpT0oR68amQ==",
+      "dev": true,
+      "requires": {
+        "safe-buffer": "^5.1.0"
+      }
+    },
     "react-is": {
       "version": "16.13.1",
       "resolved": "https://registry.npmjs.org/react-is/-/react-is-16.13.1.tgz",
@@ -1731,6 +5757,15 @@
         "util-deprecate": "^1.0.1"
       }
     },
+    "readdirp": {
+      "version": "3.5.0",
+      "resolved": "https://registry.npmjs.org/readdirp/-/readdirp-3.5.0.tgz",
+      "integrity": "sha512-cMhu7c/8rdhkHXWsY+osBhfSy0JikwpHK/5+imo+LpeasTF8ouErHrlYkwT0++njiyuDvc7OFY5T3ukvZ8qmFQ==",
+      "dev": true,
+      "requires": {
+        "picomatch": "^2.2.1"
+      }
+    },
     "regenerator-runtime": {
       "version": "0.13.7",
       "resolved": "https://registry.npmjs.org/regenerator-runtime/-/regenerator-runtime-0.13.7.tgz",
@@ -1753,6 +5788,18 @@
       "integrity": "sha512-ZOIzd8yVsQQA7j8GCSlPGXwg5PfmA1mrq0JP4nGhh54LaKN3xdai/vHUDu74pKwV8OxseMS65u2NImosQcSD0Q==",
       "dev": true
     },
+    "require-directory": {
+      "version": "2.1.1",
+      "resolved": "https://registry.npmjs.org/require-directory/-/require-directory-2.1.1.tgz",
+      "integrity": "sha1-jGStX9MNqxyXbiNE/+f3kqam30I=",
+      "dev": true
+    },
+    "require-main-filename": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/require-main-filename/-/require-main-filename-2.0.0.tgz",
+      "integrity": "sha512-NKN5kMDylKuldxYLSUfrbo5Tuzh4hd+2E8NPPX02mZtn1VuREQToYe/ZdlJy+J3uCpfaiGF05e7B8W0iXbQHmg==",
+      "dev": true
+    },
     "resolve": {
       "version": "1.17.0",
       "resolved": "https://registry.npmjs.org/resolve/-/resolve-1.17.0.tgz",
@@ -1768,6 +5815,12 @@
       "integrity": "sha512-pb/MYmXstAkysRFx8piNI1tGFNQIFA3vkE3Gq4EuA1dF6gHp/+vgZqsCGJapvy8N3Q+4o7FwvquPJcnZ7RYy4g==",
       "dev": true
     },
+    "reusify": {
+      "version": "1.0.4",
+      "resolved": "https://registry.npmjs.org/reusify/-/reusify-1.0.4.tgz",
+      "integrity": "sha512-U9nH88a3fc/ekCF1l0/UP1IosiuIjyTh7hBvXVMHYgVcfGvt897Xguj2UOLDeI5BG2m7/uwyaLVT6fbtCwTyzw==",
+      "dev": true
+    },
     "rimraf": {
       "version": "2.6.3",
       "resolved": "https://registry.npmjs.org/rimraf/-/rimraf-2.6.3.tgz",
@@ -1777,6 +5830,12 @@
         "glob": "^7.1.3"
       }
     },
+    "run-parallel": {
+      "version": "1.1.10",
+      "resolved": "https://registry.npmjs.org/run-parallel/-/run-parallel-1.1.10.tgz",
+      "integrity": "sha512-zb/1OuZ6flOlH6tQyMPUrE3x3Ulxjlo9WIVXR4yVYi4H9UXQaeIsPbLn2R3O3vQCnDKkAl2qHiuocKKX4Tz/Sw==",
+      "dev": true
+    },
     "safe-buffer": {
       "version": "5.1.2",
       "resolved": "https://registry.npmjs.org/safe-buffer/-/safe-buffer-5.1.2.tgz",
@@ -1795,6 +5854,21 @@
       "integrity": "sha512-sauaDf/PZdVgrLTNYHRtpXa1iRiKcaebiKQ1BJdpQlWH2lCvexQdX55snPFyK7QzpudqbCI0qXFfOasHdyNDGQ==",
       "dev": true
     },
+    "serialize-javascript": {
+      "version": "5.0.1",
+      "resolved": "https://registry.npmjs.org/serialize-javascript/-/serialize-javascript-5.0.1.tgz",
+      "integrity": "sha512-SaaNal9imEO737H2c05Og0/8LUXG7EnsZyMa8MzkmuHoELfT6txuj0cMqRj6zfPKnmQ1yasR4PCJc8x+M4JSPA==",
+      "dev": true,
+      "requires": {
+        "randombytes": "^2.1.0"
+      }
+    },
+    "set-blocking": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/set-blocking/-/set-blocking-2.0.0.tgz",
+      "integrity": "sha1-BF+XgtARrppoA93TgrJDkrPYkPc=",
+      "dev": true
+    },
     "shebang-command": {
       "version": "2.0.0",
       "resolved": "https://registry.npmjs.org/shebang-command/-/shebang-command-2.0.0.tgz",
@@ -1820,6 +5894,12 @@
         "object-inspect": "^1.7.0"
       }
     },
+    "slash": {
+      "version": "3.0.0",
+      "resolved": "https://registry.npmjs.org/slash/-/slash-3.0.0.tgz",
+      "integrity": "sha512-g9Q1haeby36OSStwb4ntCGGGaKsaVSjQ68fBxoQcutl5fS1vuY18H3wSt3jFyFtrkx+Kz0V1G85A4MyAdDMi2Q==",
+      "dev": true
+    },
     "slice-ansi": {
       "version": "2.1.0",
       "resolved": "https://registry.npmjs.org/slice-ansi/-/slice-ansi-2.1.0.tgz",
@@ -1875,6 +5955,23 @@
       "integrity": "sha1-BOaSb2YolTVPPdAVIDYzuFcpfiw=",
       "dev": true
     },
+    "string_decoder": {
+      "version": "1.3.0",
+      "resolved": "https://registry.npmjs.org/string_decoder/-/string_decoder-1.3.0.tgz",
+      "integrity": "sha512-hkRX8U1WjJFd8LsDJ2yQ/wWWxaopEsABU1XfkM8A+j0+85JAGppt16cr1Whg6KIbb4okU6Mql6BOj+uup/wKeA==",
+      "dev": true,
+      "requires": {
+        "safe-buffer": "~5.2.0"
+      },
+      "dependencies": {
+        "safe-buffer": {
+          "version": "5.2.1",
+          "resolved": "https://registry.npmjs.org/safe-buffer/-/safe-buffer-5.2.1.tgz",
+          "integrity": "sha512-rp3So07KcdmmKbGvgaNxQSJr7bGVSVk5S9Eq1F+ppbRo70+YeaDxkw5Dd8NPN+GD6bjnYm2VuPuCXmpuYvmCXQ==",
+          "dev": true
+        }
+      }
+    },
     "string-width": {
       "version": "3.1.0",
       "resolved": "https://registry.npmjs.org/string-width/-/string-width-3.1.0.tgz",
@@ -1937,23 +6034,6 @@
         "es-abstract": "^1.17.5"
       }
     },
-    "string_decoder": {
-      "version": "1.3.0",
-      "resolved": "https://registry.npmjs.org/string_decoder/-/string_decoder-1.3.0.tgz",
-      "integrity": "sha512-hkRX8U1WjJFd8LsDJ2yQ/wWWxaopEsABU1XfkM8A+j0+85JAGppt16cr1Whg6KIbb4okU6Mql6BOj+uup/wKeA==",
-      "dev": true,
-      "requires": {
-        "safe-buffer": "~5.2.0"
-      },
-      "dependencies": {
-        "safe-buffer": {
-          "version": "5.2.1",
-          "resolved": "https://registry.npmjs.org/safe-buffer/-/safe-buffer-5.2.1.tgz",
-          "integrity": "sha512-rp3So07KcdmmKbGvgaNxQSJr7bGVSVk5S9Eq1F+ppbRo70+YeaDxkw5Dd8NPN+GD6bjnYm2VuPuCXmpuYvmCXQ==",
-          "dev": true
-        }
-      }
-    },
     "strip-ansi": {
       "version": "6.0.0",
       "resolved": "https://registry.npmjs.org/strip-ansi/-/strip-ansi-6.0.0.tgz",
@@ -2008,6 +6088,15 @@
       "integrity": "sha1-3F5pjL0HkmW8c+A3doGk5Og/YW4=",
       "dev": true
     },
+    "to-regex-range": {
+      "version": "5.0.1",
+      "resolved": "https://registry.npmjs.org/to-regex-range/-/to-regex-range-5.0.1.tgz",
+      "integrity": "sha512-65P7iz6X5yEr1cwcgvQxbbIw7Uk3gOy5dIdtZ4rDveLqhrdJP+Li/Hx6tyK0NEb+2GCyneCMJiGqrADCSNk8sQ==",
+      "dev": true,
+      "requires": {
+        "is-number": "^7.0.0"
+      }
+    },
     "tslib": {
       "version": "1.13.0",
       "resolved": "https://registry.npmjs.org/tslib/-/tslib-1.13.0.tgz",
@@ -2038,6 +6127,12 @@
       "integrity": "sha512-4dbzIzqvjtgiM5rw1k5rEHtBANKmdudhGyBEajN01fEyhaAIhsoKNy6y7+IN93IfpFtwY9iqi7kD+xwKhQsNJA==",
       "dev": true
     },
+    "typescript": {
+      "version": "4.1.3",
+      "resolved": "https://registry.npmjs.org/typescript/-/typescript-4.1.3.tgz",
+      "integrity": "sha512-B3ZIOf1IKeH2ixgHhj6la6xdwR9QrLC5d1VKeCSY4tvkqhF2eqd9O7txNlS0PO3GrBAFIdr3L1ndNwteUbZLYg==",
+      "dev": true
+    },
     "uri-js": {
       "version": "4.4.0",
       "resolved": "https://registry.npmjs.org/uri-js/-/uri-js-4.4.0.tgz",
@@ -2078,12 +6173,88 @@
         "isexe": "^2.0.0"
       }
     },
+    "which-module": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/which-module/-/which-module-2.0.0.tgz",
+      "integrity": "sha1-2e8H3Od7mQK4o6j6SzHD4/fm6Ho=",
+      "dev": true
+    },
+    "wide-align": {
+      "version": "1.1.3",
+      "resolved": "https://registry.npmjs.org/wide-align/-/wide-align-1.1.3.tgz",
+      "integrity": "sha512-QGkOQc8XL6Bt5PwnsExKBPuMKBxnGxWWW3fU55Xt4feHozMUhdUMaBCk290qpm/wG5u/RSKzwdAC4i51YigihA==",
+      "dev": true,
+      "requires": {
+        "string-width": "^1.0.2 || 2"
+      },
+      "dependencies": {
+        "ansi-regex": {
+          "version": "3.0.0",
+          "resolved": "https://registry.npmjs.org/ansi-regex/-/ansi-regex-3.0.0.tgz",
+          "integrity": "sha1-7QMXwyIGT3lGbAKWa922Bas32Zg=",
+          "dev": true
+        },
+        "string-width": {
+          "version": "2.1.1",
+          "resolved": "https://registry.npmjs.org/string-width/-/string-width-2.1.1.tgz",
+          "integrity": "sha512-nOqH59deCq9SRHlxq1Aw85Jnt4w6KvLKqWVik6oA9ZklXLNIOlqg4F2yrT1MVaTjAqvVwdfeZ7w7aCvJD7ugkw==",
+          "dev": true,
+          "requires": {
+            "is-fullwidth-code-point": "^2.0.0",
+            "strip-ansi": "^4.0.0"
+          }
+        },
+        "strip-ansi": {
+          "version": "4.0.0",
+          "resolved": "https://registry.npmjs.org/strip-ansi/-/strip-ansi-4.0.0.tgz",
+          "integrity": "sha1-qEeQIusaw2iocTibY1JixQXuNo8=",
+          "dev": true,
+          "requires": {
+            "ansi-regex": "^3.0.0"
+          }
+        }
+      }
+    },
     "word-wrap": {
       "version": "1.2.3",
       "resolved": "https://registry.npmjs.org/word-wrap/-/word-wrap-1.2.3.tgz",
       "integrity": "sha512-Hz/mrNwitNRh/HUAtM/VT/5VH+ygD6DV7mYKZAtHOrbs8U7lvPS6xf7EJKMF0uW1KJCl0H701g3ZGus+muE5vQ==",
       "dev": true
     },
+    "workerpool": {
+      "version": "6.0.2",
+      "resolved": "https://registry.npmjs.org/workerpool/-/workerpool-6.0.2.tgz",
+      "integrity": "sha512-DSNyvOpFKrNusaaUwk+ej6cBj1bmhLcBfj80elGk+ZIo5JSkq+unB1dLKEOcNfJDZgjGICfhQ0Q5TbP0PvF4+Q==",
+      "dev": true
+    },
+    "wrap-ansi": {
+      "version": "5.1.0",
+      "resolved": "https://registry.npmjs.org/wrap-ansi/-/wrap-ansi-5.1.0.tgz",
+      "integrity": "sha512-QC1/iN/2/RPVJ5jYK8BGttj5z83LmSKmvbvrXPNCLZSEb32KKVDJDl/MOt2N01qU2H/FkzEa9PKto1BqDjtd7Q==",
+      "dev": true,
+      "requires": {
+        "ansi-styles": "^3.2.0",
+        "string-width": "^3.0.0",
+        "strip-ansi": "^5.0.0"
+      },
+      "dependencies": {
+        "ansi-regex": {
+          "version": "4.1.0",
+          "resolved": "https://registry.npmjs.org/ansi-regex/-/ansi-regex-4.1.0.tgz",
+          "integrity": "sha512-1apePfXM1UOSqw0o9IiFAovVz9M5S1Dg+4TrDwfMewQ6p/rmMueb7tWZjQ1rx4Loy1ArBggoqGpfqqdI4rondg==",
+          "dev": true
+        },
+        "strip-ansi": {
+          "version": "5.2.0",
+          "resolved": "https://registry.npmjs.org/strip-ansi/-/strip-ansi-5.2.0.tgz",
+          "integrity": "sha512-DuRs1gKbBqsMKIZlrffwlug8MHkcnpjs5VPmL1PAh+mA30U0DTotfDZ0d2UUsXpPmPmMMJ6W773MaA3J+lbiWA==",
+          "dev": true,
+          "requires": {
+            "ansi-regex": "^4.1.0"
+          }
+        }
+      }
+    },
     "wrappy": {
       "version": "1.0.2",
       "resolved": "https://registry.npmjs.org/wrappy/-/wrappy-1.0.2.tgz",
@@ -2099,11 +6270,128 @@
         "mkdirp": "^0.5.1"
       }
     },
+    "y18n": {
+      "version": "4.0.1",
+      "resolved": "https://registry.npmjs.org/y18n/-/y18n-4.0.1.tgz",
+      "integrity": "sha512-wNcy4NvjMYL8gogWWYAO7ZFWFfHcbdbE57tZO8e4cbpj8tfUcwrwqSl3ad8HxpYWCdXcJUCeKKZS62Av1affwQ==",
+      "dev": true
+    },
+    "yallist": {
+      "version": "4.0.0",
+      "resolved": "https://registry.npmjs.org/yallist/-/yallist-4.0.0.tgz",
+      "integrity": "sha512-3wdGidZyq5PB084XLES5TpOSRA3wjXAlIWMhum2kRcv/41Sn2emQ0dycQW4uZXLejwKvg6EsvbdlVL+FYEct7A==",
+      "dev": true
+    },
+    "yargs": {
+      "version": "13.3.2",
+      "resolved": "https://registry.npmjs.org/yargs/-/yargs-13.3.2.tgz",
+      "integrity": "sha512-AX3Zw5iPruN5ie6xGRIDgqkT+ZhnRlZMLMHAs8tg7nRruy2Nb+i5o9bwghAogtM08q1dpr2LVoS8KSTMYpWXUw==",
+      "dev": true,
+      "requires": {
+        "cliui": "^5.0.0",
+        "find-up": "^3.0.0",
+        "get-caller-file": "^2.0.1",
+        "require-directory": "^2.1.1",
+        "require-main-filename": "^2.0.0",
+        "set-blocking": "^2.0.0",
+        "string-width": "^3.0.0",
+        "which-module": "^2.0.0",
+        "y18n": "^4.0.0",
+        "yargs-parser": "^13.1.2"
+      },
+      "dependencies": {
+        "find-up": {
+          "version": "3.0.0",
+          "resolved": "https://registry.npmjs.org/find-up/-/find-up-3.0.0.tgz",
+          "integrity": "sha512-1yD6RmLI1XBfxugvORwlck6f75tYL+iR0jqwsOrOxMZyGYqUuDhJ0l4AXdO1iX/FTs9cBAMEk1gWSEx1kSbylg==",
+          "dev": true,
+          "requires": {
+            "locate-path": "^3.0.0"
+          }
+        },
+        "locate-path": {
+          "version": "3.0.0",
+          "resolved": "https://registry.npmjs.org/locate-path/-/locate-path-3.0.0.tgz",
+          "integrity": "sha512-7AO748wWnIhNqAuaty2ZWHkQHRSNfPVIsPIfwEOWO22AmaoVrWavlOcMR5nzTLNYvp36X220/maaRsrec1G65A==",
+          "dev": true,
+          "requires": {
+            "p-locate": "^3.0.0",
+            "path-exists": "^3.0.0"
+          }
+        },
+        "p-limit": {
+          "version": "2.3.0",
+          "resolved": "https://registry.npmjs.org/p-limit/-/p-limit-2.3.0.tgz",
+          "integrity": "sha512-//88mFWSJx8lxCzwdAABTJL2MyWB12+eIY7MDL2SqLmAkeKU9qxRvWuSyTjm3FUmpBEMuFfckAIqEaVGUDxb6w==",
+          "dev": true,
+          "requires": {
+            "p-try": "^2.0.0"
+          }
+        },
+        "p-locate": {
+          "version": "3.0.0",
+          "resolved": "https://registry.npmjs.org/p-locate/-/p-locate-3.0.0.tgz",
+          "integrity": "sha512-x+12w/To+4GFfgJhBEpiDcLozRJGegY+Ei7/z0tSLkMmxGZNybVMSfWj9aJn8Z5Fc7dBUNJOOVgPv2H7IwulSQ==",
+          "dev": true,
+          "requires": {
+            "p-limit": "^2.0.0"
+          }
+        },
+        "p-try": {
+          "version": "2.2.0",
+          "resolved": "https://registry.npmjs.org/p-try/-/p-try-2.2.0.tgz",
+          "integrity": "sha512-R4nPAVTAU0B9D35/Gk3uJf/7XYbQcyohSKdvAxIRSNghFl4e71hVoGnBNQz9cWaXxO2I10KTC+3jMdvvoKw6dQ==",
+          "dev": true
+        }
+      }
+    },
+    "yargs-parser": {
+      "version": "13.1.2",
+      "resolved": "https://registry.npmjs.org/yargs-parser/-/yargs-parser-13.1.2.tgz",
+      "integrity": "sha512-3lbsNRf/j+A4QuSZfDRA7HRSfWrzO0YjqTJd5kjAq37Zep1CEgaYmrH9Q3GwPiB9cHyd1Y1UwggGhJGoxipbzg==",
+      "dev": true,
+      "requires": {
+        "camelcase": "^5.0.0",
+        "decamelize": "^1.2.0"
+      }
+    },
+    "yargs-unparser": {
+      "version": "2.0.0",
+      "resolved": "https://registry.npmjs.org/yargs-unparser/-/yargs-unparser-2.0.0.tgz",
+      "integrity": "sha512-7pRTIA9Qc1caZ0bZ6RYRGbHJthJWuakf+WmHK0rVeLkNrrGhfoabBNdue6kdINI6r4if7ocq9aD/n7xwKOdzOA==",
+      "dev": true,
+      "requires": {
+        "camelcase": "^6.0.0",
+        "decamelize": "^4.0.0",
+        "flat": "^5.0.2",
+        "is-plain-obj": "^2.1.0"
+      },
+      "dependencies": {
+        "camelcase": {
+          "version": "6.2.0",
+          "resolved": "https://registry.npmjs.org/camelcase/-/camelcase-6.2.0.tgz",
+          "integrity": "sha512-c7wVvbw3f37nuobQNtgsgG9POC9qMbNuMQmTCqZv23b6MIz0fcYpBiOlv9gEN/hdLdnZTDQhg6e9Dq5M1vKvfg==",
+          "dev": true
+        },
+        "decamelize": {
+          "version": "4.0.0",
+          "resolved": "https://registry.npmjs.org/decamelize/-/decamelize-4.0.0.tgz",
+          "integrity": "sha512-9iE1PgSik9HeIIw2JO94IidnE3eBoQrFJ3w7sFuzSX4DpmZ3v5sZpUiV5Swcf6mQEF+Y0ru8Neo+p+nyh2J+hQ==",
+          "dev": true
+        }
+      }
+    },
     "yarn": {
       "version": "1.22.0",
       "resolved": "https://registry.npmjs.org/yarn/-/yarn-1.22.0.tgz",
       "integrity": "sha512-KMHP/Jq53jZKTY9iTUt3dIVl/be6UPs2INo96+BnZHLKxYNTfwMmlgHTaMWyGZoO74RI4AIFvnWhYrXq2USJkg==",
       "dev": true
+    },
+    "yocto-queue": {
+      "version": "0.1.0",
+      "resolved": "https://registry.npmjs.org/yocto-queue/-/yocto-queue-0.1.0.tgz",
+      "integrity": "sha512-rVksvsnNCdJ/ohGc6xgPwyN8eheCxsiLM8mxuE/t/mOVqJewPuO1miLpTHQiRgTKCLexL4MeAFVagts7HmNZ2Q==",
+      "dev": true
     }
   }
 }
diff --git a/package.json b/package.json
--- a/package.json
+++ b/package.json
@@ -3,9 +3,10 @@
   "description": "This package file is for node modules used in mozilla-central",
   "repository": {},
   "license": "MPL-2.0",
-  "dependencies": {},
   "devDependencies": {
     "@babel/core": "7.8.3",
+    "@typescript-eslint/eslint-plugin": "^4.12.0",
+    "@typescript-eslint/parser": "^4.10.0",
     "babel-eslint": "10.1.0",
     "eslint": "7.8.0",
     "eslint-config-prettier": "6.10.0",
@@ -23,6 +24,7 @@
     "eslint-plugin-react": "7.18.3",
     "eslint-plugin-spidermonkey-js": "file:tools/lint/eslint/eslint-plugin-spidermonkey-js",
     "prettier": "1.19.1",
+    "typescript": "^4.1.3",
     "yarn": "1.22.0"
   },
   "notes(private)": "We don't want to publish to npm, so this is marked as private",
diff --git a/toolkit/themes/shared/in-content/common.inc.css b/toolkit/themes/shared/in-content/common.inc.css
--- a/toolkit/themes/shared/in-content/common.inc.css
+++ b/toolkit/themes/shared/in-content/common.inc.css
@@ -152,6 +152,50 @@
   }
 }
 
+html[forcedarkmode] {
+  /* Keep this in sync with layout/base/PresShell.cpp! */
+  --in-content-page-background: #2A2A2E;
+  --in-content-page-color: rgb(249, 249, 250);
+  --in-content-text-color: var(--in-content-page-color);
+  --in-content-deemphasized-text: var(--grey-40);
+  --in-content-box-background: #202023;
+  --in-content-box-background-hover: rgba(249,249,250,0.15);
+  --in-content-box-background-active: rgba(249,249,250,0.2);
+  --in-content-box-background-odd: rgba(249,249,250,0.05);
+  --in-content-box-info-background: rgba(249,249,250,0.15);
+
+  --in-content-border-color: rgba(249,249,250,0.2);
+  --in-content-border-hover: rgba(249,249,250,0.3);
+  --in-content-box-border-color: rgba(249,249,250,0.2);
+
+  --in-content-button-background: rgba(249,249,250,0.1);
+  --in-content-button-background-hover: rgba(249,249,250,0.15);
+  --in-content-button-background-active: rgba(249,249,250,0.2);
+
+  --in-content-category-background-hover: rgba(249,249,250,0.1);
+  --in-content-category-background-active: rgba(249,249,250,0.15);
+  --in-content-category-background-selected-hover: rgba(249,249,250,0.15);
+  --in-content-category-background-selected-active: rgba(249,249,250,0.2);
+
+  --in-content-table-background: #202023;
+  --in-content-table-border-dark-color: rgba(249,249,250,0.2);
+  --in-content-table-header-background: #002b57;
+  --in-content-dialog-header-background: rgba(249,249,250,0.05);
+
+  --in-content-category-text-selected: var(--blue-40);
+  --in-content-category-text-selected-active: var(--blue-50);
+  --in-content-link-color: var(--blue-40);
+  --in-content-link-color-hover: var(--blue-50);
+  --in-content-link-color-active: var(--blue-60);
+  --in-content-link-color-visited: var(--blue-40);
+
+  --in-content-tab-color: var(--in-content-page-color);
+
+  --card-outline-color: var(--grey-60);
+
+  scrollbar-color: rgba(249,249,250,.4) rgba(20,20,25,.3);
+}
+
 :root {
   font: message-box;
   appearance: none;
