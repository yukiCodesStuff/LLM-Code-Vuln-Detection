
                  $ENV{"HOMEDIR"} = "/home/mydir/public/";my $stream = AcceptUntrustedInputStream();while (<$stream>) {
                        chomp;if (/^ENV ([\w\_]+) (.*)/) {$ENV{$1} = $2;}elsif (/^QUIT/) { ... }elsif (/^LIST/) {open($fh, "/bin/ls -l $ENV{HOMEDIR}|");while (<$fh>) {SendOutput($stream, "FILEINFO: $_");}close($fh);}
                     }
               
               