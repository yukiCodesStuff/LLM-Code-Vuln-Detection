
                  int processMessagesFromServer(char *hostaddr, int port) {
                        ...int servsock;int connected;struct sockaddr_in servaddr;
                           
                           // create socket to connect to server
                           servsock = socket( AF_INET, SOCK_STREAM, 0);memset( &servaddr, 0, sizeof(servaddr));servaddr.sin_family = AF_INET;servaddr.sin_port = htons(port);servaddr.sin_addr.s_addr = inet_addr(hostaddr);
                           do {
                              
                                 
                                 // establish connection to server
                                 connected = connect(servsock, (struct sockaddr *)&servaddr, sizeof(servaddr));
                                 
                                 // if connected then read and process messages from server
                                 if (connected > -1) {
                                    
                                       
                                       // read and process messages
                                       ...
                                 }
                              
                           
                           
                           // keep trying to establish connection to the server
                           } while (connected < 0);
                           
                           // close socket and return success or failure
                           ...
                     }
               
               