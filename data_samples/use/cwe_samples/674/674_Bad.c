
	    void do_something_recursive (int flg)
	    {
	    
	      ... // Do some real work here, but the value of flg is unmodified
	      if (flg) { do_something_recursive (flg); }    // flg is never modified so it is always TRUE - this call will continue until the stack explodes
	    
	    }
	    int flag = 1; // Set to TRUE
	    do_something_recursive (flag);
          
          