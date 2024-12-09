
		/*
		 * No indirect branch prediction barrier needed when switching
		 * the active VMCS within a vCPU, unless IBRS is advertised to
		 * the vCPU.  To minimize the number of IBPBs executed, KVM
		 * performs IBPB on nested VM-Exit (a single nested transition
		 * may switch the active VMCS multiple times).
		 */
		if (!buddy || WARN_ON_ONCE(buddy->vmcs != prev))
			indirect_branch_prediction_barrier();
	}