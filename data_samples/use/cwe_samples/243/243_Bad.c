
                  chroot("/var/ftproot");...fgets(filename, sizeof(filename), network);localfile = fopen(filename, "r");while ((len = fread(buf, 1, sizeof(buf), localfile)) != EOF) {fwrite(buf, 1, sizeof(buf), network);}fclose(localfile);
               
               