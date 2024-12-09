
					
					  // Gets the size from the number of objects in a database, which over time can conceivably get very large
					  int end_limit = get_nmbr_obj_from_db();
					  int i;
					  int *base = NULL;
					  int *p =base;
					  for (i = 0; i < end_limit; i++)
					  {
					    
					      *p = alloca(sizeof(int *)); // Allocate memory on the stack
					      p = *p; // // Point to the next location to be saved
					    
					    }
					
					
					