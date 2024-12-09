
                  $phone = GetPhoneNumber();if ($phone =~ /\d+-\d+/) {
                        # looks like it only has hyphens and digits
                        system("lookup-phone $phone");}
				  else {error("malformed number!");}
               
               