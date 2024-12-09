
                  $username = mysql_real_escape_string($username);$fullName = mysql_real_escape_string($fullName);$query = sprintf('Insert Into users (username,password) Values ("%s","%s","%s")', $username, crypt($password),$fullName) ;mysql_query($query);/.../
               
               