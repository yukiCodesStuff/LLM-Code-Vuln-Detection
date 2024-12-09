#include <asm/iommu.h>
#include <asm/switch_to.h>
#include <asm/xive.h>

#include "timing.h"
#include "irq.h"
#include "../mm/mmu_decl.h"
#ifdef CONFIG_KVM_XICS
	case KVM_CAP_IRQ_XICS:
#endif
		r = 1;
		break;

	case KVM_CAP_PPC_ALLOC_HTAB:
	return r;
}

long kvm_arch_vm_ioctl(struct file *filp,
                       unsigned int ioctl, unsigned long arg)
{
	struct kvm *kvm __maybe_unused = filp->private_data;
			r = -EFAULT;
		break;
	}
	default: {
		struct kvm *kvm = filp->private_data;
		r = kvm->arch.kvm_ops->arch_vm_ioctl(filp, ioctl, arg);
	}