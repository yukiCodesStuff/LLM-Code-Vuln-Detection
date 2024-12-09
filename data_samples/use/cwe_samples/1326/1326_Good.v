
						...
                            always_ff @(posedge clk_i) begin
                                if (req_i) begin
										raddr_q <= addr_i[$clog2(RomSize)-1+3:3];
									end
								end
                        	...
							
								// this prevents spurious Xes from propagating into the speculative fetch stage of the core
							
							assign rdata_o = (raddr_q < RomSize) ? mem[raddr_q] : '0;
							...
							
					
				