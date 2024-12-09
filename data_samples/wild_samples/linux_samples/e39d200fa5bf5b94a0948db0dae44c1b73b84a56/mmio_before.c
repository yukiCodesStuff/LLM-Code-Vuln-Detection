		    len < sizeof(unsigned long)) {
			mask = 1U << ((len * 8) - 1);
			data = (data ^ mask) - mask;
		}

		trace_kvm_mmio(KVM_TRACE_MMIO_READ, len, run->mmio.phys_addr,
			       data);
		data = vcpu_data_host_to_guest(vcpu, data, len);
		vcpu_set_reg(vcpu, vcpu->arch.mmio_decode.rt, data);
	}

	return 0;
}

static int decode_hsr(struct kvm_vcpu *vcpu, bool *is_write, int *len)
{
	unsigned long rt;
	int access_size;
	bool sign_extend;

	if (kvm_vcpu_dabt_iss1tw(vcpu)) {
		/* page table accesses IO mem: tell guest to fix its TTBR */
		kvm_inject_dabt(vcpu, kvm_vcpu_get_hfar(vcpu));
		return 1;
	}

	access_size = kvm_vcpu_dabt_get_as(vcpu);
	if (unlikely(access_size < 0))
		return access_size;

	*is_write = kvm_vcpu_dabt_iswrite(vcpu);
	sign_extend = kvm_vcpu_dabt_issext(vcpu);
	rt = kvm_vcpu_dabt_get_rd(vcpu);

	*len = access_size;
	vcpu->arch.mmio_decode.sign_extend = sign_extend;
	vcpu->arch.mmio_decode.rt = rt;

	/*
	 * The MMIO instruction is emulated and should not be re-executed
	 * in the guest.
	 */
	kvm_skip_instr(vcpu, kvm_vcpu_trap_il_is32bit(vcpu));
	return 0;
}

int io_mem_abort(struct kvm_vcpu *vcpu, struct kvm_run *run,
		 phys_addr_t fault_ipa)
{
	unsigned long data;
	unsigned long rt;
	int ret;
	bool is_write;
	int len;
	u8 data_buf[8];

	/*
	 * Prepare MMIO operation. First decode the syndrome data we get
	 * from the CPU. Then try if some in-kernel emulation feels
	 * responsible, otherwise let user space do its magic.
	 */
	if (kvm_vcpu_dabt_isvalid(vcpu)) {
		ret = decode_hsr(vcpu, &is_write, &len);
		if (ret)
			return ret;
	} else {
		kvm_err("load/store instruction decoding not implemented\n");
		return -ENOSYS;
	}
	if (is_write) {
		data = vcpu_data_guest_to_host(vcpu, vcpu_get_reg(vcpu, rt),
					       len);

		trace_kvm_mmio(KVM_TRACE_MMIO_WRITE, len, fault_ipa, data);
		kvm_mmio_write_buf(data_buf, len, data);

		ret = kvm_io_bus_write(vcpu, KVM_MMIO_BUS, fault_ipa, len,
				       data_buf);
	} else {
		trace_kvm_mmio(KVM_TRACE_MMIO_READ_UNSATISFIED, len,
			       fault_ipa, 0);

		ret = kvm_io_bus_read(vcpu, KVM_MMIO_BUS, fault_ipa, len,
				      data_buf);
	}