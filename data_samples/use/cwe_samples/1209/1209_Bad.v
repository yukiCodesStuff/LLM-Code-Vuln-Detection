
		      reg gpio_out = 0;  //gpio should remain low for normal operation
                      
                      case (register_address)
		      
                        4'b1111 : //0x0F
			
			  begin
			  
                            gpio_out = 1;
			  
			  end
			
		      
                    
                    