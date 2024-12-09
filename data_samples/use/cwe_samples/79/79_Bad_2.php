
                  $query = 'Select * From users Where loggedIn=true';$results = mysql_query($query);
                     if (!$results) {exit;}
                     
                     //Print list of users to page
                     echo '<div id="userlist">Currently Active Users:';while ($row = mysql_fetch_assoc($results)) {echo '<div class="userNames">'.$row['fullname'].'</div>';}echo '</div>';
               
               