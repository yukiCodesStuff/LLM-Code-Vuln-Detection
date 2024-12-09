
                  struct stat *sb;...lstat("...",sb); // it has not been updated since the last time it was readprintf("stated file\n");if (sb->st_mtimespec==...){print("Now updating things\n");updateThings();}
               
               