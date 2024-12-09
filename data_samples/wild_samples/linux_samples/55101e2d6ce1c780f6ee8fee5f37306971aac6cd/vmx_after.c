		if (!is_paging(vcpu)) {
			hw_cr4 &= ~X86_CR4_PAE;
			hw_cr4 |= X86_CR4_PSE;
			/*
			 * SMEP/SMAP is disabled if CPU is in non-paging mode
			 * in hardware. However KVM always uses paging mode to
			 * emulate guest non-paging mode with TDP.
			 * To emulate this behavior, SMEP/SMAP needs to be
			 * manually disabled when guest switches to non-paging
			 * mode.
			 */
			hw_cr4 &= ~(X86_CR4_SMEP | X86_CR4_SMAP);
		} else if (!(cr4 & X86_CR4_PAE)) {
			hw_cr4 &= ~X86_CR4_PAE;
		}