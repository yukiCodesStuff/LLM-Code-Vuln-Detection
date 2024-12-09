
                  #define PATH_SIZE 60
                     char filename[PATH_SIZE];
                     for(i=0; i<=PATH_SIZE; i++) {
                     
                        char c = getc();if (c == 'EOF') {filename[i] = '\0';}
                           filename[i] = getc();
                     }
               
               