
#include "trace.h"

void kvm_mmio_write_buf(void *buf, unsigned int len, unsigned long data)
{
	void *datap = NULL;
	union {
		u8	byte;
	memcpy(buf, datap, len);
}

unsigned long kvm_mmio_read_buf(const void *buf, unsigned int len)
{
	unsigned long data = 0;
	union {
		u16	hword;

	switch (len) {
	case 1:
		data = *(u8 *)buf;
		break;
	case 2:
		memcpy(&tmp.hword, buf, len);
		data = tmp.hword;

/**
 * kvm_handle_mmio_return -- Handle MMIO loads after user space emulation
 *			     or in-kernel IO emulation
 *
 * @vcpu: The VCPU pointer
 * @run:  The VCPU run struct containing the mmio data
 */
int kvm_handle_mmio_return(struct kvm_vcpu *vcpu, struct kvm_run *run)
{
	unsigned long data;
		if (len > sizeof(unsigned long))
			return -EINVAL;

		data = kvm_mmio_read_buf(run->mmio.data, len);

		if (vcpu->arch.mmio_decode.sign_extend &&
		    len < sizeof(unsigned long)) {
			mask = 1U << ((len * 8) - 1);
					       len);

		trace_kvm_mmio(KVM_TRACE_MMIO_WRITE, len, fault_ipa, data);
		kvm_mmio_write_buf(data_buf, len, data);

		ret = kvm_io_bus_write(vcpu, KVM_MMIO_BUS, fault_ipa, len,
				       data_buf);
	} else {
	run->mmio.is_write	= is_write;
	run->mmio.phys_addr	= fault_ipa;
	run->mmio.len		= len;

	if (!ret) {
		/* We handled the access successfully in the kernel. */
		if (!is_write)
			memcpy(run->mmio.data, data_buf, len);
		vcpu->stat.mmio_exit_kernel++;
		kvm_handle_mmio_return(vcpu, run);
		return 1;
	}

	if (is_write)
		memcpy(run->mmio.data, data_buf, len);
	vcpu->stat.mmio_exit_user++;
	run->exit_reason	= KVM_EXIT_MMIO;
	return 0;
}