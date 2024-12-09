
                  public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
                        
                           
                           // Perform servlet tasks.
                           ...
                           
                           // Create a new thread to handle background processing.
                           Runnable r = new Runnable() {
                              public void run() {
                                    
                                       
                                       // Process and store request statistics.
                                       ...
                                 }
                           };
                           new Thread(r).start();
                     }
               
            