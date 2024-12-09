
                  FILE *stream;if( (stream = tmpfile()) == NULL ) {
                        
                           perror("Could not open new temporary file\n");return (-1);
                     }
                     // write data to tmp file
                     ...// remove tmp filermtmp();
               
               