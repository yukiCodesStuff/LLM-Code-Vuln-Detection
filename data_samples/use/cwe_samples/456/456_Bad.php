
                  if (authenticate($username,$password) && setAdmin($username)){$isAdmin = true;}
                     /.../
                     
                     if ($isAdmin){deleteUser($userToDelete);}
               
               