{
	struct clk_device *cdev = to_clk_device(hw);
	u32 divisor;
	unsigned long flags = 0;

	if (rate == 0)
		return 0;

	divisor =  parent_rate / rate;

	/* If prate / rate would be decimal, incr the divisor */
	if (rate * divisor < *prate)
		divisor++;

	if (divisor == cdev->div_mask + 1)
		divisor = 0;

	/* SDMMC mask may need to be corrected before testing if its valid */
	if ((cdev->div_mask == 0x3F) && (divisor > 31)) {
		/*
		 * Bit 5 is a fixed /64 predivisor. If the requested divisor
		 * is >31 then correct for the fixed divisor being required.
		 */
		divisor = 0x20 + (divisor / 64);
	}