
                  my $dataPath = "/users/cwe/profiles";my $username = param("user");my $profilePath = $dataPath . "/" . $username;
                     open(my $fh, "<", $profilePath) || ExitError("profile read error: $profilePath");print "<ul>\n";while (<$fh>) {print "<li>$_</li>\n";}print "</ul>\n";
               
               