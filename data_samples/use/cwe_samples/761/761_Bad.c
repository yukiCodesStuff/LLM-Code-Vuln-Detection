
                  #define SUCCESS (1)#define FAILURE (0)
                     int contains_char(char c){
                        char *str;str = (char*)malloc(20*sizeof(char));strcpy(str, "Search Me!");while( *str != NULL){
                              if( *str == c ){
                                    
                                       
                                       /* matched char, free string and return success */
                                       free(str);return SUCCESS;
                                 }
                                 /* didn't match yet, increment pointer and try next char */
                                 
                                 str = str + 1;
                           }
                           /* we did not match the char in the string, free mem and return failure */
                           
                           free(str);return FAILURE;
                     }
               
               