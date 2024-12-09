	return 1;
}

/*
 * The exit handlers return 1 if the exit was handled fully and guest execution
 * may resume.  Otherwise they set the kvm_run parameter to indicate what needs
 * to be done to userspace and return 0.
	[EXIT_REASON_MWAIT_INSTRUCTION]	      = handle_mwait,
	[EXIT_REASON_MONITOR_INSTRUCTION]     = handle_monitor,
	[EXIT_REASON_INVEPT]                  = handle_invept,
};

static const int kvm_vmx_max_exit_handlers =
	ARRAY_SIZE(kvm_vmx_exit_handlers);
	case EXIT_REASON_VMPTRST: case EXIT_REASON_VMREAD:
	case EXIT_REASON_VMRESUME: case EXIT_REASON_VMWRITE:
	case EXIT_REASON_VMOFF: case EXIT_REASON_VMON:
	case EXIT_REASON_INVEPT:
		/*
		 * VMX instructions trap unconditionally. This allows L1 to
		 * emulate them for its L2 guest, i.e., allows 3-level nesting!
		 */