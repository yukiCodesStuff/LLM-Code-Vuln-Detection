
                  
                     //assume the password is already encrypted, avoiding CWE-312
                     
                     function authenticate($username,$password){
                        include("http://external.example.com/dbInfo.php");
                        
                        //dbInfo.php makes $dbhost, $dbuser, $dbpass, $dbname available
                        mysql_connect($dbhost, $dbuser, $dbpass) or die ('Error connecting to mysql');mysql_select_db($dbname);$query = 'Select * from users where username='.$username.' And password='.$password;$result = mysql_query($query);
                        if(mysql_numrows($result) == 1){mysql_close();return true;}else{mysql_close();return false;}
                     }
               
               