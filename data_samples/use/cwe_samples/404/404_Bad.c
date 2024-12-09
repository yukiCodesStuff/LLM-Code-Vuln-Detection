
                  int decodeFile(char* fName) {
                        char buf[BUF_SZ];FILE* f = fopen(fName, "r");if (!f) {printf("cannot open %s\n", fName);return DECODE_FAIL;}else {
                              while (fgets(buf, BUF_SZ, f)) {if (!checkChecksum(buf)) {return DECODE_FAIL;}else {decodeBlock(buf);}}
                           }fclose(f);return DECODE_SUCCESS;
                     }
               
            