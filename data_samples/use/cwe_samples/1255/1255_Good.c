
           static nonvolatile password_tries = NUM_RETRIES;
           do
	   
             while (password_tries == 0) ; // Hang here if no more password tries
             password_tries--;  // Put retry code here to catch partial retries
             password_ok = 0;
             for (i = 0; i < NUM_PW_DIGITS; i++)
	     
               if (GetPasswordByte() == stored_password([i])
	       
		 password_ok |= 0x10; // Power consumption here
	       
               else
	       
		 password_ok |= 0x01; // is now the same here
	       
	     
             end
             if ((password_ok & 1) == 0)
	     
               password_tries = NUM_RETRIES;
               break_to_Ok_to_proceed
	     
	   
           while (true)
           // Password OK
          
	     