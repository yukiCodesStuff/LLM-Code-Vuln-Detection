
                  #define MAX_PASSWORD_LENGTH 15#define MAX_USERNAME_LENGTH 15
                     class UserAccount{
                        public:
                              UserAccount(char *username, char *password){if ((strlen(username) > MAX_USERNAME_LENGTH) ||(strlen(password) > MAX_PASSWORD_LENGTH)) {ExitError("Invalid username or password");}strcpy(this->username, username);strcpy(this->password, password);}
                           
                           
                           int authorizeAccess(char *username, char *password){
                              if ((strlen(username) > MAX_USERNAME_LENGTH) ||(strlen(password) > MAX_PASSWORD_LENGTH)) {ExitError("Invalid username or password");}
                                 // if the username and password in the input parameters are equal to
                                 
                                 
                                 // the username and password of this account class then authorize access
                                 if (strcmp(this->username, username) ||strcmp(this->password, password))return 0;
                                 
                                 // otherwise do not authorize access
                                 elsereturn 1;
                              
                           }
                           char username[MAX_USERNAME_LENGTH+1];char password[MAX_PASSWORD_LENGTH+1];
                     };
               
               