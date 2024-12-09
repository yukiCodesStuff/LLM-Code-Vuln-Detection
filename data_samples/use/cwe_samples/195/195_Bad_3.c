
                  char* processNext(char* strm) {char buf[512];short len = *(short*) strm;strm += sizeof(len);if (len <= 512) {memcpy(buf, strm, len);process(buf);return strm + len;}else {return -1;}}
               
               