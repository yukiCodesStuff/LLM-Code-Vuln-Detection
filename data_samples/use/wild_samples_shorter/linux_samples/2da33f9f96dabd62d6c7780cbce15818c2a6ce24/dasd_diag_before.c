 * and function code cmd.
 * In case of an exception return 3. Otherwise return result of bitwise OR of
 * resulting condition code and DIAG return code. */
static inline int dia250(void *iob, int cmd)
{
	register unsigned long reg2 asm ("2") = (unsigned long) iob;
	typedef union {
		struct dasd_diag_init_io init_io;
	int rc;

	rc = 3;
	diag_stat_inc(DIAG_STAT_X250);
	asm volatile(
		"	diag	2,%2,0x250\n"
		"0:	ipm	%0\n"
		"	srl	%0,28\n"
	return rc;
}

/* Initialize block I/O to DIAG device using the specified blocksize and
 * block offset. On success, return zero and set end_block to contain the
 * number of blocks on the device minus the specified offset. Return non-zero
 * otherwise. */