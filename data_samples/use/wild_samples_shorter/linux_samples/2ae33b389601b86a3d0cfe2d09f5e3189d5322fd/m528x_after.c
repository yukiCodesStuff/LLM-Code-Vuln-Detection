	u8 port;

	/* make sure PUAPAR is set for UART0 and UART1 */
	port = readb(MCFGPIO_PUAPAR);
	port |= 0x03 | (0x03 << 2);
	writeb(port, MCFGPIO_PUAPAR);
}
