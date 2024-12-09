
                  
                     
                     //assume getCurrentUser() returns a username that is guaranteed to be alphanumeric (avoiding CWE-78)
                     $userName = getCurrentUser();$command = 'ps aux | grep ' . $userName;system($command);
               
               