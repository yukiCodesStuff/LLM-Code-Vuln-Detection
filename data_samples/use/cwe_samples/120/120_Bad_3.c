
                  ...
                        struct hostent *clienthp;char hostname[MAX_LEN];
                           // create server socket, bind to server address and listen on socket...
                           // accept client connections and process requestsint count = 0;for (count = 0; count < MAX_CONNECTIONS; count++) {
                              
                                 int clientlen = sizeof(struct sockaddr_in);int clientsocket = accept(serversocket, (struct sockaddr *)&clientaddr, &clientlen);
                                 if (clientsocket >= 0) {
                                    clienthp = gethostbyaddr((char*) &clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);strcpy(hostname, clienthp->h_name);logOutput("Accepted client connection from host ", hostname);
                                       // process client request...close(clientsocket);
                                 }
                           }close(serversocket);
                     
                     ...
               
               