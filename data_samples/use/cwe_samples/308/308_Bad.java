
                  String plainText = new String(plainTextIn);MessageDigest encer = MessageDigest.getInstance("SHA");encer.update(plainTextIn);byte[] digest = password.digest();
                     //Login if hash matches stored hash
                     if (equal(digest,secret_password())) {login_user();}
               
               