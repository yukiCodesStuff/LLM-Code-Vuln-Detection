
#ifdef CONFIG_HAVE_KVM_IRQFD


static void
irqfd_inject(struct work_struct *work)
{

	list_del_init(&irqfd->list);

	schedule_work(&irqfd->shutdown);
}

int __attribute__((weak)) kvm_arch_set_irq_inatomic(
				struct kvm_kernel_irq_routing_entry *irq,
	 * so that we guarantee there will not be any more interrupts on this
	 * gsi once this deassign function returns.
	 */
	flush_work(&irqfd->shutdown);

	return 0;
}

	 * Block until we know all outstanding shutdown jobs have completed
	 * since we do not take a kvm* reference.
	 */
	flush_work(&irqfd->shutdown);

}

/*
	spin_unlock_irq(&kvm->irqfds.lock);
}

void kvm_irqfd_exit(void)
{
}
#endif

/*