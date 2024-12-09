
extern const ulong vmx_return;

struct kvm_vmx {
	struct kvm kvm;

	unsigned int tss_addr;
	.enable_smi_window = enable_smi_window,
};

static int __init vmx_init(void)
{
	int r;

	}
#endif

	r = kvm_init(&vmx_x86_ops, sizeof(struct vcpu_vmx),
                     __alignof__(struct vcpu_vmx), THIS_MODULE);
	if (r)
		return r;