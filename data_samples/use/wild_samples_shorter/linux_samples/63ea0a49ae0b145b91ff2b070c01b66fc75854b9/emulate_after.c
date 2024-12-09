};

static const struct opcode group6[] = {
	DI(Prot | DstMem,	sldt),
	DI(Prot | DstMem,	str),
	II(Prot | Priv | SrcMem16, em_lldt, lldt),
	II(Prot | Priv | SrcMem16, em_ltr, ltr),
	N, N, N, N,
};