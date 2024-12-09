
                  char *createOutputDirectory(char *name) {
                        char outputDirectoryName[128];if (getCurrentDirectory(128, outputDirectoryName) == 0) {return null;}if (!PathAppend(outputDirectoryName, "output")) {return null;}if (!PathAppend(outputDirectoryName, name)) {
                              
                                 return null;
                           }if (SHCreateDirectoryEx(NULL, outputDirectoryName, NULL) != ERROR_SUCCESS) {
                              
                                 return null;
                           }return StrDup(outputDirectoryName);
                     }
               
               