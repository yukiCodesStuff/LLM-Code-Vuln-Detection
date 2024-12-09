
                  $server = "server.example.com";$username = AskForUserName();$password = AskForPassword();$address = AskForAddress();$sock = OpenSocket($server, 1234);writeSocket($sock, "AUTH $username $password\n");$resp = readSocket($sock);if ($resp eq "success") {
                        
                           
                           # username/pass is valid, go ahead and update the info!
                           writeSocket($sock, "CHANGE-ADDRESS $username $address\n";
                     }else {print "ERROR: Invalid Authentication!\n";}
               
               