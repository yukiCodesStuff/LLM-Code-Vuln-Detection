
                  $inputString = readLineFromFileHandle($serverFH);
                     
                     # generate an array of strings separated by the "|" character.
                     @commands = split(/\|/, $inputString);
                     foreach $cmd (@commands) {
                        
                        # separate the operator from its arguments based on a single whitespace
                        ($operator, $args) = split(/ /, $cmd, 2);
                        $args = UrlDecode($args);if ($operator eq "BAN") {ExecuteBan($args);}elsif ($operator eq "SAY") {ExecuteSay($args);}}
               
               