
                  
                     
                     /* capture the sizes of all messages */
                     int getsizes(int sock, int count, int *sizes) {
                        ...char buf[BUFFER_SIZE];int ok;int num, size;
                           
                           // read values from socket and added to sizes array
                           while ((ok = gen_recv(sock, buf, sizeof(buf))) == 0){
                              
                                 // continue read from socket until buf only contains '.'
                                 if (DOTLINE(buf))break;
                                 else if (sscanf(buf, "%d %d", &num, &size) == 2)sizes[num - 1] = size;
                              
                           }...
                        
                     }
               
               