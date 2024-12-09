
		   
		   module aes0_wrapper #(...)(...);
		   ...
		   always @(posedge clk_i)
		   
			 begin
			 
			   if(~(rst_ni && ~rst_1)) //clear p_c[i] at reset
			   
				 ...
			   
			   else if(ct_valid) //encryption process complete, clear p_c[i]
			   
				 begin
				 
				   p_c[0] <= 0;
				   p_c[1] <= 0;
				   p_c[2] <= 0;
				   p_c[3] <= 0;
				 
				 end
			   
			   else if(en && we)
			   
				 case(address[8:3])
				 ...
				 endcase
			   
			   end // always @ (posedge wb_clk_i)
			 
			 endmodule
			 
		   
		 
	   