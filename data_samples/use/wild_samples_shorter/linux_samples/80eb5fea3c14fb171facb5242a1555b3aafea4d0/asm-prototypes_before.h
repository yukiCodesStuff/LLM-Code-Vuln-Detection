/* Patch sites */
extern s32 patch__call_flush_count_cache;
extern s32 patch__flush_count_cache_return;
extern s32 patch__memset_nocache, patch__memcpy_nocache;

extern long flush_count_cache;

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
void kvmppc_save_tm_hv(struct kvm_vcpu *vcpu, u64 msr, bool preserve_nv);
void kvmppc_restore_tm_hv(struct kvm_vcpu *vcpu, u64 msr, bool preserve_nv);