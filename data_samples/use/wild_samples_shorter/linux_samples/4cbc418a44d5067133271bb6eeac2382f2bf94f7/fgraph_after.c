	return 0;
}

/*
 * Not all archs define MCOUNT_INSN_SIZE which is used to look for direct
 * functions. But those archs currently don't support direct functions
 * anyway, and ftrace_find_rec_direct() is just a stub for them.
 * Define MCOUNT_INSN_SIZE to keep those archs compiling.
 */
#ifndef MCOUNT_INSN_SIZE
/* Make sure this only works without direct calls */
# ifdef CONFIG_DYNAMIC_FTRACE_WITH_DIRECT_CALLS
#  error MCOUNT_INSN_SIZE not defined with direct calls enabled
# endif
# define MCOUNT_INSN_SIZE 0
#endif

int function_graph_enter(unsigned long ret, unsigned long func,
			 unsigned long frame_pointer, unsigned long *retp)
{
	struct ftrace_graph_ent trace;