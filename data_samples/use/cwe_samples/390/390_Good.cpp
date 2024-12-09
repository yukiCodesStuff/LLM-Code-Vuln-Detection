
                  char* readFile (char *filename) {
                        try {
                              // open input fileifstream infile;infile.open(filename);
                                 if (!infile.is_open()) {throw "Unable to open file " + filename;}
                                 // get length of fileinfile.seekg (0, ios::end);int length = infile.tellg();infile.seekg (0, ios::beg);
                                 // allocate memorychar *buffer = new char [length];
                                 // read data from fileinfile.read (buffer,length);
                                 if (!infile.good()) {throw "Unable to read from file " + filename;}infile.close();
                                 return buffer;
                           }catch (char *str) {printf("Error: %s \n", str);infile.close();throw str;}catch (...) {printf("Error occurred trying to read from file \n");infile.close();throw;}
                     }
               
            