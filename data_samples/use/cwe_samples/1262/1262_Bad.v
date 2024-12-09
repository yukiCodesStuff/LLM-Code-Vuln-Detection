
						if (csr_we || csr_read) begin
							
								if ((riscv::priv_lvl_t'(priv_lvl_o & csr_addr.csr_decode.priv_lvl) != csr_addr.csr_decode.priv_lvl) && !(csr_addr.address==riscv::CSR_MEPC)) begin
								
									csr_exception_o.cause = riscv::ILLEGAL_INSTR;
									csr_exception_o.valid = 1'b1;
								
								end
								// check access to debug mode only CSRs
								if (csr_addr_i[11:4] == 8'h7b && !debug_mode_q) begin
								
									csr_exception_o.cause = riscv::ILLEGAL_INSTR;
									csr_exception_o.valid = 1'b1;
								
								end
							
							end
						       
				
				