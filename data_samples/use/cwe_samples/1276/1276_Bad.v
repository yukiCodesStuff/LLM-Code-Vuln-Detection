
			      // IP definition
			      module tz_peripheral(clk, reset, data_in, data_in_security_level, ...);
			      
				input clk, reset;
				input [31:0] data_in;
				input data_in_security_level;
				...
			      
			      endmodule
			      // Instantiation of IP in a parent system
			      module soc(...)
			      
				...
				tz_peripheral u_tz_peripheral(
				
				  .clk(clk),
				  .rst(rst),
				  .data_in(rdata),
				  //Copy-and-paste error or typo grounds data_in_security_level (in this example 0=secure, 1=non-secure) effectively promoting all data to "secure")
				  .data_in_security_level(1'b0),
				
				);
				...
			      
			      endmodule
			    
			    