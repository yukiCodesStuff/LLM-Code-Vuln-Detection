
                  char *logMessage;
                     void handler (int sigNum) {
                        syslog(LOG_NOTICE, "%s\n", logMessage);free(logMessage);
                           /* artificially increase the size of the timing window to make demonstration of this weakness easier. */
                           
                           sleep(10);exit(0);
                     }
                     int main (int argc, char* argv[]) {
                        logMessage = strdup(argv[1]);
                           /* Register signal handlers. */
                           
                           signal(SIGHUP, handler);signal(SIGTERM, handler);
                           /* artificially increase the size of the timing window to make demonstration of this weakness easier. */
                           
                           sleep(10);
                     }
               
               