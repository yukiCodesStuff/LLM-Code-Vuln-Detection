
                  int main() {
                     ...
                     char *result = strstr(destBuf, "Replace Me");
                     int idx = result - destBuf;
                     strcpy(&destBuf[idx], srcBuf);
                     ...}
                  
               
               