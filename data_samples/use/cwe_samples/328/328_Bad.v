
               ...
               logic [31:0] data_d, data_q
               logic [512-1:0] pass_data;
               ...
                  
                  Write: begin
                     
                     ...
                        
                        if (pass_mode) begin
                           
                           pass_data = { {60{8'h00}}, data_d};
                           state_d = PassChk;
                           pass_mode = 1'b0;
                           
                        ...
                        
                     
                  end
                  
               ...
               
	       