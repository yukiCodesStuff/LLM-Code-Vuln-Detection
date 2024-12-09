			kvm_rip_write(vcpu, ctxt->eip);
			if (r && ctxt->tf)
				r = kvm_vcpu_do_singlestep(vcpu);
			__kvm_set_rflags(vcpu, ctxt->eflags);
		}

		/*