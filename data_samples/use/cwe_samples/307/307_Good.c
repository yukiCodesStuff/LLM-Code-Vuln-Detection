
                  int validateUser(char *host, int port){
                        ...
                           int count = 0;while ((isValidUser == 0) && (count < MAX_ATTEMPTS)) {
                              if (getNextMessage(socket, username, USERNAME_SIZE) > 0) {if (getNextMessage(socket, password, PASSWORD_SIZE) > 0) {isValidUser = AuthenticateUser(username, password);}}count++;
                           }if (isValidUser) {return(SUCCESS);}else {return(FAIL);}
                     }
               
            