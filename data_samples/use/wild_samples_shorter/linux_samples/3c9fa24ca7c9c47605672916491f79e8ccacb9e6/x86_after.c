
static int emulator_read_std(struct x86_emulate_ctxt *ctxt,
			     gva_t addr, void *val, unsigned int bytes,
			     struct x86_exception *exception, bool system)
{
	struct kvm_vcpu *vcpu = emul_to_vcpu(ctxt);
	u32 access = 0;

	if (!system && kvm_x86_ops->get_cpl(vcpu) == 3)
		access |= PFERR_USER_MASK;

	return kvm_read_guest_virt_helper(addr, val, bytes, vcpu, access, exception);
}

static int kvm_read_guest_phys_system(struct x86_emulate_ctxt *ctxt,
		unsigned long addr, void *val, unsigned int bytes)
}

static int emulator_write_std(struct x86_emulate_ctxt *ctxt, gva_t addr, void *val,
			      unsigned int bytes, struct x86_exception *exception,
			      bool system)
{
	struct kvm_vcpu *vcpu = emul_to_vcpu(ctxt);
	u32 access = PFERR_WRITE_MASK;

	if (!system && kvm_x86_ops->get_cpl(vcpu) == 3)
		access |= PFERR_USER_MASK;

	return kvm_write_guest_virt_helper(addr, val, bytes, vcpu,
					   access, exception);
}

int kvm_write_guest_virt_system(struct kvm_vcpu *vcpu, gva_t addr, void *val,
				unsigned int bytes, struct x86_exception *exception)
	struct x86_exception e;

	if (force_emulation_prefix &&
	    kvm_read_guest_virt(vcpu, kvm_get_linear_rip(vcpu),
				sig, sizeof(sig), &e) == 0 &&
	    memcmp(sig, "\xf\xbkvm", sizeof(sig)) == 0) {
		kvm_rip_write(vcpu, kvm_rip_read(vcpu) + sizeof(sig));
		emul_type = 0;
	}