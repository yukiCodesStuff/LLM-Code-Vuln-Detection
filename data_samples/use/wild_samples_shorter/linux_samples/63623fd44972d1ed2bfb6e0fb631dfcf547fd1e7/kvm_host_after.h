bool kvm_arch_vcpu_in_kernel(struct kvm_vcpu *vcpu);
int kvm_arch_vcpu_should_kick(struct kvm_vcpu *vcpu);
bool kvm_arch_dy_runnable(struct kvm_vcpu *vcpu);
int kvm_arch_post_init_vm(struct kvm *kvm);
void kvm_arch_pre_destroy_vm(struct kvm *kvm);

#ifndef __KVM_HAVE_ARCH_VM_ALLOC
/*
 * All architectures that want to use vzalloc currently also