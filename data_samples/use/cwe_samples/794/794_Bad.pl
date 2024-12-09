
                  my $Username = GetUntrustedInput();$Username =~ s/\.\.\///;my $filename = "/home/user/" . $Username;ReadAndSendFile($filename);
               
               