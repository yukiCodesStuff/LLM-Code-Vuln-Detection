
                  $ConfigDir = "/home/myprog/config";$uname = GetUserInput("username");
                     
                     # avoid CWE-22, CWE-78, others.
                     ExitError("Bad hacker!") if ($uname !~ /^\w+$/);$file = "$ConfigDir/$uname.txt";if (! (-e $file)) {ExitError("Error: $file does not exist");}...
               
               