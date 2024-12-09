
                  
                     /* copy filename parameter to variable, no off-by-one overflow */
                     strncpy(Filename, argv[2], sizeof(Filename)-1);Filename[255]='\0';
                     
                     /* copy pattern parameter to variable, no off-by-one overflow */
                     strncpy(Pattern, argv[3], sizeof(Pattern)-1);Pattern[31]='\0';
               
            