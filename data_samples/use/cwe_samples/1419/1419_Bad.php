
                  
                    
                    // $user and $pass automatically set from POST request
                    if (login_user($user,$pass)) {$authorized = true;}
                    ...
                    
                  if ($authorized) {generatePage();}
				
				