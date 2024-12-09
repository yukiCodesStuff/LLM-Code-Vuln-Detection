
                  function printFile($username,$filename){
                        
                           
                           //read file into string
                           $file = file_get_contents($filename);if ($file && isOwnerOf($username,$filename)){echo $file;return true;}else{echo 'You are not authorized to view this file';}return false;
                     }
               
               