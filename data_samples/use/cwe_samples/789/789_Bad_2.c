
		 int proc_msg(char *s, int msg_len)
		 {
                  
		    // Note space at the end of the string - assume all strings have preamble with space
		    int pre_len = sizeof("preamble: ");
		    char buf[pre_len - msg_len];
		    ... Do processing here if we get this far
		  
		  }
		  char *s = "preamble: message\n";
		  char *sl = strchr(s, ':');        // Number of characters up to ':' (not including space)
		  int jnklen = sl == NULL ? 0 : sl - s;    // If undefined pointer, use zero length
		  int ret_val = proc_msg ("s",  jnklen);    // Violate assumption of preamble length, end up with negative value, blow out stack
               
               