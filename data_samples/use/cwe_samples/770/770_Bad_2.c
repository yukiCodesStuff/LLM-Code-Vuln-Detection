
                  
                     
                     /* process message accepts a two-dimensional character array of the form [length][body] containing the message to be processed */
                     int processMessage(char **message){
                        char *body;
                           int length = getMessageLength(message[0]);
                           if (length > 0) {body = &message[1][0];processMessageBody(body);return(SUCCESS);}else {printf("Unable to process message; invalid message length");return(FAIL);}
                     }
               
               