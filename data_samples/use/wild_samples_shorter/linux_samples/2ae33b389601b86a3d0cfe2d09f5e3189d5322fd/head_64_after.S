	.asciz	"SUNW,UltraSPARC-T"
prom_sparc_prefix:
	.asciz	"SPARC-"
prom_sparc64x_prefix:
	.asciz	"SPARC64-X"
	.align	4
prom_root_compatible:
	.skip	64
prom_cpu_compatible:
	cmp	%g2, 'T'
	be,pt	%xcc, 70f
	 cmp	%g2, 'M'
	bne,pn	%xcc, 49f
	 nop

70:	ldub	[%g1 + 7], %g2
	cmp	%g2, '3'
	cmp	%g2, '5'
	be,pt	%xcc, 5f
	 mov	SUN4V_CHIP_NIAGARA5, %g4
	ba,pt	%xcc, 49f
	 nop

91:	sethi	%hi(prom_cpu_compatible), %g1
	or	%g1, %lo(prom_cpu_compatible), %g1
	 mov	SUN4V_CHIP_NIAGARA2, %g4
	
4:
	/* Athena */
	sethi	%hi(prom_cpu_compatible), %g1
	or	%g1, %lo(prom_cpu_compatible), %g1
	sethi	%hi(prom_sparc64x_prefix), %g7
	or	%g7, %lo(prom_sparc64x_prefix), %g7
	mov	9, %g3
41:	ldub	[%g7], %g2
	ldub	[%g1], %g4
	cmp	%g2, %g4
	bne,pn	%icc, 49f
	add	%g7, 1, %g7
	subcc	%g3, 1, %g3
	bne,pt	%xcc, 41b
	add	%g1, 1, %g1
	mov	SUN4V_CHIP_SPARC64X, %g4
	ba,pt	%xcc, 5f
	nop

49:
	mov	SUN4V_CHIP_UNKNOWN, %g4
5:	sethi	%hi(sun4v_chip_type), %g2
	or	%g2, %lo(sun4v_chip_type), %g2
	stw	%g4, [%g2]