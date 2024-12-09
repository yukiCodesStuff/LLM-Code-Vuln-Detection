
                  $uname = GetUserInput("username");
                     
                     
                     # avoid CWE-22, CWE-78, others.
                     if ($uname !~ /^\w+$/){ExitError("Bad hacker!") ;}
                     $filename = "/home/myprog/config/" . $uname . ".txt";if (!(-e $filename)){ExitError("Error: $filename does not exist");}
               
               