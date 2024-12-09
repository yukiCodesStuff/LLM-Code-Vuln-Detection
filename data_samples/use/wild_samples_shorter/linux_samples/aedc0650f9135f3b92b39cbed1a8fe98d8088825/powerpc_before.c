#include <asm/hvcall.h>
#include <asm/plpar_wrappers.h>
#endif

#include "timing.h"
#include "irq.h"
#include "../mm/mmu_decl.h"
			r = -EFAULT;
		break;
	}
	default: {
		struct kvm *kvm = filp->private_data;
		r = kvm->arch.kvm_ops->arch_vm_ioctl(filp, ioctl, arg);
	}