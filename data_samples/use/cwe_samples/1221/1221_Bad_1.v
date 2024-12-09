
					parameter  MEM_SIZE = 100;
					localparam JTAG_OFFSET = 81;
					
					const logic [MEM_SIZE-1:0][31:0] mem = {
					
						// JTAG expected hamc hash
						32'h49ac13af, 32'h1276f1b8, 32'h6703193a, 32'h65eb531b,
						32'h3025ccca, 32'h3e8861f4, 32'h329edfe5, 32'h98f763b4,
					
					...
					assign jtag_hash_o = {mem[JTAG_OFFSET-1],mem[JTAG_OFFSET-2],mem[JTAG_OFFSET-3],
					mem[JTAG_OFFSET-4],mem[JTAG_OFFSET-5],mem[JTAG_OFFSET-6],mem[JTAG_OFFSET-7],mem[JTAG_OFFSET-8]};
					...
					
					