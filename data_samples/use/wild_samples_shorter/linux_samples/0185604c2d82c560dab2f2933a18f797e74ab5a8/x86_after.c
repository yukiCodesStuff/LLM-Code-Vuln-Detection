
static int kvm_vm_ioctl_set_pit(struct kvm *kvm, struct kvm_pit_state *ps)
{
	int i;
	mutex_lock(&kvm->arch.vpit->pit_state.lock);
	memcpy(&kvm->arch.vpit->pit_state, ps, sizeof(struct kvm_pit_state));
	for (i = 0; i < 3; i++)
		kvm_pit_load_count(kvm, i, ps->channels[i].count, 0);
	mutex_unlock(&kvm->arch.vpit->pit_state.lock);
	return 0;
}

static int kvm_vm_ioctl_set_pit2(struct kvm *kvm, struct kvm_pit_state2 *ps)
{
	int start = 0;
	int i;
	u32 prev_legacy, cur_legacy;
	mutex_lock(&kvm->arch.vpit->pit_state.lock);
	prev_legacy = kvm->arch.vpit->pit_state.flags & KVM_PIT_FLAGS_HPET_LEGACY;
	cur_legacy = ps->flags & KVM_PIT_FLAGS_HPET_LEGACY;
	memcpy(&kvm->arch.vpit->pit_state.channels, &ps->channels,
	       sizeof(kvm->arch.vpit->pit_state.channels));
	kvm->arch.vpit->pit_state.flags = ps->flags;
	for (i = 0; i < 3; i++)
		kvm_pit_load_count(kvm, i, kvm->arch.vpit->pit_state.channels[i].count, start);
	mutex_unlock(&kvm->arch.vpit->pit_state.lock);
	return 0;
}
