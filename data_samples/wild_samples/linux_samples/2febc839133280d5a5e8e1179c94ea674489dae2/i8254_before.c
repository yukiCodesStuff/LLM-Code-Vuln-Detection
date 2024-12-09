{
	struct kvm_pit *pit = vcpu->kvm->arch.vpit;
	struct hrtimer *timer;

	if (!kvm_vcpu_is_bsp(vcpu) || !pit)
		return;

	timer = &pit->pit_state.timer;
	if (hrtimer_cancel(timer))
		hrtimer_start_expires(timer, HRTIMER_MODE_ABS);
}

static void destroy_pit_timer(struct kvm_pit *pit)
{
	hrtimer_cancel(&pit->pit_state.timer);
	flush_kthread_work(&pit->expired);
}

static void pit_do_work(struct kthread_work *work)
{
	struct kvm_pit *pit = container_of(work, struct kvm_pit, expired);
	struct kvm *kvm = pit->kvm;
	struct kvm_vcpu *vcpu;
	int i;
	struct kvm_kpit_state *ps = &pit->pit_state;
	int inject = 0;

	/* Try to inject pending interrupts when
	 * last one has been acked.
	 */
	spin_lock(&ps->inject_lock);
	if (ps->irq_ack) {
		ps->irq_ack = 0;
		inject = 1;
	}