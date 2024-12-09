
                  my $Username = GetUntrustedInput();if (substr($Username, 0, 3) eq '../') {$Username = substr($Username, 3);}my $filename = "/home/user/" . $Username;ReadAndSendFile($filename);
               
               