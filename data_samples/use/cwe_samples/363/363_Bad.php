
                  function readFile($filename){
                        $user = getCurrentUser();
                           
                           //resolve file if its a symbolic link
                           if(is_link($filename)){$filename = readlink($filename);}
                           if(fileowner($filename) == $user){echo file_get_contents($realFile);return;}else{echo 'Access denied';return false;}
                     }
               
               