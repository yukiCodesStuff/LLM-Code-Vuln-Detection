
/* Physical address needed since MMU not enabled yet on secondary core */
#define AUX_CORE_BOOT0_PA			0x48281800

/*
 * OMAP5 specific entry point for secondary CPU to jump from ROM
 * code.  This routine also provides a holding flag into which
	bne	wait
	b	secondary_startup
ENDPROC(omap5_secondary_startup)
/*
 * OMAP4 specific entry point for secondary CPU to jump from ROM
 * code.  This routine also provides a holding flag into which
 * secondary core is held until we're ready for it to initialise.