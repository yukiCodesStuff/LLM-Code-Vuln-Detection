
                  public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
                        
                           
                           // Perform servlet tasks.
                           ...
                           
                           // Open a socket to a remote server (bad).
                           Socket sock = null;
                           try {
                              sock = new Socket(remoteHostname, 3000);
                                 
                                 // Do something with the socket.
                                 ...
                           } catch (Exception e) {...}
                     }
               
               