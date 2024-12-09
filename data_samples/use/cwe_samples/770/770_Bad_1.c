
                  int writeDataFromSocketToFile(char *host, int port){
                        
                           char filename[FILENAME_SIZE];char buffer[BUFFER_SIZE];int socket = openSocketConnection(host, port);
                           if (socket < 0) {printf("Unable to open socket connection");return(FAIL);}if (getNextMessage(socket, filename, FILENAME_SIZE) > 0) {
                              if (openFileToWrite(filename) > 0) {
                                    while (getNextMessage(socket, buffer, BUFFER_SIZE) > 0){if (!(writeToFile(buffer) > 0))break;
                                       }
                                 }closeFile();
                           }closeSocket(socket);
                     }
               
               