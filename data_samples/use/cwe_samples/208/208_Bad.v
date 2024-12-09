
                    always_comb @ (posedge clk)
                    
                    begin
		    
                      assign check_pass[3:0] = 4'b0;
                      for (i = 0; i < 4; i++) begin
		      
                    	if (entered_pass[(i*8 - 1) : i] eq golden_pass([i*8 - 1) : i])
			
			  assign check_pass[i] = 1;
                    	  continue;
			
                    	else
			
                    	  assign check_pass[i] = 0;
                    	  break;
			
                    	end
		      
                      assign grant_access = (check_pass == 4'b1111) ? 1'b1: 1'b0;
		    
                    end
		
                