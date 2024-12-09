
                  function createUserDir($username){$path = '/home/'.$username;if(!mkdir($path)){return false;}if(!chown($path,$username)){rmdir($path);return false;}return true;}
               
               