
                  function createDir($userName,$dirName){$userDir = '/users/'. $userName;if(strpos($dirName,'..') !== false){echo 'Directory name contains invalid sequence';return;}
                        //filter out '~' because other scripts identify user directories by this prefix
                        $dirName = str_replace('~','',$dirName);$newDir = $userDir . $dirName;mkdir($newDir, 0700);chown($newDir,$userName);}
               
               