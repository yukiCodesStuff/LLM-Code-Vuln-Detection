
					  module fsm_1(out, user_input, clk, rst_n);
					  input [2:0] user_input; 
					  input clk, rst_n;
					  output reg [2:0] out;
					  reg [1:0] state;
					  always @ (posedge clk or negedge rst_n )
					  
					  begin
					  
					  
						if (!rst_n)
						
						  state = 3'h0;
						
						else
						case (user_input)
						
						  3'h0:
						  3'h1:
						  3'h2:
						  3'h3: state = 2'h3;
						  3'h4: state = 2'h2;
						  3'h5: state = 2'h1;
						
						endcase
					  
					  end
					  out <= {1'h1, state};
					  
					  endmodule
					  
					
					