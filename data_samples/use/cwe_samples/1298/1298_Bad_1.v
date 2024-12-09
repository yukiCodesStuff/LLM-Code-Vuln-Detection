
					module dma # (...)(...);
					...
						
						input [7:0] [16-1:0] pmpcfg_i;
						input logic [16-1:0][53:0]     pmpaddr_i;
						...
						//// Save the input command
  						always @ (posedge clk_i or negedge rst_ni)
							
							begin: save_inputs
							if (!rst_ni)
								
								begin
								...
								end
								
							else
								
								begin
									
									if (dma_ctrl_reg == CTRL_IDLE || dma_ctrl_reg == CTRL_DONE)
									begin
									...
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
								.priv_lvl_i    ( riscv::PRIV_LVL_U   ),
								.access_type_i ( pmp_access_type_reg ),
								// Configuration
								.conf_addr_i   ( pmpaddr_i           ),
								.conf_i        ( pmpcfg_i            ),
								.allow_o       ( pmp_data_allow      )
								
							);
							
						
					endmodule
					
					