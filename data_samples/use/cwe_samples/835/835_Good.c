
                  int processMessagesFromServer(char *hostaddr, int port) {
                        ...
                           // initialize number of attempts counter
                           int count = 0;do {
                              
                                 
                                 // establish connection to server
                                 connected = connect(servsock, (struct sockaddr *)&servaddr, sizeof(servaddr));
                                 
                                 // increment counter
                                 count++;
                                 
                                 // if connected then read and process messages from server
                                 if (connected > -1) {
                                    
                                       
                                       // read and process messages
                                       ...
                                 }
                              
                           
                           
                           // keep trying to establish connection to the server
                           
                           
                           // up to a maximum number of attempts
                           } while (connected < 0 && count < MAX_ATTEMPTS);
                           
                           // close socket and return success or failure
                           ...
                     }
               
            