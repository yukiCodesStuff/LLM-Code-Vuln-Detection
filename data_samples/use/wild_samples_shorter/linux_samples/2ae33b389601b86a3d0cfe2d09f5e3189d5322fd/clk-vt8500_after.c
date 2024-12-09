	divisor =  parent_rate / rate;

	/* If prate / rate would be decimal, incr the divisor */
	if (rate * divisor < parent_rate)
		divisor++;

	if (divisor == cdev->div_mask + 1)
		divisor = 0;