
                  int processMessageFromSocket(int socket) {
                        int success;
                           char buffer[BUFFER_SIZE];char message[MESSAGE_SIZE];
                           
                           // get message from socket and store into buffer
                           
                           
                           //Ignoring possibliity that buffer > BUFFER_SIZE
                           if (getMessage(socket, buffer, BUFFER_SIZE) > 0) {
                              
                                 
                                 // place contents of the buffer into message structure
                                 ExMessage *msg = recastBuffer(buffer);
                                 
                                 // copy message body into string for processing
                                 int index;for (index = 0; index < msg->msgLength; index++) {message[index] = msg->msgBody[index];}message[index] = '\0';
                                 
                                 // process message
                                 success = processMessage(message);
                           }return success;
                     }
               
               