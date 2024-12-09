
					  int choose_bit(int reg_bit, int bit_number_from_elsewhere) 
					  {
					  
					    if (NEED_TO_SHIFT)
					    {
					    
					      reg_bit -= bit_number_from_elsewhere;
					    
					    }
					    return reg_bit;
					  
					  }
					  
					  unsigned int handle_io_register(unsigned int *r)
					  {
					  
					    int the_bit_number = choose_bit(5, 10);
					    if ((the_bit_number > 0) && (the_bit_number < 63))
					    {
					    
					      unsigned int the_bit = 1 << the_bit_number;
					      *r |= the_bit;
					    
					    }
					    return the_bit;
					  
					  }
					
					