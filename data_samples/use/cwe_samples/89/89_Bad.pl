
                  $userKey = getUserID();$name = getUserInput();
                     
                     # ensure only letters, hyphens and apostrophe are allowed
                     $name = allowList($name, "^a-zA-z'-$");$query = "INSERT INTO last_names VALUES('$userKey', '$name')";
               
               