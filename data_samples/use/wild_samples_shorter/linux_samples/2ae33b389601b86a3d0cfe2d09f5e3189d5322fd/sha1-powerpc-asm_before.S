	STEPUP4((t)+16, fn)

_GLOBAL(powerpc_sha_transform)
	PPC_STLU r1,-STACKFRAMESIZE(r1)
	SAVE_8GPRS(14, r1)
	SAVE_10GPRS(22, r1)

	/* Load up A - E */

	REST_8GPRS(14, r1)
	REST_10GPRS(22, r1)
	addi	r1,r1,STACKFRAMESIZE
	blr