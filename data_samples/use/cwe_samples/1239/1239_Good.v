
					...
						
						// Implement SHA256 I/O memory map interface
						// Write side
						always @(posedge clk_i)
							
							begin
								
								if(~(rst_ni && ~rst_3))
									
									begin
										
										startHash <= 0;
										newMessage <= 0;
										data[0] <= 0;
										data[1] <= 0;
										data[2] <= 0;
										...
										data[14] <= 0;
										data[15] <= 0;
										
									end
									
								else if(hashValid && ~hashValid_r)
									
									begin
										
										data[0] <= 0;
										data[1] <= 0;
										data[2] <= 0;
										...
										data[14] <= 0;
										data[15] <= 0;
										
									end
									
								
							
						
					...
					
				