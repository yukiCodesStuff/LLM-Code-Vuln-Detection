
		   
		   module aes0_wrapper #(...)(...);
		   ...
		   always @(posedge clk_i)
		   
			 begin
			 
			   if(~(rst_ni && ~rst_1)) //clear p_c[i] at reset
			   
				 begin
				 
                   start <= 0;
                   p_c[0] <= 0;
                   p_c[1] <= 0;
                   p_c[2] <= 0;
                   p_c[3] <= 0;
                   ...
				 
				 end
			   
			   else if(en && we)
			   
				 case(address[8:3])
				 
                   0:
				   
                     start  <= reglk_ctrl_i[1] ? start  : wdata[0];
				   
                   1:
				   
                     p_c[3] <= reglk_ctrl_i[3] ? p_c[3] : wdata[31:0];
				   
                   2:
				   
                     p_c[2] <= reglk_ctrl_i[3] ? p_c[2] : wdata[31:0];
				   
                   3:
				   
                     p_c[1] <= reglk_ctrl_i[3] ? p_c[1] : wdata[31:0];
				   
                   4:
				   
                     p_c[0] <= reglk_ctrl_i[3] ? p_c[0] : wdata[31:0];
				   
                   ...
				 
				 endcase
			   
			   end // always @ (posedge wb_clk_i)
			 
			 endmodule
		   
		 
		 