
                  int outputStringToFile(char *output, char *filename) {
                        int isOutput = SUCCESS;
                           int isOpen = openFileToWrite(filename);if (isOpen == FAIL) {printf("Unable to open file %s", filename);isOutput = FAIL;}else {
                              int isWrite = writeToFile(output);if (isWrite == FAIL) {printf("Unable to write to file %s", filename);isOutput = FAIL;}
                                 int isClose = closeFile(filename);if (isClose == FAIL)isOutput = FAIL;
                              
                           }return isOutput;
                     }
               
            