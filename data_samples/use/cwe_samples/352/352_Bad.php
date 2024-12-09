
                  // initiate the session in order to validate sessions
                     session_start();
                     
                     //if the session is registered to a valid user then allow update
                     
                     if (! session_is_registered("username")) {
                        
                           echo "invalid session detected!";
                           
                           // Redirect user to login page
                           [...]
                           exit;
                     }
                     
                     // The user session is valid, so process the request
                     
                     
                     // and update the information
                     
                     update_profile();
                     function update_profile {
                        
                           
                           // read in the data from $POST and send an update
                           
                           
                           // to the database
                           SendUpdateToDatabase($_SESSION['username'], $_POST['email']);[...]echo "Your profile has been successfully updated.";
                     }
               
               