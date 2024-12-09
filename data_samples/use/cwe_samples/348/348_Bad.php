
                  $requestingIP = '0.0.0.0';if (array_key_exists('HTTP_X_FORWARDED_FOR', $_SERVER)) {$requestingIP = $_SERVER['HTTP_X_FORWARDED_FOR'];
                     else{$requestingIP = $_SERVER['REMOTE_ADDR'];}
                     if(in_array($requestingIP,$ipAllowlist)){generatePage();return;}else{echo "You are not authorized to view this page";return;}
               
               