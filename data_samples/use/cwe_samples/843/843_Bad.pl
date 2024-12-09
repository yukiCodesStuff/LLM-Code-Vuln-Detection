
                  my $UserPrivilegeArray = ["user", "user", "admin", "user"];
                     my $userID = get_current_user_ID();
                     if ($UserPrivilegeArray eq "user") {print "Regular user!\n";}else {print "Admin!\n";}
                     print "\$UserPrivilegeArray = $UserPrivilegeArray\n";
               
               