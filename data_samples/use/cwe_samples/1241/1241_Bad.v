
						reg in_sr, entropy16_valid;
						reg [15:0] entropy16;
						
						assign entropy16_o = entropy16;
						assign entropy16_valid_o = entropy16_valid;
						
						always @ (*)
						begin
						
							in_sr = ^ (poly_i [15:0] & entropy16 [15:0]);
						
						end
					
					