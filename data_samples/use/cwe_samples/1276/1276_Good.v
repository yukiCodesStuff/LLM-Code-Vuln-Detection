
			      // Instantiation of IP in a parent system
			      module soc(...)
			      
				...
				tz_peripheral u_tz_peripheral(
				
				  .clk(clk),
				  .rst(rst),
				  .data_in(rdata),
				  // This port is no longer grounded, but instead driven by the appropriate signal
				  .data_in_security_level(rdata_security_level),
				
				);
				...
			      
			      endmodule
			    
			  