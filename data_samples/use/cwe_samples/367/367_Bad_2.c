
                  
		    #include <sys/types.h>
		    #include <sys/stat.h>
		    
		    ...
		    
		      struct stat sb;
		      stat("MYFILE.txt",&sb);
		      printf("file change time: %d\n",sb->st_ctime);
		      switch(sb->st_ctime % 2){
		        case 0: printf("Option 1\n"); break;
		        case 1: printf("Option 2\n"); break;
		        default: printf("this should be unreachable?\n"); break;
		      }
                  
               
               