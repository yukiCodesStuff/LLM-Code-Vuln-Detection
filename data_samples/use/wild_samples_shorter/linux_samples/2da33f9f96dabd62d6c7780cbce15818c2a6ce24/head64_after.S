
__HEAD
ENTRY(startup_continue)
	tm	__LC_STFLE_FAC_LIST+5,0x80	# LPP available ?
	jz	0f
	xc	__LC_LPP+1(7,0),__LC_LPP+1	# clear lpp and current_pid
	mvi	__LC_LPP,0x80			#   and set LPP_MAGIC
	.insn	s,0xb2800000,__LC_LPP		# load program parameter