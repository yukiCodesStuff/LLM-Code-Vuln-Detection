#include <asm/iommu.h>
#include <asm/switch_to.h>
#include <asm/xive.h>
#ifdef CONFIG_PPC_PSERIES
#include <asm/hvcall.h>
#include <asm/plpar_wrappers.h>
#endif

#include "timing.h"
#include "irq.h"
#include "../mm/mmu_decl.h"
#ifdef CONFIG_KVM_XICS
	case KVM_CAP_IRQ_XICS:
#endif
	case KVM_CAP_PPC_GET_CPU_CHAR:
		r = 1;
		break;

	case KVM_CAP_PPC_ALLOC_HTAB:
	return r;
}

#ifdef CONFIG_PPC_BOOK3S_64
/*
 * These functions check whether the underlying hardware is safe
 * against attacks based on observing the effects of speculatively
 * executed instructions, and whether it supplies instructions for
 * use in workarounds.  The information comes from firmware, either
 * via the device tree on powernv platforms or from an hcall on
 * pseries platforms.
 */
#ifdef CONFIG_PPC_PSERIES
static int pseries_get_cpu_char(struct kvm_ppc_cpu_char *cp)
{
	struct h_cpu_char_result c;
	unsigned long rc;

	if (!machine_is(pseries))
		return -ENOTTY;

	rc = plpar_get_cpu_characteristics(&c);
	if (rc == H_SUCCESS) {
		cp->character = c.character;
		cp->behaviour = c.behaviour;
		cp->character_mask = KVM_PPC_CPU_CHAR_SPEC_BAR_ORI31 |
			KVM_PPC_CPU_CHAR_BCCTRL_SERIALISED |
			KVM_PPC_CPU_CHAR_L1D_FLUSH_ORI30 |
			KVM_PPC_CPU_CHAR_L1D_FLUSH_TRIG2 |
			KVM_PPC_CPU_CHAR_L1D_THREAD_PRIV |
			KVM_PPC_CPU_CHAR_BR_HINT_HONOURED |
			KVM_PPC_CPU_CHAR_MTTRIG_THR_RECONF |
			KVM_PPC_CPU_CHAR_COUNT_CACHE_DIS;
		cp->behaviour_mask = KVM_PPC_CPU_BEHAV_FAVOUR_SECURITY |
			KVM_PPC_CPU_BEHAV_L1D_FLUSH_PR |
			KVM_PPC_CPU_BEHAV_BNDS_CHK_SPEC_BAR;
	}
	return 0;
}
#else
static int pseries_get_cpu_char(struct kvm_ppc_cpu_char *cp)
{
	return -ENOTTY;
}
#endif

static inline bool have_fw_feat(struct device_node *fw_features,
				const char *state, const char *name)
{
	struct device_node *np;
	bool r = false;

	np = of_get_child_by_name(fw_features, name);
	if (np) {
		r = of_property_read_bool(np, state);
		of_node_put(np);
	}
	return r;
}

static int kvmppc_get_cpu_char(struct kvm_ppc_cpu_char *cp)
{
	struct device_node *np, *fw_features;
	int r;

	memset(cp, 0, sizeof(*cp));
	r = pseries_get_cpu_char(cp);
	if (r != -ENOTTY)
		return r;

	np = of_find_node_by_name(NULL, "ibm,opal");
	if (np) {
		fw_features = of_get_child_by_name(np, "fw-features");
		of_node_put(np);
		if (!fw_features)
			return 0;
		if (have_fw_feat(fw_features, "enabled",
				 "inst-spec-barrier-ori31,31,0"))
			cp->character |= KVM_PPC_CPU_CHAR_SPEC_BAR_ORI31;
		if (have_fw_feat(fw_features, "enabled",
				 "fw-bcctrl-serialized"))
			cp->character |= KVM_PPC_CPU_CHAR_BCCTRL_SERIALISED;
		if (have_fw_feat(fw_features, "enabled",
				 "inst-l1d-flush-ori30,30,0"))
			cp->character |= KVM_PPC_CPU_CHAR_L1D_FLUSH_ORI30;
		if (have_fw_feat(fw_features, "enabled",
				 "inst-l1d-flush-trig2"))
			cp->character |= KVM_PPC_CPU_CHAR_L1D_FLUSH_TRIG2;
		if (have_fw_feat(fw_features, "enabled",
				 "fw-l1d-thread-split"))
			cp->character |= KVM_PPC_CPU_CHAR_L1D_THREAD_PRIV;
		if (have_fw_feat(fw_features, "enabled",
				 "fw-count-cache-disabled"))
			cp->character |= KVM_PPC_CPU_CHAR_COUNT_CACHE_DIS;
		cp->character_mask = KVM_PPC_CPU_CHAR_SPEC_BAR_ORI31 |
			KVM_PPC_CPU_CHAR_BCCTRL_SERIALISED |
			KVM_PPC_CPU_CHAR_L1D_FLUSH_ORI30 |
			KVM_PPC_CPU_CHAR_L1D_FLUSH_TRIG2 |
			KVM_PPC_CPU_CHAR_L1D_THREAD_PRIV |
			KVM_PPC_CPU_CHAR_COUNT_CACHE_DIS;

		if (have_fw_feat(fw_features, "enabled",
				 "speculation-policy-favor-security"))
			cp->behaviour |= KVM_PPC_CPU_BEHAV_FAVOUR_SECURITY;
		if (!have_fw_feat(fw_features, "disabled",
				  "needs-l1d-flush-msr-pr-0-to-1"))
			cp->behaviour |= KVM_PPC_CPU_BEHAV_L1D_FLUSH_PR;
		if (!have_fw_feat(fw_features, "disabled",
				  "needs-spec-barrier-for-bound-checks"))
			cp->behaviour |= KVM_PPC_CPU_BEHAV_BNDS_CHK_SPEC_BAR;
		cp->behaviour_mask = KVM_PPC_CPU_BEHAV_FAVOUR_SECURITY |
			KVM_PPC_CPU_BEHAV_L1D_FLUSH_PR |
			KVM_PPC_CPU_BEHAV_BNDS_CHK_SPEC_BAR;

		of_node_put(fw_features);
	}

	return 0;
}
#endif

long kvm_arch_vm_ioctl(struct file *filp,
                       unsigned int ioctl, unsigned long arg)
{
	struct kvm *kvm __maybe_unused = filp->private_data;
			r = -EFAULT;
		break;
	}
	case KVM_PPC_GET_CPU_CHAR: {
		struct kvm_ppc_cpu_char cpuchar;

		r = kvmppc_get_cpu_char(&cpuchar);
		if (r >= 0 && copy_to_user(argp, &cpuchar, sizeof(cpuchar)))
			r = -EFAULT;
		break;
	}
	default: {
		struct kvm *kvm = filp->private_data;
		r = kvm->arch.kvm_ops->arch_vm_ioctl(filp, ioctl, arg);
	}