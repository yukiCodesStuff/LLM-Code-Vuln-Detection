
                  #define FAIL 0#define SUCCESS 1
                     ...
                     int validateUser(char *username, char *password) {
                        
                           int isUser = FAIL;
                           
                           // call method to authenticate username and password
                           
                           
                           // if authentication fails then return failure otherwise return success
                           if (isUser = AuthenticateUser(username, password) == FAIL) {return isUser;}else {isUser = SUCCESS;}
                           return isUser;
                     }
               
               