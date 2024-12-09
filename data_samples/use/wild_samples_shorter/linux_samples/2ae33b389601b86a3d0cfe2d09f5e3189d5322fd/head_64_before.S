	.asciz	"SUNW,UltraSPARC-T"
prom_sparc_prefix:
	.asciz	"SPARC-"
	.align	4
prom_root_compatible:
	.skip	64
prom_cpu_compatible:
	cmp	%g2, 'T'
	be,pt	%xcc, 70f
	 cmp	%g2, 'M'
	bne,pn	%xcc, 4f
	 nop

70:	ldub	[%g1 + 7], %g2
	cmp	%g2, '3'
	cmp	%g2, '5'
	be,pt	%xcc, 5f
	 mov	SUN4V_CHIP_NIAGARA5, %g4
	ba,pt	%xcc, 4f
	 nop

91:	sethi	%hi(prom_cpu_compatible), %g1
	or	%g1, %lo(prom_cpu_compatible), %g1
	 mov	SUN4V_CHIP_NIAGARA2, %g4
	
4:
	mov	SUN4V_CHIP_UNKNOWN, %g4
5:	sethi	%hi(sun4v_chip_type), %g2
	or	%g2, %lo(sun4v_chip_type), %g2
	stw	%g4, [%g2]