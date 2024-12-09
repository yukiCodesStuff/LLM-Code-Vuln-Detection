
                  static void print (char * string) {
                        char * word;int counter;for (word = string; counter = *word++; ) {
                              putc(counter, stdout);fflush(stdout);
                                 /* Make timing window a little larger... */
                                 
                                 sleep(1);
                           }
                     }
                     int main(void) {
                        pid_t pid;
                           pid = fork();if (pid == -1) {exit(-2);}else if (pid == 0) {print("child\n");}else {print("PARENT\n");}exit(0);
                     }
               
               