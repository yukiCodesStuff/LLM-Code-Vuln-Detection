};

static const struct gprefix pfx_0f_ae_7 = {
	I(0, em_clflush), N, N, N,
};

static const struct group_dual group15 = { {
	N, N, N, N, N, N, N, GP(0, &pfx_0f_ae_7),
	N, I(ImplicitOps | EmulateOnUD, em_syscall),
	II(ImplicitOps | Priv, em_clts, clts), N,
	DI(ImplicitOps | Priv, invd), DI(ImplicitOps | Priv, wbinvd), N, N,
	N, D(ImplicitOps | ModRM), N, N,
	/* 0x10 - 0x1F */
	N, N, N, N, N, N, N, N,
	D(ImplicitOps | ModRM), N, N, N, N, N, N, D(ImplicitOps | ModRM),
	/* 0x20 - 0x2F */
	DIP(ModRM | DstMem | Priv | Op3264 | NoMod, cr_read, check_cr_read),
	DIP(ModRM | DstMem | Priv | Op3264 | NoMod, dr_read, check_dr_read),
	IIP(ModRM | SrcMem | Priv | Op3264 | NoMod, em_cr_write, cr_write,