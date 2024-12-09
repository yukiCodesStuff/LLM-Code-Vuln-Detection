
					module dmi_jtag(...)(...);
					...
						
							
								
								PassChkValid: begin
								if(hashValid) begin
									
										
										if(exp_hash == pass_hash) begin
											 
											pass_check = 1'b1;
											
										end else begin
											
											pass_check = 1'b0;
											
										end
										state_d = Idle;
										
									end else begin
									state_d = PassChkValid;
									end
									
								end
								
							
						
					...
						
						hmac hmac(
						
					...
						
							
							.key_i(256'h24e6fa2254c2ff632a41b...),
							
						
					...
						
						);
						
					...
					endmodule
					
					