
                  char* getBlock(int fd) {
                        char* buf = (char*) malloc(BLOCK_SIZE);if (!buf) {return NULL;}if (read(fd, buf, BLOCK_SIZE) != BLOCK_SIZE) {
                              
                                 return NULL;
                           }return buf;
                     }
               
            