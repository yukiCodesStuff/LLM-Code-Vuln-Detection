
                  char **ap, *argv[10], *inputstring;for (ap = argv; (*ap = strsep(&inputstring, " \t")) != NULL;)
                        if (**ap != '\0')if (++ap >= &argv[10])break;
                           
                        
                     
                     /.../free(ap[4]);
               
               