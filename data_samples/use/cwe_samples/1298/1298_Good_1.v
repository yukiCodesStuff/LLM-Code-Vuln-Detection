
					module dma # (...)(...);
					...
						
						input [7:0] [16-1:0] pmpcfg_i;   
						input logic [16-1:0][53:0]     pmpaddr_i;
						...
						reg [7:0] [16-1:0] pmpcfg_reg;
						reg [16-1:0][53:0] pmpaddr_reg;
						...
						//// Save the input command
						always @ (posedge clk_i or negedge rst_ni)
							
							begin: save_inputs
							if (!rst_ni)
								
								begin
								...
								pmpaddr_reg <= 'b0 ;
								pmpcfg_reg <= 'b0 ;
								end
								
							else 
								
								begin
									
									if (dma_ctrl_reg == CTRL_IDLE || dma_ctrl_reg == CTRL_DONE) 
									begin
									...
									pmpaddr_reg <= pmpaddr_i;
									pmpcfg_reg <= pmpcfg_i;
									end
									
								end 
								
							end // save_inputs
							...
							// Load/store PMP check
							pmp #(
								
								.XLEN       ( 64                     ),
								.PMP_LEN    ( 54                     ),
								.NR_ENTRIES ( 16           )
								
							) i_pmp_data (
								
								.addr_i        ( pmp_addr_reg        ),
								.priv_lvl_i    ( riscv::PRIV_LVL_U   ), // we intend to apply filter on
								// DMA always, so choose the least privilege
								.access_type_i ( pmp_access_type_reg ),
								// Configuration
								.conf_addr_i   ( pmpaddr_reg           ),
								.conf_i        ( pmpcfg_reg            ),
								.allow_o       ( pmp_data_allow      )
								
							);
							
						
					endmodule
					
				