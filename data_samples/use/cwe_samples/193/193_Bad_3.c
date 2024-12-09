
                  int setFilename(char *filename) {char name[20];sprintf(name, "%16s.dat", filename);int success = saveFormattedFilenameToDB(name);return success;}
               
               