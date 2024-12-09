
                  #define LOG_INPUT_SIZE 40
                     
                     // saves the file name to a log file
                     int outputFilenameToLog(char *filename, int length) {
                        int success;
                           
                           // buffer with size set to maximum size for input to log file
                           char buf[LOG_INPUT_SIZE];
                           
                           // copy filename to buffer
                           strncpy(buf, filename, length);
                           
                           // save to log file
                           success = saveToLogFile(buf);
                           return success;
                     }
               
               