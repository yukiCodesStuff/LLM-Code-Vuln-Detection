
                  def storePassword(userName,Password):hasher = hashlib.new('md5',b'SaltGoesHere')hasher.update(Password)hashedPassword = hasher.digest()
                        
                        # UpdateUserLogin returns True on success, False otherwise
                        return updateUserLogin(userName,hashedPassword)
                  
               
               