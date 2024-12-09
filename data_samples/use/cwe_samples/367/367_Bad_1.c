
                  if(!access(file,W_OK)) {f = fopen(file,"w+");operate(f);...}else {
                        
                           fprintf(stderr,"Unable to open file %s.\n",file);
                     }
               
               