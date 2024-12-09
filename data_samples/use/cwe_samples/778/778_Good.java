
                  if LoginUser(){
                     // Login successful
		     log.warn("Login by user successful.");
		     RunProgram();
                     } else {
                     
		       // Login unsuccessful
		       log.warn("Login attempt by user failed, trying again.");
		     LoginRetry();
                  }
               
            