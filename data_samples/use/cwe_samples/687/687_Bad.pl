
                  sub ReportAuth {my ($username, $result, $fatal) = @_;PrintLog("auth: username=%s, result=%d", $username, $result);if (($result ne "success") && $fatal) {die "Failed!\n";}}
                     sub PrivilegedFunc{my $result = CheckAuth($username);ReportAuth($username, $result, 0);DoReallyImportantStuff();}
               
            