
                  ...messageBody = (char*)malloc(length*sizeof(char));messageBody = &message[1][0];
                     int success = processMessageBody(messageBody);
                     if (success == ERROR) {result = ERROR;logError("Error processing message", messageBody);free(messageBody);}...
               
            