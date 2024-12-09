}
EXPORT_SYMBOL_GPL(ex_handler_fault);

/*
 * Handler for UD0 exception following a failed test against the
 * result of a refcount inc/dec/add/sub.
 */
bool ex_handler_refcount(const struct exception_table_entry *fixup,
			 struct pt_regs *regs, int trapnr)
{
	/* First unconditionally saturate the refcount. */
	*(int *)regs->cx = INT_MIN / 2;

	/*
	 * Strictly speaking, this reports the fixup destination, not
	 * the fault location, and not the actually overflowing
	 * instruction, which is the instruction before the "js", but
	 * since that instruction could be a variety of lengths, just
	 * report the location after the overflow, which should be close
	 * enough for finding the overflow, as it's at least back in
	 * the function, having returned from .text.unlikely.
	 */
	regs->ip = ex_fixup_addr(fixup);

	/*
	 * This function has been called because either a negative refcount
	 * value was seen by any of the refcount functions, or a zero
	 * refcount value was seen by refcount_dec().
	 *
	 * If we crossed from INT_MAX to INT_MIN, OF (Overflow Flag: result
	 * wrapped around) will be set. Additionally, seeing the refcount
	 * reach 0 will set ZF (Zero Flag: result was zero). In each of
	 * these cases we want a report, since it's a boundary condition.
	 *
	 */
	if (regs->flags & (X86_EFLAGS_OF | X86_EFLAGS_ZF)) {
		bool zero = regs->flags & X86_EFLAGS_ZF;

		refcount_error_report(regs, zero ? "hit zero" : "overflow");
	}

	return true;
}
EXPORT_SYMBOL_GPL(ex_handler_refcount);

bool ex_handler_ext(const struct exception_table_entry *fixup,
		   struct pt_regs *regs, int trapnr)
{
	/* Special hack for uaccess_err */