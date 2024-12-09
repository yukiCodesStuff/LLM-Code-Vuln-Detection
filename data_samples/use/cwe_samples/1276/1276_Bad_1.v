
				...
				csr_regfile #(
					
					...
					
				) csr_regfile_i (
					
					.flush_o             ( flush_csr_ctrl ),
					.halt_csr_o          ( halt_csr_ctrl ),
					...
					.irq_i(),
					.time_irq_i(),
					.*
					
				);
				...
				
				