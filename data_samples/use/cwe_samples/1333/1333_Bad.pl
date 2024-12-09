
                        my $test_string = "Bad characters: \$\@\#";
                        my $bdrslt = $test_string;
                        $bdrslt =~ /^(\w+\s?)*$/i;
                    
                    