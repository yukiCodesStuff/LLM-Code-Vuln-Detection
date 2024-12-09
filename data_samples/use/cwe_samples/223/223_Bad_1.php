
                  function readFile($filename){
                        $user = getCurrentUser();$realFile = $filename;
                           
                           //resolve file if its a symbolic link
                           if(is_link($filename)){$realFile = readlink($filename);}
                           if(fileowner($realFile) == $user){echo file_get_contents($realFile);return;}else{echo 'Access denied';writeLog($user . ' attempted to access the file '. $filename . ' on '. date('r'));}
                     }
               
               