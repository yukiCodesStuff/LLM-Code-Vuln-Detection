
                  $username = $_POST['username'];$picSource = $_POST['picsource'];$picAltText = $_POST['picalttext'];
                     ...
                     
                     echo "<title>Welcome, " . htmlentities($username) ."</title>";echo "<img src='". htmlentities($picSource) ." ' alt='". htmlentities($picAltText) . '" />';
                     ...
                     
                  
               
               