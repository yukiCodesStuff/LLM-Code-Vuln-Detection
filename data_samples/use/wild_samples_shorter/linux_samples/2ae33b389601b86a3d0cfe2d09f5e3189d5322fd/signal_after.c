#include <sysdep/mcontext.h>
#include "internal.h"

void (*sig_info[NSIG])(int, struct siginfo *, struct uml_pt_regs *) = {
	[SIGTRAP]	= relay_signal,
	[SIGFPE]	= relay_signal,
	[SIGILL]	= relay_signal,
	[SIGWINCH]	= winch,