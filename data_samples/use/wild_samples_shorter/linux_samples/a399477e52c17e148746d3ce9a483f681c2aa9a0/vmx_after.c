
extern const ulong vmx_return;

static DEFINE_STATIC_KEY_FALSE(vmx_l1d_should_flush);

/* These MUST be in sync with vmentry_l1d_param order. */
enum vmx_l1d_flush_state {
	VMENTER_L1D_FLUSH_NEVER,
	VMENTER_L1D_FLUSH_COND,
	VMENTER_L1D_FLUSH_ALWAYS,
};

static enum vmx_l1d_flush_state __read_mostly vmentry_l1d_flush = VMENTER_L1D_FLUSH_COND;

static const struct {
	const char *option;
	enum vmx_l1d_flush_state cmd;
} vmentry_l1d_param[] = {
	{"never",	VMENTER_L1D_FLUSH_NEVER},
	{"cond",	VMENTER_L1D_FLUSH_COND},
	{"always",	VMENTER_L1D_FLUSH_ALWAYS},
};

static int vmentry_l1d_flush_set(const char *s, const struct kernel_param *kp)
{
	unsigned int i;

	if (!s)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(vmentry_l1d_param); i++) {
		if (!strcmp(s, vmentry_l1d_param[i].option)) {
			vmentry_l1d_flush = vmentry_l1d_param[i].cmd;
			return 0;
		}
	}

	return -EINVAL;
}

static int vmentry_l1d_flush_get(char *s, const struct kernel_param *kp)
{
	return sprintf(s, "%s\n", vmentry_l1d_param[vmentry_l1d_flush].option);
}

static const struct kernel_param_ops vmentry_l1d_flush_ops = {
	.set = vmentry_l1d_flush_set,
	.get = vmentry_l1d_flush_get,
};
module_param_cb(vmentry_l1d_flush, &vmentry_l1d_flush_ops, &vmentry_l1d_flush, S_IRUGO);

struct kvm_vmx {
	struct kvm kvm;

	unsigned int tss_addr;
	.enable_smi_window = enable_smi_window,
};

static void __init vmx_setup_l1d_flush(void)
{
	if (vmentry_l1d_flush == VMENTER_L1D_FLUSH_NEVER ||
	    !boot_cpu_has_bug(X86_BUG_L1TF))
		return;

	static_branch_enable(&vmx_l1d_should_flush);
}

static int __init vmx_init(void)
{
	int r;

	}
#endif

	vmx_setup_l1d_flush();

	r = kvm_init(&vmx_x86_ops, sizeof(struct vcpu_vmx),
                     __alignof__(struct vcpu_vmx), THIS_MODULE);
	if (r)
		return r;