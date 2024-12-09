
					...
						
							localparam logic[63:0] PLICLength = 64'h03FF_FFFF;
							localparam logic[63:0] UARTLength = 64'h0000_1000;
							localparam logic[63:0] AESLength = 64'h0000_1000;
							localparam logic[63:0] SPILength = 64'h0080_0000;
						
					...
						
							typedef enum logic [63:0] {
							
								...
								PLICBase = 64'h0C00_0000,
								UARTBase = 64'h1000_0000,
								AESBase = 64'h1010_0000,
								SPIBase = 64'h2000_0000,
								...
							
						       
					
               
			