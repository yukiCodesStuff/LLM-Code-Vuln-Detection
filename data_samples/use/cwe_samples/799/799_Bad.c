
                  char username[USERNAME_SIZE];char password[PASSWORD_SIZE];
                     while (isValidUser == 0) {
                        if (getNextMessage(socket, username, USERNAME_SIZE) > 0) {if (getNextMessage(socket, password, PASSWORD_SIZE) > 0) {isValidUser = AuthenticateUser(username, password);}}
                     }return(SUCCESS);
               
               