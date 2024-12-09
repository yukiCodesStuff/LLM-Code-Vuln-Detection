
                  int GetUntrustedInt () {return(0x0000FFFF);}
                     void main (int argc, char **argv) {
                        char path[256];char *input;int i;short s;unsigned int sz;
                           i = GetUntrustedInt();s = i;/* s is -1 so it passes the safety check - CWE-697 */if (s > 256) {DiePainfully("go away!\n");}
                           /* s is sign-extended and saved in sz */sz = s;
                           /* output: i=65535, s=-1, sz=4294967295 - your mileage may vary */printf("i=%d, s=%d, sz=%u\n", i, s, sz);
                           input = GetUserInput("Enter pathname:");
                           /* strncpy interprets s as unsigned int, so it's treated as MAX_INT(CWE-195), enabling buffer overflow (CWE-119) */strncpy(path, input, s);path[255] = '\0'; /* don't want CWE-170 */printf("Path is: %s\n", path);
                     }
               
               