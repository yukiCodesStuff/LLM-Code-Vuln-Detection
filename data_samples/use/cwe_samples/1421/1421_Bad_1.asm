
				  
				  1 LDR X1, [X2] ; arranged to miss in the cache
				  2 CBZ X1, over ; This will be taken 
				  3 MRS X3, TTBR0_EL1; 
				  4 LSL X3, X3, #imm 
				  5 AND X3, X3, #0xFC0
				  6 LDR X5, [X6,X3] ; X6 is an EL0 base address
				  7 over
				
			  