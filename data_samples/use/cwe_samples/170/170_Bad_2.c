
                  #include <stdio.h>#include <string.h>
                     int main() {
                        
                           char longString[] = "String signifying nothing";char shortString[16];
                           strncpy(shortString, longString, 16);printf("The last character in shortString is: %c (%1$x)\n", shortString[15]);return (0);
                     }
               
               