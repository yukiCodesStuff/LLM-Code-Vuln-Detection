
                        my $test_string = "Bad characters: \$\@\#";
                        my $gdrslt = $test_string;
                         $gdrslt =~ /^((?=(\w+))\2\s?)*$/i;
                    
                    