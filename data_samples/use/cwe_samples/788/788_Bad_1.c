
                  int returnChunkSize(void *) {
                        
                           
                           /* if chunk info is valid, return the size of usable memory,
                           
                           
                           * else, return -1 to indicate an error
                           
                           
                           */
                           ...
                     }int main() {...memcpy(destBuf, srcBuf, (returnChunkSize(destBuf)-1));...}
               
               