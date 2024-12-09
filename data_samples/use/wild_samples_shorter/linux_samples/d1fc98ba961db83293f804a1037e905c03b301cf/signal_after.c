
		/* Set up to return from userspace.  */
		restorer = current->mm->context.vdso +
			selected_vdso32->sym___kernel_rt_sigreturn;
		if (ksig->ka.sa.sa_flags & SA_RESTORER)
			restorer = ksig->ka.sa.sa_restorer;
		put_user_ex(restorer, &frame->pretcode);
