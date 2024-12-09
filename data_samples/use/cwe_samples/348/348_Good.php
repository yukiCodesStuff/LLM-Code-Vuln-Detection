
                  $requestingIP = '0.0.0.0';if (array_key_exists('HTTP_X_FORWARDED_FOR', $_SERVER)) {echo "This application cannot be accessed through a proxy.";return;
                     else{$requestingIP = $_SERVER['REMOTE_ADDR'];}
                     ...
                     
                  
               
               