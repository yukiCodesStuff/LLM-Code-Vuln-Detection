
/* Verify next sizeof(t) bytes can be on the same instruction */
#define validate_next(t, insn, n)	\
	((insn)->next_byte + sizeof(t) + n < (insn)->end_kaddr)

#define __get_next(t, insn)	\
	({ t r = *(t*)insn->next_byte; insn->next_byte += sizeof(t); r; })
