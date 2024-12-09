
                  int VerifyAdmin(char *password) {if (strcmp(compress(password), compressed_password)) {printf("Incorrect Password!\n");return(0);}printf("Entering Diagnostic Mode...\n");return(1);}
               
               