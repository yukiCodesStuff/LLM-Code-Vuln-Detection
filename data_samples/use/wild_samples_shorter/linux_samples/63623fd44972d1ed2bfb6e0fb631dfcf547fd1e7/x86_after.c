			kvm_rip_write(vcpu, ctxt->eip);
			if (r && ctxt->tf)
				r = kvm_vcpu_do_singlestep(vcpu);
			if (kvm_x86_ops->update_emulated_instruction)
				kvm_x86_ops->update_emulated_instruction(vcpu);
			__kvm_set_rflags(vcpu, ctxt->eflags);
		}

		/*