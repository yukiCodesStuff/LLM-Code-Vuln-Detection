
					
					...
						
						bootrom i_bootrom_patch (
							
							.clk_i                   ,
							.req_i      ( rom_req   ),
							.addr_i     ( rom_addr  ),
							.rdata_o    ( rom_rdata_patch )
							
						);
						bootrom_linux i_bootrom_linux (
							
							.clk_i                   ,
							.req_i      ( rom_req   ),
							.addr_i     ( rom_addr  ),
							.rdata_o    ( rom_rdata_linux )
							
						);
						
					assign rom_rdata = (ariane_boot_sel_i) ? rom_rdata_patch : rom_rdata_linux;
					...
					
				