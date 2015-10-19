# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  fb04af2d1606919b083dbc87ce33ab21f895ad6a
x

diff --git a/devtools/client/framework/target.js b/devtools/client/framework/target.js
--- a/devtools/client/framework/target.js
+++ b/devtools/client/framework/target.js
@@ -727,16 +727,20 @@ WorkerTarget.prototype = {
   get isTabActor() {
     return true;
   },
 
   get url() {
     return this._workerClient.url;
   },
 
+  get isWorkerTarget() {
+    return true;
+  },
+
   get form() {
     return {};
   },
 
   get activeTab() {
     return this._workerClient;
   },
 
diff --git a/devtools/server/actors/worker.js b/devtools/server/actors/worker.js
--- a/devtools/server/actors/worker.js
+++ b/devtools/server/actors/worker.js
@@ -37,17 +37,17 @@ function WorkerActor(dbg) {
 }
 
 WorkerActor.prototype = {
   actorPrefix: "worker",
 
   form: function () {
     return {
       actor: this.actorID,
-      webConsoleActor: this._webConsoleActor,
+      consoleActor: this._consoleActor,
       url: this._dbg.url,
       type: this._dbg.type
     };
   },
 
   onAttach: function () {
     if (this._dbg.isClosed) {
       return { error: "closed" };
@@ -83,25 +83,25 @@ WorkerActor.prototype = {
       return {
         type: "connected",
         threadActor: this._threadActor
       };
     }
 
     return DebuggerServer.connectToWorker(
       this.conn, this._dbg, this.actorID, request.options
-    ).then(({ threadActor, transport, webConsoleActor }) => {
+    ).then(({ threadActor, transport, consoleActor }) => {
       this._threadActor = threadActor;
       this._transport = transport;
-      this._webConsoleActor = webConsoleActor;
+      this._consoleActor = consoleActor;
 
       return {
         type: "connected",
         threadActor: this._threadActor,
-        webConsoleActor: this._webConsoleActor
+        consoleActor: this._consoleActor
       };
     }, (error) => {
       return { error: error.toString() };
     });
   },
 
   onClose: function () {
     if (this._isAttached) {
diff --git a/devtools/server/main.js b/devtools/server/main.js
--- a/devtools/server/main.js
+++ b/devtools/server/main.js
@@ -828,17 +828,17 @@ var DebuggerServer = {
       aDbg.postMessage(JSON.stringify({
         type: "connect",
         id: aId,
         options: aOptions
       }));
 
       // Steps 3-5 are performed on the worker thread (see worker.js).
       let threadActor;
-      let webConsoleActor;
+      let consoleActor;
 
       // Step 6: Wait for a response from the worker debugger.
       let listener = {
         onClose: () => {
           aDbg.removeListener(listener);
 
           reject("closed");
         },
@@ -852,17 +852,17 @@ var DebuggerServer = {
           message = packet.message;
           if (message.error) {
             reject(error);
           }
 
           // Step 7: Assign the actors that we created in the worker thread.
           if (message.type === "actors-created") {
             threadActor = message.threadActor;
-            webConsoleActor = message.webConsoleActor;
+            consoleActor = message.consoleActor;
           }
 
           if (message.type !== "paused") {
             return;
           }
 
           aDbg.removeListener(listener);
 
@@ -891,17 +891,17 @@ var DebuggerServer = {
 
           // Ensure that any packets received from the client on the main thread
           // to actors on the worker thread are forwarded to the server on the
           // worker thread.
           aConnection.setForwarding(aId, transport);
 
           resolve({
             threadActor: threadActor,
-            webConsoleActor: webConsoleActor,
+            consoleActor: consoleActor,
             transport: transport
           });
         }
       };
       aDbg.addListener(listener);
     });
   },
 
diff --git a/devtools/server/worker.js b/devtools/server/worker.js
--- a/devtools/server/worker.js
+++ b/devtools/server/worker.js
@@ -74,30 +74,30 @@ this.addEventListener("message",  functi
       },
 
       window: global
     };
 
     let threadActor = new ThreadActor(parent, global);
     pool.addActor(threadActor);
 
-    let webConsoleActor = new WebConsoleActor(connection, parent);
+    let consoleActor = new WebConsoleActor(connection, parent);
     pool.addActor(webConsoleActor);
 
     // XXX: USE POST MESSAGE FOR THIS
 
     // postMessage({
     //   type: "connected",
 
 
     
     connection.send({
       type: "actors-created",
       threadActor: threadActor.actorID,
-      webConsoleActor: webConsoleActor.actorID,
+      consoleActor: consoleActor.actorID,
     });
 
     // Step 5: Attach to the thread actor.
     //
     // This will cause a packet to be sent over the connection to the parent.
     // Because this connection uses WorkerDebuggerTransport internally, this
     // packet will be sent using WorkerDebuggerGlobalScope.postMessage, causing
     // an onMessage event to be fired on the WorkerDebugger in the main thread.
diff --git a/devtools/shared/client/main.js b/devtools/shared/client/main.js
--- a/devtools/shared/client/main.js
+++ b/devtools/shared/client/main.js
@@ -1340,29 +1340,29 @@ WorkerClient.prototype = {
     telemetry: "WORKERDETACH"
   }),
 
   attachThread: function(aOptions = {}, aOnResponse = noop) {
     if (this.thread) {
       DevToolsUtils.executeSoon(() => aOnResponse({
         type: "connected",
         threadActor: this.thread._actor,
-        webConsoleActor: this.webConsoleActor,
+        consoleActor: this.consoleActor,
       }, this.thread));
       return;
     }
 
     this.request({
       to: this._actor,
       type: "connect",
       options: aOptions,
     }, (aResponse) => {
       if (!aResponse.error) {
         this.thread = new ThreadClient(this, aResponse.threadActor);
-        this.webConsoleActor = aResponse.webConsoleActor;
+        this.consoleActor = aResponse.webConsoleActor;
         this.client.registerClient(this.thread);
       }
       aOnResponse(aResponse, this.thread);
     });
   },
 
   _onClose: function () {
     this.removeListener("close", this._onClose);
