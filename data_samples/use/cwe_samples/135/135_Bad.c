
                  #include <stdio.h>#include <strings.h>#include <wchar.h>
                     int main() {
                        
                           wchar_t wideString[] = L"The spazzy orange tiger jumped " \"over the tawny jaguar.";wchar_t *newString;
                           printf("Strlen() output: %d\nWcslen() output: %d\n",strlen(wideString), wcslen(wideString));
                           /* Wrong because the number of chars in a string isn't related to its length in bytes //newString = (wchar_t *) malloc(strlen(wideString));*/
                           /* Wrong because wide characters aren't 1 byte long! //newString = (wchar_t *) malloc(wcslen(wideString));*/
                           /* Wrong because wcslen does not include the terminating null */newString = (wchar_t *) malloc(wcslen(wideString) * sizeof(wchar_t));
                           /* correct! */newString = (wchar_t *) malloc((wcslen(wideString) + 1) * sizeof(wchar_t));
                           /* ... */
                     }
               
               