
	movi	a2, cpu_start_ccount
1:
	memw
	l32i	a3, a2, 0
	beqi	a3, 0, 1b
	movi	a3, 0
	s32i	a3, a2, 0
1:
	memw
	l32i	a3, a2, 0
	beqi	a3, 0, 1b
	wsr	a3, ccount
	movi	a3, 0
	rsr	a0, prid
	neg	a2, a0
	movi	a3, cpu_start_id
	memw
	s32i	a2, a3, 0
#if XCHAL_DCACHE_IS_WRITEBACK
	dhwbi	a3, 0
#endif
1:
	memw
	l32i	a2, a3, 0
	dhi	a3, 0
	bne	a2, a0, 1b
