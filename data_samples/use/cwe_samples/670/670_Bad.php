
               $requestingIP = $_SERVER['REMOTE_ADDR'];if(!in_array($requestingIP,$ipAllowList)){echo "You are not authorized to view this page";http_redirect($errorPageURL);}$status = getServerStatus();echo $status;
               ...
               
               
             
             