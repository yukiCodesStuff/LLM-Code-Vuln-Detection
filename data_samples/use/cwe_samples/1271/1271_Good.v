
					  always @(posedge clk) begin
					  
					    if (~reset) lock_jtag <= 0;
					    else if (en) lock_jtag <= d;
					  
					  end
					
				