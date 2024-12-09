
						if (csr_exception_i.valid && csr_exception_i.cause[63] && commit_instr_i[0].fu != CSR) begin
							
								exception_o = csr_exception_i;
								exception_o.tval = commit_instr_i[0].ex.tval;
							
							end
						       
					
					