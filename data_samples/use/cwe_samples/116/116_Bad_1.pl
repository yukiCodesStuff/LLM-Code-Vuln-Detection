
                  $inputString = GetUntrustedArgument("command");($cmd, $argstr) = split(/\s+/, $inputString, 2);
                     
                     # removes extra whitespace and also changes CRLF's to spaces
                     $argstr =~ s/\s+/ /gs;
                     $argstr = UrlEncode($argstr);if (($cmd eq "BAN") && (! IsAdministrator($username))) {die "Error: you are not the admin.\n";}
                     
                     # communicate with file server using a file handle
                     $fh = GetServerFileHandle("myserver");
                     print $fh "$cmd $argstr\n";
               
               