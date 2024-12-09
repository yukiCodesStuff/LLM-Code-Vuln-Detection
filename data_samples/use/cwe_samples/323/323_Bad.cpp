
                  String command = new String("some command to execute");MessageDigest nonce = MessageDigest.getInstance("SHA");nonce.update(String.valueOf("bad nonce"));byte[] nonce = nonce.digest();MessageDigest password = MessageDigest.getInstance("SHA");password.update(nonce + "secretPassword");byte[] digest = password.digest();sendCommand(digest, command)
               
               