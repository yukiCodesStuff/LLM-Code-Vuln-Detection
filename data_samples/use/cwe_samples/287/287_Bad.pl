
                  my $q = new CGI;
                     if ($q->cookie('loggedin') ne "true") {
                        if (! AuthenticateUser($q->param('username'), $q->param('password'))) {ExitError("Error: you need to log in first");}else {
                              # Set loggedin and user cookies.$q->cookie(-name => 'loggedin',-value => 'true');
                                 $q->cookie(-name => 'user',-value => $q->param('username'));
                              
                           }
                     }
                     if ($q->cookie('user') eq "Administrator") {DoAdministratorTasks();}
               
               