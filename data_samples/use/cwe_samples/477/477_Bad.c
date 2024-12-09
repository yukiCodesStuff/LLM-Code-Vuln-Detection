
                  ...getpw(uid, pwdline);for (i=0; i<3; i++){cryptpw=strtok(pwdline, ":");pwdline=0;}result = strcmp(crypt(plainpw,cryptpw), cryptpw) == 0;...
               
               