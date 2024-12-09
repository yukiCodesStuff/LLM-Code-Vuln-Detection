
                  my $username=param('username');
                  my $password=param('password');
                  
                  if (IsValidUsername($username) == 1)
                  {
                  if (IsValidPassword($username, $password) == 1)
                  {
                  print "Login Successful";
                  }
                  else
                  {
                  print "Login Failed - incorrect password";
                  }
                  }
                  else
                  {
                  print "Login Failed - unknown username";
                  }
                  
               
               