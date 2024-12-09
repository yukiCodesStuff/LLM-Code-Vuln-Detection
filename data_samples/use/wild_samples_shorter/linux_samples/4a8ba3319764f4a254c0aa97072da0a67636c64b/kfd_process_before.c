#include <linux/slab.h>
#include <linux/amd-iommu.h>
#include <linux/notifier.h>
struct mm_struct;

#include "kfd_priv.h"

	if (err != 0)
		goto err_process_pqm_init;

	return process;

err_process_pqm_init:
	hash_del_rcu(&process->kfd_processes);
	synchronize_rcu();
	mmu_notifier_unregister_no_release(&process->mmu_notifier, process->mm);