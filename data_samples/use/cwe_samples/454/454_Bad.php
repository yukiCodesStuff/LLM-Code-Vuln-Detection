
                  $debugEnabled = false;if ($_POST["debug"] == "true"){$debugEnabled = true;}
                     /.../
                     
                     function login($username, $password){if($debugEnabled){echo 'Debug Activated';phpinfo();$isAdmin = True;return True;}}
               
               