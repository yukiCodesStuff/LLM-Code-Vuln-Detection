
						// 2x1 Multiplexor using logic-gates
                        
						module glitchEx(
						
							input wire in0, in1, sel,
							output wire z
						
						);
                        
						wire not_sel;
						wire and_out1, and_out2;
						
						assign not_sel = ~sel;
						assign and_out1 = not_sel & in0;
						assign and_out2 = sel & in1;
                        
						// Buggy line of code:
						assign z = and_out1 | and_out2; // glitch in signal z
						
						endmodule
					
					