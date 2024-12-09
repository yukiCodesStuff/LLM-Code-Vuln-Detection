
                  unsigned int size = GetUntrustedInt();
                     /* ignore integer overflow (CWE-190) for this example */
                     
                     unsigned int totBytes = size * sizeof(char);char *string = (char *)malloc(totBytes);InitializeString(string);
               
               