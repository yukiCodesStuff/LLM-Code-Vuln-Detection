
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
					  
					    unsigned int the_bit = 1 << choose_bit(5, 10);
					    *r |= the_bit;
					    return the_bit;
					  
					  }
					
					