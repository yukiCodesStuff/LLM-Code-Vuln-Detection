
                  // Parameterized Register module example 
				  // Secure_mode : REGISTER_DEFAULT[0] : When set to 1 register is read only and not writable// 
				  module register_example 
				  #( 
				  parameter REGISTER_WIDTH = 8, // Parameter defines width of register, default 8 bits 
				  parameter [REGISTER_WIDTH-1:0] REGISTER_DEFAULT = 2**REGISTER_WIDTH -2 // Default value of register computed from Width. Sets all bits to 1s except bit 0 (Secure _mode) 
				  ) 
				  ( 
				  input [REGISTER_WIDTH-1:0] Data_in, 
				  input Clk, 
				  input resetn, 
				  input write, 
				  output reg [REGISTER_WIDTH-1:0] Data_out 
				  ); 
				  
				  reg Secure_mode; 
				  
				  always @(posedge Clk or negedge resetn) 
				  
					if (~resetn) 
					begin 
					
					  Data_out <= REGISTER_DEFAULT; // Register content set to Default at reset 
					  Secure_mode <= REGISTER_DEFAULT[0]; // Register Secure_mode set at reset 
					
					end 
					else if (write & ~Secure_mode) 
					begin 
					
					  Data_out <= Data_in; 
					
					end 
				  
				  endmodule 
                  
                  
				  module register_top 
				  ( 
				  input Clk, 
				  input resetn, 
				  input write, 
				  input [31:0] Data_in, 
				  output reg [31:0] Secure_reg, 
				  output reg [31:0] Insecure_reg 
				  ); 
				  
				  register_example #( 
				  
					.REGISTER_WIDTH (32), 
					.REGISTER_DEFAULT (1224) // Incorrect Default value used bit 0 is 0. 
				  
				  ) Insecure_Device_ID_1 ( 
				  
					.Data_in (Data_in), 
					.Data_out (Secure_reg), 
					.Clk (Clk), 
					.resetn (resetn), 
					.write (write) 
				  
				  ); 
                  
				  register_example #(
				  
					.REGISTER_WIDTH (32) // Default not defined 2^32-2 value will be used as default. 
				  
				  ) Insecure_Device_ID_2 ( 
				  
					.Data_in (Data_in), 
					.Data_out (Insecure_reg), 
					.Clk (Clk), 
					.resetn (resetn), 
					.write (write) 
				  
				  ); 
                  
				  endmodule 
                
                