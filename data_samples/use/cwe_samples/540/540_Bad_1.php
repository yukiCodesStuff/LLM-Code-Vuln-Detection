
               <?phpinclude('database.inc');$db = connectToDB($dbName, $dbPassword);$db.authenticateUser($username, $password);?>
             
             