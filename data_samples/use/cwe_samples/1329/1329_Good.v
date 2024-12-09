
					module dmi_jtag(...
					) (
						
						input logic [255:0] hmac_key_i,
						input logic         hmac_patch_en,
						... 
						reg [255:0] hmac_key_reg;
						...
						
					);
					...
						
						always_ff @(posedge tck_i or negedge trst_ni) begin
						...
						if (hmac_patch_en)
							
							hmac_key_reg <= hmac_key_i;
							
						...
						end
						
					...
						
						hmac hmac(
						...
						.key_i(hmac_key_reg),
						...
						);
						
					...
					endmodule
					
				