	clk_prepare_enable(clk[gpio3_gate]);
	clk_prepare_enable(clk[iim_gate]);
	clk_prepare_enable(clk[emi_gate]);

	/*
	 * SCC is needed to boot via mmc after a watchdog reset. The clock code
	 * before conversion to common clk also enabled UART1 (which isn't