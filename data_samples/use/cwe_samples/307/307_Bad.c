
                  int validateUser(char *host, int port){
                        int socket = openSocketConnection(host, port);if (socket < 0) {printf("Unable to open socket connection");return(FAIL);}
                           int isValidUser = 0;char username[USERNAME_SIZE];char password[PASSWORD_SIZE];
                           while (isValidUser == 0) {
                              if (getNextMessage(socket, username, USERNAME_SIZE) > 0) {if (getNextMessage(socket, password, PASSWORD_SIZE) > 0) {isValidUser = AuthenticateUser(username, password);}}
                           }return(SUCCESS);
                     }
               
               