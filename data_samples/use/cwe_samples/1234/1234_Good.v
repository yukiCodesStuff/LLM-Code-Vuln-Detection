
		...
		always @(posedge clk_i)
			
			begin
				
				if(~(rst_ni && ~rst_9))
					
					begin
						
						for (j=0; j < 6; j=j+1) begin
							
							reglk_mem[j] <= 'h0;
							
						end
						
					end
					
				
			
		...
		
	 