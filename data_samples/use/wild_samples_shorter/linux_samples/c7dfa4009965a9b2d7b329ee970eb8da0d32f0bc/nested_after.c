	/* If SMI is not intercepted, ignore guest SMI intercept as well  */
	if (!intercept_smi)
		vmcb_clr_intercept(c, INTERCEPT_SMI);

	vmcb_set_intercept(c, INTERCEPT_VMLOAD);
	vmcb_set_intercept(c, INTERCEPT_VMSAVE);
}

static void copy_vmcb_control_area(struct vmcb_control_area *dst,
				   struct vmcb_control_area *from)