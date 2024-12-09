
                  #include <signal.h>#include <syslog.h>#include <string.h>#include <stdlib.h>
                     void *global1, *global2;char *what;void sh (int dummy) {
                        syslog(LOG_NOTICE,"%s\n",what);free(global2);free(global1);
                           /* Sleep statements added to expand timing window for race condition */
                           
                           sleep(10);exit(0);
                     }
                     int main (int argc,char* argv[]) {
                        what=argv[1];global1=strdup(argv[2]);global2=malloc(340);signal(SIGHUP,sh);signal(SIGTERM,sh);
                           /* Sleep statements added to expand timing window for race condition */
                           
                           sleep(10);exit(0);
                     }
               
               