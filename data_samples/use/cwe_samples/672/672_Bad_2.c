
                  #define FAIL 0#define SUCCESS 1#define ERROR -1#define MAX_MESSAGE_SIZE 32
                     int processMessage(char **message){
                        int result = SUCCESS;
                           int length = getMessageLength(message[0]);char *messageBody;
                           if ((length > 0) && (length < MAX_MESSAGE_SIZE)) {
                           
                              messageBody = (char*)malloc(length*sizeof(char));messageBody = &message[1][0];
                                 int success = processMessageBody(messageBody);
                                 if (success == ERROR) {result = ERROR;free(messageBody);}
                           }else {printf("Unable to process message; invalid message length");result = FAIL;}
                           if (result == ERROR) {logError("Error processing message", messageBody);}
                           return result;
                     }
               
               