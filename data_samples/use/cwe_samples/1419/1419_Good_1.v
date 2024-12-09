
						module acct_wrapper #(
						...
						
							always @(posedge clk_i)
							
								begin
								
									if(~(rst_ni && ~rst_6))
									
										begin
										
											for (j=0; j < AcCt_MEM_SIZE; j=j+1)
											
												begin
												
													acct_mem[j] <= 32'h00000000;
												
												end
											
										
										end
									
								
								...
							
						
					
				