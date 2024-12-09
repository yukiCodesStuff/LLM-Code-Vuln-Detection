
                  #define DIR "/restricted/directory"
                     char cmd[500];sprintf(cmd, "ls -l %480s", DIR);
                     /* Raise privileges to those needed for accessing DIR. */
                     
                     RaisePrivileges(...);system(cmd);DropPrivileges(...);...
               
               