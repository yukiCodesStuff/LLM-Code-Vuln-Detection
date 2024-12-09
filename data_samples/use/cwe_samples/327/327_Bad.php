
                  function encryptPassword($password){$iv_size = mcrypt_get_iv_size(MCRYPT_DES, MCRYPT_MODE_ECB);$iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);$key = "This is a password encryption key";$encryptedPassword = mcrypt_encrypt(MCRYPT_DES, $key, $password, MCRYPT_MODE_ECB, $iv);return $encryptedPassword;}
               
               