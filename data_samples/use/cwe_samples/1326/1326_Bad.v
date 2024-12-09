
						...
                            always_ff @(posedge clk_i) begin
                                if (req_i) begin
                                    if (!we_i) begin
										raddr_q <= addr_i[$clog2(RomSize)-1+3:3];
										end else begin
										mem[addr_i[$clog2(RomSize)-1+3:3]] <= wdata_i;
										end
									end
								end
                        	...
							
								// this prevents spurious Xes from propagating into the speculative fetch stage of the core
							
							assign rdata_o = (raddr_q < RomSize) ? mem[raddr_q] : '0;
							...
							
					
					