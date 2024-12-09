
	    void do_something_recursive (int flg)
	    {
	    
	      ... // Do some real work here
	      // Modify value of flg on done condition
	      if (flg) { do_something_recursive (flg); }    // returns when flg changes to 0
	    
	    }
	    int flag = 1; // Set to TRUE
	    do_something_recursive (flag);
          
        