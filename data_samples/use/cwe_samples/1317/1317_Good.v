
					module clint #(...
					) (
					... 
						
						input logic     acct_ctrl_i,
						
					...
					);
						
						logic     en, en_acct;
						
					...
						
						axi_lite_interface #(...
						) axi_lite_interface_i (
						
					...
						
						.en_o       ( en_acct    ),
						
					...
						
						);
						assign en = en_acct && acct_ctrl_i;
						
					...
					endmodule
					
				