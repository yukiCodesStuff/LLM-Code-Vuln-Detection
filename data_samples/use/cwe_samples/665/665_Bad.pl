
                  $username = GetCurrentUser();$state = GetStateData($username);if (defined($state)) {$uid = ExtractUserID($state);}
                     
                     # do stuff
                     if ($uid == 0) {DoAdminThings();}
               
               