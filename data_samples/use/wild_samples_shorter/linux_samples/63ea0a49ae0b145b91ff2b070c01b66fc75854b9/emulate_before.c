};

static const struct opcode group6[] = {
	DI(Prot,	sldt),
	DI(Prot,	str),
	II(Prot | Priv | SrcMem16, em_lldt, lldt),
	II(Prot | Priv | SrcMem16, em_ltr, ltr),
	N, N, N, N,
};