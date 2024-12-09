
                  
                     
                     //hardcode input length for simplicity
                     char* input = (char*) malloc(40*sizeof(char));char *tok;char* sep = " \t";
                     get_user_input( input );
                     
                     /* The following loop will parse and process each token in the input string */
                     
                     tok = strtok( input, sep);while( NULL != tok ){
                        if( isMalformed( tok ) ){
                              
                                 
                                 /* ignore and discard bad data */
                                 free( tok );
                           }else{add_to_command_queue( tok );}tok = strtok( NULL, sep));
                     }
               
               