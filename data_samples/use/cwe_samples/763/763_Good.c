
                  #define SUCCESS (1)#define FAILURE (0)
                     int cointains_char(char c){
                        char *str;int i = 0;str = (char*)malloc(20*sizeof(char));strcpy(str, "Search Me!");while( i < strlen(str) ){
                              if( str[i] == c ){
                                    
                                       
                                       /* matched char, free string and return success */
                                       free(str);return SUCCESS;
                                 }
                                 /* didn't match yet, increment pointer and try next char */
                                 
                                 i = i + 1;
                           }
                           /* we did not match the char in the string, free mem and return failure */
                           
                           free(str);return FAILURE;
                     }
               
            