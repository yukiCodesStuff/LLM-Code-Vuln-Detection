#include <sysdep/mcontext.h>
#include "internal.h"

void (*sig_info[NSIG])(int, siginfo_t *, struct uml_pt_regs *) = {
	[SIGTRAP]	= relay_signal,
	[SIGFPE]	= relay_signal,
	[SIGILL]	= relay_signal,
	[SIGWINCH]	= winch,