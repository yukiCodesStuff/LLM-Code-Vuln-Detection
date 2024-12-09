
#include "trace.h"

static void mmio_write_buf(char *buf, unsigned int len, unsigned long data)
{
	void *datap = NULL;
	union {
		u8	byte;
	memcpy(buf, datap, len);
}

static unsigned long mmio_read_buf(char *buf, unsigned int len)
{
	unsigned long data = 0;
	union {
		u16	hword;

	switch (len) {
	case 1:
		data = buf[0];
		break;
	case 2:
		memcpy(&tmp.hword, buf, len);
		data = tmp.hword;

/**
 * kvm_handle_mmio_return -- Handle MMIO loads after user space emulation
 * @vcpu: The VCPU pointer
 * @run:  The VCPU run struct containing the mmio data
 *
 * This should only be called after returning from userspace for MMIO load
 * emulation.
 */
int kvm_handle_mmio_return(struct kvm_vcpu *vcpu, struct kvm_run *run)
{
	unsigned long data;
		if (len > sizeof(unsigned long))
			return -EINVAL;

		data = mmio_read_buf(run->mmio.data, len);

		if (vcpu->arch.mmio_decode.sign_extend &&
		    len < sizeof(unsigned long)) {
			mask = 1U << ((len * 8) - 1);
					       len);

		trace_kvm_mmio(KVM_TRACE_MMIO_WRITE, len, fault_ipa, data);
		mmio_write_buf(data_buf, len, data);

		ret = kvm_io_bus_write(vcpu, KVM_MMIO_BUS, fault_ipa, len,
				       data_buf);
	} else {
	run->mmio.is_write	= is_write;
	run->mmio.phys_addr	= fault_ipa;
	run->mmio.len		= len;
	if (is_write)
		memcpy(run->mmio.data, data_buf, len);

	if (!ret) {
		/* We handled the access successfully in the kernel. */
		vcpu->stat.mmio_exit_kernel++;
		kvm_handle_mmio_return(vcpu, run);
		return 1;
	} else {
		vcpu->stat.mmio_exit_user++;
	}

	run->exit_reason	= KVM_EXIT_MMIO;
	return 0;
}