
                  String command = new String("some cmd to execute & the password") MessageDigest encer = MessageDigest.getInstance("SHA");encer.update(command.getBytes("UTF-8"));byte[] digest = encer.digest();
               
            