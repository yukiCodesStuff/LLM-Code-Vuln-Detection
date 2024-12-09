
                  
                     
                     //hardcode input length for simplicity
                     char* input = (char*) malloc(40*sizeof(char));char *tok, *command;char* sep = " \t";
                     get_user_input( input );
                     
                     /* The following loop will parse and process each token in the input string */
                     
                     tok = strtok( input, sep);while( NULL != tok ){
                        if( !isMalformed( command ) ){
                              
                                 
                                 /* copy and enqueue good data */
                                 command = (char*) malloc( (strlen(tok) + 1) * sizeof(char) );strcpy( command, tok );add_to_command_queue( command );
                           }tok = strtok( NULL, sep));
                     }
                     free( input )
               
            