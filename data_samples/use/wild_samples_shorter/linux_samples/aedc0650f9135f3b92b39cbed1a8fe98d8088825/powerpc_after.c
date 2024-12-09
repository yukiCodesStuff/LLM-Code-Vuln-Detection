#include <asm/hvcall.h>
#include <asm/plpar_wrappers.h>
#endif
#include <asm/ultravisor.h>
#include <asm/kvm_host.h>

#include "timing.h"
#include "irq.h"
#include "../mm/mmu_decl.h"
			r = -EFAULT;
		break;
	}
	case KVM_PPC_SVM_OFF: {
		struct kvm *kvm = filp->private_data;

		r = 0;
		if (!kvm->arch.kvm_ops->svm_off)
			goto out;

		r = kvm->arch.kvm_ops->svm_off(kvm);
		break;
	}
	default: {
		struct kvm *kvm = filp->private_data;
		r = kvm->arch.kvm_ops->arch_vm_ioctl(filp, ioctl, arg);
	}