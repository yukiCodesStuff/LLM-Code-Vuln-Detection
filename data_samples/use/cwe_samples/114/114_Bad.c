
                  ...RegQueryValueEx(hkey, "APPHOME",0, 0, (BYTE*)home, &size);char* lib=(char*)malloc(strlen(home)+strlen(INITLIB));if (lib) {
                        
                           strcpy(lib,home);strcat(lib,INITCMD);LoadLibrary(lib);
                     }...
               
               