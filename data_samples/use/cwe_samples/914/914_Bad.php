
                  
                     //Log user in, and set $isAdmin to true if user is an administrator
                     
                     function login($user,$pass){$query = buildQuery($user,$pass);mysql_query($query);if(getUserRole($user) == "Admin"){$isAdmin = true;}}
                     $isAdmin = false;extract($_POST);login(mysql_real_escape_string($user),mysql_real_escape_string($pass));
               
               