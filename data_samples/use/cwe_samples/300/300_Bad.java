
                  Socket sock;PrintWriter out;
                     try {
                        sock = new Socket(REMOTE_HOST, REMOTE_PORT);out = new PrintWriter(echoSocket.getOutputStream(), true);
                           
                           // Write data to remote host via socket output stream.
                           ...
                     }
               
               