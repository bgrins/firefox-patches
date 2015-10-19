# HG changeset patch
# User Brian Grinstead <bgrinstead@mozilla.com>
# Parent  e6297047093b0b1df9f12dc0ce4ecce3d542bbb2
x

diff --git a/devtools/server/main.js b/devtools/server/main.js
--- a/devtools/server/main.js
+++ b/devtools/server/main.js
@@ -785,16 +785,17 @@ var DebuggerServer = {
         // for each connection.
         let listener = {
           onClose: () => {
             aDbg.removeListener(listener);
           },
 
           onMessage: (message) => {
             let packet = JSON.parse(message);
+
             if (packet.type !== "rpc") {
               return;
             }
 
             Promise.resolve().then(() => {
               let method = {
                 "fetch": DevToolsUtils.fetch,
               }[packet.method];
@@ -827,83 +828,129 @@ var DebuggerServer = {
       // Step 2: Send a connect request to the worker debugger.
       aDbg.postMessage(JSON.stringify({
         type: "connect",
         id: aId,
         options: aOptions
       }));
 
       // Steps 3-5 are performed on the worker thread (see worker.js).
-      let threadActor;
-      let webConsoleActor;
+
 
       // Step 6: Wait for a response from the worker debugger.
       let listener = {
         onClose: () => {
           aDbg.removeListener(listener);
 
           reject("closed");
         },
 
         onMessage: (message) => {
           let packet = JSON.parse(message);
+
+          // Step 7: Assign the actors that we created in the worker thread.
+          if (packet.type === "connected") {
+            console.log(packet);
+            aDbg.removeListener(listener);
+
+            let threadActor = packet.threadActor;
+            let webConsoleActor = packet.webConsoleActor;
+
+            // Step 8: Create a transport for the connection to the worker.
+            let transport = new WorkerDebuggerTransport(aDbg, aId);
+            transport.ready();
+            transport.hooks = {
+              onClosed: () => {
+                if (!aDbg.isClosed) {
+                  aDbg.postMessage(JSON.stringify({
+                    type: "disconnect",
+                    id: aId
+                  }));
+                }
+
+                aConnection.cancelForwarding(aId);
+              },
+
+              onPacket: (packet) => {
+                // Ensure that any packets received from the server on the worker
+                // thread are forwarded to the client on the main thread, as if
+                // they had been sent by the server on the main thread.
+                aConnection.send(packet);
+              }
+            };
+
+            // Ensure that any packets received from the client on the main thread
+            // to actors on the worker thread are forwarded to the server on the
+            // worker thread.
+            aConnection.setForwarding(aId, transport);
+
+            aConnection.send({
+              to: threadActor,
+              from: "me",
+              type: "attach"
+            });
+            resolve({
+              threadActor: threadActor,
+              webConsoleActor: webConsoleActor,
+              transport: transport
+            });
+          }
+
+          // Step 8: The thread has been attached
+
           if (packet.type !== "message" || packet.id !== aId) {
             return;
           }
 
+          // console.log("Step 7: Assign the actors that we created in the worker thread " + message + "\n");
           message = packet.message;
           if (message.error) {
             reject(error);
           }
 
-          // Step 7: Assign the actors that we created in the worker thread.
-          if (message.type === "actors-created") {
-            threadActor = message.threadActor;
-            webConsoleActor = message.webConsoleActor;
-          }
 
-          if (message.type !== "paused") {
-            return;
-          }
+          // if (message.type !== "paused") {
+          //   return;
+          // }
 
-          aDbg.removeListener(listener);
+          // aDbg.removeListener(listener);
 
-          // Step 8: Create a transport for the connection to the worker.
-          let transport = new WorkerDebuggerTransport(aDbg, aId);
-          transport.ready();
-          transport.hooks = {
-            onClosed: () => {
-              if (!aDbg.isClosed) {
-                aDbg.postMessage(JSON.stringify({
-                  type: "disconnect",
-                  id: aId
-                }));
-              }
+          // // Step 8: Create a transport for the connection to the worker.
+          // let transport = new WorkerDebuggerTransport(aDbg, aId);
+          // transport.ready();
+          // transport.hooks = {
+          //   onClosed: () => {
+          //     if (!aDbg.isClosed) {
+          //       aDbg.postMessage(JSON.stringify({
+          //         type: "disconnect",
+          //         id: aId
+          //       }));
+          //     }
 
-              aConnection.cancelForwarding(aId);
-            },
+          //     aConnection.cancelForwarding(aId);
+          //   },
 
-            onPacket: (packet) => {
-              // Ensure that any packets received from the server on the worker
-              // thread are forwarded to the client on the main thread, as if
-              // they had been sent by the server on the main thread.
-              aConnection.send(packet);
-            }
-          };
+          //   onPacket: (packet) => {
+          //     // Ensure that any packets received from the server on the worker
+          //     // thread are forwarded to the client on the main thread, as if
+          //     // they had been sent by the server on the main thread.
+          //     aConnection.send(packet);
+          //   }
+          // };
 
-          // Ensure that any packets received from the client on the main thread
-          // to actors on the worker thread are forwarded to the server on the
-          // worker thread.
-          aConnection.setForwarding(aId, transport);
+          // // Ensure that any packets received from the client on the main thread
+          // // to actors on the worker thread are forwarded to the server on the
+          // // worker thread.
+          // aConnection.setForwarding(aId, transport);
 
-          resolve({
-            threadActor: threadActor,
-            webConsoleActor: webConsoleActor,
-            transport: transport
-          });
+          // resolve({
+          //   threadActor: threadActor,
+          //   webConsoleActor: webConsoleActor,
+          //   transport: transport
+          // });
         }
       };
       aDbg.addListener(listener);
     });
   },
 
   /**
    * Check if the caller is running in a content child process.
diff --git a/devtools/server/worker.js b/devtools/server/worker.js
--- a/devtools/server/worker.js
+++ b/devtools/server/worker.js
@@ -79,34 +79,37 @@ this.addEventListener("message",  functi
     let threadActor = new ThreadActor(parent, global);
     pool.addActor(threadActor);
 
     let webConsoleActor = new WebConsoleActor(connection, parent);
     pool.addActor(webConsoleActor);
 
     // XXX: USE POST MESSAGE FOR THIS
 
-    // postMessage({
-    //   type: "connected",
 
 
-    
-    connection.send({
-      type: "actors-created",
-      threadActor: threadActor.actorID,
-      webConsoleActor: webConsoleActor.actorID,
-    });
+    // connection.send({
+    //   type: "actors-created",
+    //   threadActor: threadActor.actorID,
+    //   webConsoleActor: webConsoleActor.actorID,
+    // });
 
     // Step 5: Attach to the thread actor.
     //
     // This will cause a packet to be sent over the connection to the parent.
     // Because this connection uses WorkerDebuggerTransport internally, this
     // packet will be sent using WorkerDebuggerGlobalScope.postMessage, causing
     // an onMessage event to be fired on the WorkerDebugger in the main thread.
-    threadActor.onAttach({});
+    // threadActor.onAttach({});
+    postMessage(JSON.stringify({
+      type: "connected",
+      threadActor: threadActor.actorID,
+      webConsoleActor: webConsoleActor.actorID,
+    }));
+
     break;
 
   case "disconnect":
     connections[packet.id].connection.close();
     break;
 
   case "rpc":
     let deferred = rpcDeferreds[packet.id];
