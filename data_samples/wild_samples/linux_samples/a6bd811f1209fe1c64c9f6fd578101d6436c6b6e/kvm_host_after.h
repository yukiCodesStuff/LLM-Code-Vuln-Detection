	struct kvm_queued_interrupt {
		bool injected;
		bool soft;
		u8 nr;
	} interrupt;

	int halt_request; /* real mode on Intel only */

	int cpuid_nent;
	struct kvm_cpuid_entry2 cpuid_entries[KVM_MAX_CPUID_ENTRIES];

	int maxphyaddr;

	/* emulate context */

	struct x86_emulate_ctxt emulate_ctxt;
	bool emulate_regs_need_sync_to_vcpu;
	bool emulate_regs_need_sync_from_vcpu;
	int (*complete_userspace_io)(struct kvm_vcpu *vcpu);

	gpa_t time;
	struct pvclock_vcpu_time_info hv_clock;
	unsigned int hw_tsc_khz;
	struct gfn_to_hva_cache pv_time;
	bool pv_time_enabled;
	/* set guest stopped flag in pvclock flags field */
	bool pvclock_set_guest_stopped_request;

	struct {
		u8 preempted;
		u64 msr_val;
		u64 last_steal;
		struct gfn_to_pfn_cache cache;
	} st;

	u64 tsc_offset;
	u64 last_guest_tsc;
	u64 last_host_tsc;
	u64 tsc_offset_adjustment;
	u64 this_tsc_nsec;
	u64 this_tsc_write;
	u64 this_tsc_generation;
	bool tsc_catchup;
	bool tsc_always_catchup;
	s8 virtual_tsc_shift;
	u32 virtual_tsc_mult;
	u32 virtual_tsc_khz;
	s64 ia32_tsc_adjust_msr;
	u64 msr_ia32_power_ctl;
	u64 tsc_scaling_ratio;

	atomic_t nmi_queued;  /* unprocessed asynchronous NMIs */
	unsigned nmi_pending; /* NMI queued after currently running handler */
	bool nmi_injected;    /* Trying to inject an NMI this entry */
	bool smi_pending;    /* SMI queued after currently running handler */

	struct kvm_mtrr mtrr_state;
	u64 pat;

	unsigned switch_db_regs;
	unsigned long db[KVM_NR_DB_REGS];
	unsigned long dr6;
	unsigned long dr7;
	unsigned long eff_db[KVM_NR_DB_REGS];
	unsigned long guest_debug_dr7;
	u64 msr_platform_info;
	u64 msr_misc_features_enables;

	u64 mcg_cap;
	u64 mcg_status;
	u64 mcg_ctl;
	u64 mcg_ext_ctl;
	u64 *mce_banks;

	/* Cache MMIO info */
	u64 mmio_gva;
	unsigned mmio_access;
	gfn_t mmio_gfn;
	u64 mmio_gen;

	struct kvm_pmu pmu;

	/* used for guest single stepping over the given code position */
	unsigned long singlestep_rip;

	struct kvm_vcpu_hv hyperv;

	cpumask_var_t wbinvd_dirty_mask;

	unsigned long last_retry_eip;
	unsigned long last_retry_addr;

	struct {
		bool halted;
		gfn_t gfns[roundup_pow_of_two(ASYNC_PF_PER_VCPU)];
		struct gfn_to_hva_cache data;
		u64 msr_val;
		u32 id;
		bool send_user_only;
		u32 host_apf_reason;
		unsigned long nested_apf_token;
		bool delivery_as_pf_vmexit;
	} apf;

	/* OSVW MSRs (AMD only) */
	struct {
		u64 length;
		u64 status;
	} osvw;

	struct {
		u64 msr_val;
		struct gfn_to_hva_cache data;
	} pv_eoi;

	u64 msr_kvm_poll_control;

	/*
	 * Indicate whether the access faults on its page table in guest
	 * which is set when fix page fault and used to detect unhandeable
	 * instruction.
	 */
	bool write_fault_to_shadow_pgtable;

	/* set at EPT violation at this point */
	unsigned long exit_qualification;

	/* pv related host specific info */
	struct {
		bool pv_unhalted;
	} pv;

	int pending_ioapic_eoi;
	int pending_external_vector;

	/* GPA available */
	bool gpa_available;
	gpa_t gpa_val;

	/* be preempted when it's in kernel-mode(cpl=0) */
	bool preempted_in_kernel;

	/* Flush the L1 Data cache for L1TF mitigation on VMENTER */
	bool l1tf_flush_l1d;

	/* AMD MSRC001_0015 Hardware Configuration */
	u64 msr_hwcr;
};

struct kvm_lpage_info {
	int disallow_lpage;
};

struct kvm_arch_memory_slot {
	struct kvm_rmap_head *rmap[KVM_NR_PAGE_SIZES];
	struct kvm_lpage_info *lpage_info[KVM_NR_PAGE_SIZES - 1];
	unsigned short *gfn_track[KVM_PAGE_TRACK_MAX];
};

/*
 * We use as the mode the number of bits allocated in the LDR for the
 * logical processor ID.  It happens that these are all powers of two.
 * This makes it is very easy to detect cases where the APICs are
 * configured for multiple modes; in that case, we cannot use the map and
 * hence cannot use kvm_irq_delivery_to_apic_fast either.
 */
#define KVM_APIC_MODE_XAPIC_CLUSTER          4
#define KVM_APIC_MODE_XAPIC_FLAT             8
#define KVM_APIC_MODE_X2APIC                16

struct kvm_apic_map {
	struct rcu_head rcu;
	u8 mode;
	u32 max_apic_id;
	union {
		struct kvm_lapic *xapic_flat_map[8];
		struct kvm_lapic *xapic_cluster_map[16][4];
	};
	struct kvm_lapic *phys_map[];
};

/* Hyper-V emulation context */
struct kvm_hv {
	struct mutex hv_lock;
	u64 hv_guest_os_id;
	u64 hv_hypercall;
	u64 hv_tsc_page;

	/* Hyper-v based guest crash (NT kernel bugcheck) parameters */
	u64 hv_crash_param[HV_X64_MSR_CRASH_PARAMS];
	u64 hv_crash_ctl;

	HV_REFERENCE_TSC_PAGE tsc_ref;

	struct idr conn_to_evt;

	u64 hv_reenlightenment_control;
	u64 hv_tsc_emulation_control;
	u64 hv_tsc_emulation_status;

	/* How many vCPUs have VP index != vCPU index */
	atomic_t num_mismatched_vp_indexes;

	struct hv_partition_assist_pg *hv_pa_pg;
};

enum kvm_irqchip_mode {
	KVM_IRQCHIP_NONE,
	KVM_IRQCHIP_KERNEL,       /* created with KVM_CREATE_IRQCHIP */
	KVM_IRQCHIP_SPLIT,        /* created with KVM_CAP_SPLIT_IRQCHIP */
};

struct kvm_arch {
	unsigned long n_used_mmu_pages;
	unsigned long n_requested_mmu_pages;
	unsigned long n_max_mmu_pages;
	unsigned int indirect_shadow_pages;
	u8 mmu_valid_gen;
	struct hlist_head mmu_page_hash[KVM_NUM_MMU_PAGES];
	/*
	 * Hash table of struct kvm_mmu_page.
	 */
	struct list_head active_mmu_pages;
	struct list_head zapped_obsolete_pages;
	struct list_head lpage_disallowed_mmu_pages;
	struct kvm_page_track_notifier_node mmu_sp_tracker;
	struct kvm_page_track_notifier_head track_notifier_head;

	struct list_head assigned_dev_head;
	struct iommu_domain *iommu_domain;
	bool iommu_noncoherent;
#define __KVM_HAVE_ARCH_NONCOHERENT_DMA
	atomic_t noncoherent_dma_count;
#define __KVM_HAVE_ARCH_ASSIGNED_DEVICE
	atomic_t assigned_device_count;
	struct kvm_pic *vpic;
	struct kvm_ioapic *vioapic;
	struct kvm_pit *vpit;
	atomic_t vapics_in_nmi_mode;
	struct mutex apic_map_lock;
	struct kvm_apic_map *apic_map;

	bool apic_access_page_done;

	gpa_t wall_clock;

	bool mwait_in_guest;
	bool hlt_in_guest;
	bool pause_in_guest;
	bool cstate_in_guest;

	unsigned long irq_sources_bitmap;
	s64 kvmclock_offset;
	raw_spinlock_t tsc_write_lock;
	u64 last_tsc_nsec;
	u64 last_tsc_write;
	u32 last_tsc_khz;
	u64 cur_tsc_nsec;
	u64 cur_tsc_write;
	u64 cur_tsc_offset;
	u64 cur_tsc_generation;
	int nr_vcpus_matched_tsc;

	spinlock_t pvclock_gtod_sync_lock;
	bool use_master_clock;
	u64 master_kernel_ns;
	u64 master_cycle_now;
	struct delayed_work kvmclock_update_work;
	struct delayed_work kvmclock_sync_work;

	struct kvm_xen_hvm_config xen_hvm_config;

	/* reads protected by irq_srcu, writes by irq_lock */
	struct hlist_head mask_notifier_list;

	struct kvm_hv hyperv;

	#ifdef CONFIG_KVM_MMU_AUDIT
	int audit_point;
	#endif

	bool backwards_tsc_observed;
	bool boot_vcpu_runs_old_kvmclock;
	u32 bsp_vcpu_id;

	u64 disabled_quirks;

	enum kvm_irqchip_mode irqchip_mode;
	u8 nr_reserved_ioapic_pins;

	bool disabled_lapic_found;

	bool x2apic_format;
	bool x2apic_broadcast_quirk_disabled;

	bool guest_can_read_msr_platform_info;
	bool exception_payload_enabled;

	struct kvm_pmu_event_filter *pmu_event_filter;
	struct task_struct *nx_lpage_recovery_thread;
};

struct kvm_vm_stat {
	ulong mmu_shadow_zapped;
	ulong mmu_pte_write;
	ulong mmu_pte_updated;
	ulong mmu_pde_zapped;
	ulong mmu_flooded;
	ulong mmu_recycled;
	ulong mmu_cache_miss;
	ulong mmu_unsync;
	ulong remote_tlb_flush;
	ulong lpages;
	ulong nx_lpage_splits;
	ulong max_mmu_page_hash_collisions;
};

struct kvm_vcpu_stat {
	u64 pf_fixed;
	u64 pf_guest;
	u64 tlb_flush;
	u64 invlpg;

	u64 exits;
	u64 io_exits;
	u64 mmio_exits;
	u64 signal_exits;
	u64 irq_window_exits;
	u64 nmi_window_exits;
	u64 l1d_flush;
	u64 halt_exits;
	u64 halt_successful_poll;
	u64 halt_attempted_poll;
	u64 halt_poll_invalid;
	u64 halt_wakeup;
	u64 request_irq_exits;
	u64 irq_exits;
	u64 host_state_reload;
	u64 fpu_reload;
	u64 insn_emulation;
	u64 insn_emulation_fail;
	u64 hypercalls;
	u64 irq_injections;
	u64 nmi_injections;
	u64 req_event;
};

struct x86_instruction_info;

struct msr_data {
	bool host_initiated;
	u32 index;
	u64 data;
};

struct kvm_lapic_irq {
	u32 vector;
	u16 delivery_mode;
	u16 dest_mode;
	bool level;
	u16 trig_mode;
	u32 shorthand;
	u32 dest_id;
	bool msi_redir_hint;
};

struct kvm_x86_ops {
	int (*cpu_has_kvm_support)(void);          /* __init */
	int (*disabled_by_bios)(void);             /* __init */
	int (*hardware_enable)(void);
	void (*hardware_disable)(void);
	int (*check_processor_compatibility)(void);/* __init */
	int (*hardware_setup)(void);               /* __init */
	void (*hardware_unsetup)(void);            /* __exit */
	bool (*cpu_has_accelerated_tpr)(void);
	bool (*has_emulated_msr)(int index);
	void (*cpuid_update)(struct kvm_vcpu *vcpu);

	struct kvm *(*vm_alloc)(void);
	void (*vm_free)(struct kvm *);
	int (*vm_init)(struct kvm *kvm);
	void (*vm_destroy)(struct kvm *kvm);

	/* Create, but do not attach this VCPU */
	struct kvm_vcpu *(*vcpu_create)(struct kvm *kvm, unsigned id);
	void (*vcpu_free)(struct kvm_vcpu *vcpu);
	void (*vcpu_reset)(struct kvm_vcpu *vcpu, bool init_event);

	void (*prepare_guest_switch)(struct kvm_vcpu *vcpu);
	void (*vcpu_load)(struct kvm_vcpu *vcpu, int cpu);
	void (*vcpu_put)(struct kvm_vcpu *vcpu);

	void (*update_bp_intercept)(struct kvm_vcpu *vcpu);
	int (*get_msr)(struct kvm_vcpu *vcpu, struct msr_data *msr);
	int (*set_msr)(struct kvm_vcpu *vcpu, struct msr_data *msr);
	u64 (*get_segment_base)(struct kvm_vcpu *vcpu, int seg);
	void (*get_segment)(struct kvm_vcpu *vcpu,
			    struct kvm_segment *var, int seg);
	int (*get_cpl)(struct kvm_vcpu *vcpu);
	void (*set_segment)(struct kvm_vcpu *vcpu,
			    struct kvm_segment *var, int seg);
	void (*get_cs_db_l_bits)(struct kvm_vcpu *vcpu, int *db, int *l);
	void (*decache_cr0_guest_bits)(struct kvm_vcpu *vcpu);
	void (*decache_cr4_guest_bits)(struct kvm_vcpu *vcpu);
	void (*set_cr0)(struct kvm_vcpu *vcpu, unsigned long cr0);
	void (*set_cr3)(struct kvm_vcpu *vcpu, unsigned long cr3);
	int (*set_cr4)(struct kvm_vcpu *vcpu, unsigned long cr4);
	void (*set_efer)(struct kvm_vcpu *vcpu, u64 efer);
	void (*get_idt)(struct kvm_vcpu *vcpu, struct desc_ptr *dt);
	void (*set_idt)(struct kvm_vcpu *vcpu, struct desc_ptr *dt);
	void (*get_gdt)(struct kvm_vcpu *vcpu, struct desc_ptr *dt);
	void (*set_gdt)(struct kvm_vcpu *vcpu, struct desc_ptr *dt);
	u64 (*get_dr6)(struct kvm_vcpu *vcpu);
	void (*set_dr6)(struct kvm_vcpu *vcpu, unsigned long value);
	void (*sync_dirty_debug_regs)(struct kvm_vcpu *vcpu);
	void (*set_dr7)(struct kvm_vcpu *vcpu, unsigned long value);
	void (*cache_reg)(struct kvm_vcpu *vcpu, enum kvm_reg reg);
	unsigned long (*get_rflags)(struct kvm_vcpu *vcpu);
	void (*set_rflags)(struct kvm_vcpu *vcpu, unsigned long rflags);

	void (*tlb_flush)(struct kvm_vcpu *vcpu, bool invalidate_gpa);
	int  (*tlb_remote_flush)(struct kvm *kvm);
	int  (*tlb_remote_flush_with_range)(struct kvm *kvm,
			struct kvm_tlb_range *range);

	/*
	 * Flush any TLB entries associated with the given GVA.
	 * Does not need to flush GPA->HPA mappings.
	 * Can potentially get non-canonical addresses through INVLPGs, which
	 * the implementation may choose to ignore if appropriate.
	 */
	void (*tlb_flush_gva)(struct kvm_vcpu *vcpu, gva_t addr);

	void (*run)(struct kvm_vcpu *vcpu);
	int (*handle_exit)(struct kvm_vcpu *vcpu);
	int (*skip_emulated_instruction)(struct kvm_vcpu *vcpu);
	void (*set_interrupt_shadow)(struct kvm_vcpu *vcpu, int mask);
	u32 (*get_interrupt_shadow)(struct kvm_vcpu *vcpu);
	void (*patch_hypercall)(struct kvm_vcpu *vcpu,
				unsigned char *hypercall_addr);
	void (*set_irq)(struct kvm_vcpu *vcpu);
	void (*set_nmi)(struct kvm_vcpu *vcpu);
	void (*queue_exception)(struct kvm_vcpu *vcpu);
	void (*cancel_injection)(struct kvm_vcpu *vcpu);
	int (*interrupt_allowed)(struct kvm_vcpu *vcpu);
	int (*nmi_allowed)(struct kvm_vcpu *vcpu);
	bool (*get_nmi_mask)(struct kvm_vcpu *vcpu);
	void (*set_nmi_mask)(struct kvm_vcpu *vcpu, bool masked);
	void (*enable_nmi_window)(struct kvm_vcpu *vcpu);
	void (*enable_irq_window)(struct kvm_vcpu *vcpu);
	void (*update_cr8_intercept)(struct kvm_vcpu *vcpu, int tpr, int irr);
	bool (*get_enable_apicv)(struct kvm *kvm);
	void (*refresh_apicv_exec_ctrl)(struct kvm_vcpu *vcpu);
	void (*hwapic_irr_update)(struct kvm_vcpu *vcpu, int max_irr);
	void (*hwapic_isr_update)(struct kvm_vcpu *vcpu, int isr);
	bool (*guest_apic_has_interrupt)(struct kvm_vcpu *vcpu);
	void (*load_eoi_exitmap)(struct kvm_vcpu *vcpu, u64 *eoi_exit_bitmap);
	void (*set_virtual_apic_mode)(struct kvm_vcpu *vcpu);
	void (*set_apic_access_page_addr)(struct kvm_vcpu *vcpu, hpa_t hpa);
	void (*deliver_posted_interrupt)(struct kvm_vcpu *vcpu, int vector);
	int (*sync_pir_to_irr)(struct kvm_vcpu *vcpu);
	int (*set_tss_addr)(struct kvm *kvm, unsigned int addr);
	int (*set_identity_map_addr)(struct kvm *kvm, u64 ident_addr);
	int (*get_tdp_level)(struct kvm_vcpu *vcpu);
	u64 (*get_mt_mask)(struct kvm_vcpu *vcpu, gfn_t gfn, bool is_mmio);
	int (*get_lpage_level)(void);
	bool (*rdtscp_supported)(void);
	bool (*invpcid_supported)(void);

	void (*set_tdp_cr3)(struct kvm_vcpu *vcpu, unsigned long cr3);

	void (*set_supported_cpuid)(u32 func, struct kvm_cpuid_entry2 *entry);

	bool (*has_wbinvd_exit)(void);

	u64 (*read_l1_tsc_offset)(struct kvm_vcpu *vcpu);
	/* Returns actual tsc_offset set in active VMCS */
	u64 (*write_l1_tsc_offset)(struct kvm_vcpu *vcpu, u64 offset);

	void (*get_exit_info)(struct kvm_vcpu *vcpu, u64 *info1, u64 *info2);

	int (*check_intercept)(struct kvm_vcpu *vcpu,
			       struct x86_instruction_info *info,
			       enum x86_intercept_stage stage);
	void (*handle_exit_irqoff)(struct kvm_vcpu *vcpu);
	bool (*mpx_supported)(void);
	bool (*xsaves_supported)(void);
	bool (*umip_emulated)(void);
	bool (*pt_supported)(void);

	int (*check_nested_events)(struct kvm_vcpu *vcpu, bool external_intr);
	void (*request_immediate_exit)(struct kvm_vcpu *vcpu);

	void (*sched_in)(struct kvm_vcpu *kvm, int cpu);

	/*
	 * Arch-specific dirty logging hooks. These hooks are only supposed to
	 * be valid if the specific arch has hardware-accelerated dirty logging
	 * mechanism. Currently only for PML on VMX.
	 *
	 *  - slot_enable_log_dirty:
	 *	called when enabling log dirty mode for the slot.
	 *  - slot_disable_log_dirty:
	 *	called when disabling log dirty mode for the slot.
	 *	also called when slot is created with log dirty disabled.
	 *  - flush_log_dirty:
	 *	called before reporting dirty_bitmap to userspace.
	 *  - enable_log_dirty_pt_masked:
	 *	called when reenabling log dirty for the GFNs in the mask after
	 *	corresponding bits are cleared in slot->dirty_bitmap.
	 */
	void (*slot_enable_log_dirty)(struct kvm *kvm,
				      struct kvm_memory_slot *slot);
	void (*slot_disable_log_dirty)(struct kvm *kvm,
				       struct kvm_memory_slot *slot);
	void (*flush_log_dirty)(struct kvm *kvm);
	void (*enable_log_dirty_pt_masked)(struct kvm *kvm,
					   struct kvm_memory_slot *slot,
					   gfn_t offset, unsigned long mask);
	int (*write_log_dirty)(struct kvm_vcpu *vcpu);

	/* pmu operations of sub-arch */
	const struct kvm_pmu_ops *pmu_ops;

	/*
	 * Architecture specific hooks for vCPU blocking due to
	 * HLT instruction.
	 * Returns for .pre_block():
	 *    - 0 means continue to block the vCPU.
	 *    - 1 means we cannot block the vCPU since some event
	 *        happens during this period, such as, 'ON' bit in
	 *        posted-interrupts descriptor is set.
	 */
	int (*pre_block)(struct kvm_vcpu *vcpu);
	void (*post_block)(struct kvm_vcpu *vcpu);

	void (*vcpu_blocking)(struct kvm_vcpu *vcpu);
	void (*vcpu_unblocking)(struct kvm_vcpu *vcpu);

	int (*update_pi_irte)(struct kvm *kvm, unsigned int host_irq,
			      uint32_t guest_irq, bool set);
	void (*apicv_post_state_restore)(struct kvm_vcpu *vcpu);
	bool (*dy_apicv_has_pending_interrupt)(struct kvm_vcpu *vcpu);

	int (*set_hv_timer)(struct kvm_vcpu *vcpu, u64 guest_deadline_tsc,
			    bool *expired);
	void (*cancel_hv_timer)(struct kvm_vcpu *vcpu);

	void (*setup_mce)(struct kvm_vcpu *vcpu);

	int (*get_nested_state)(struct kvm_vcpu *vcpu,
				struct kvm_nested_state __user *user_kvm_nested_state,
				unsigned user_data_size);
	int (*set_nested_state)(struct kvm_vcpu *vcpu,
				struct kvm_nested_state __user *user_kvm_nested_state,
				struct kvm_nested_state *kvm_state);
	bool (*get_vmcs12_pages)(struct kvm_vcpu *vcpu);

	int (*smi_allowed)(struct kvm_vcpu *vcpu);
	int (*pre_enter_smm)(struct kvm_vcpu *vcpu, char *smstate);
	int (*pre_leave_smm)(struct kvm_vcpu *vcpu, const char *smstate);
	int (*enable_smi_window)(struct kvm_vcpu *vcpu);

	int (*mem_enc_op)(struct kvm *kvm, void __user *argp);
	int (*mem_enc_reg_region)(struct kvm *kvm, struct kvm_enc_region *argp);
	int (*mem_enc_unreg_region)(struct kvm *kvm, struct kvm_enc_region *argp);

	int (*get_msr_feature)(struct kvm_msr_entry *entry);

	int (*nested_enable_evmcs)(struct kvm_vcpu *vcpu,
				   uint16_t *vmcs_version);
	uint16_t (*nested_get_evmcs_version)(struct kvm_vcpu *vcpu);

	bool (*need_emulation_on_page_fault)(struct kvm_vcpu *vcpu);

	bool (*apic_init_signal_blocked)(struct kvm_vcpu *vcpu);
	int (*enable_direct_tlbflush)(struct kvm_vcpu *vcpu);
};

struct kvm_arch_async_pf {
	u32 token;
	gfn_t gfn;
	unsigned long cr3;
	bool direct_map;
};

extern struct kvm_x86_ops *kvm_x86_ops;
extern struct kmem_cache *x86_fpu_cache;

#define __KVM_HAVE_ARCH_VM_ALLOC
static inline struct kvm *kvm_arch_alloc_vm(void)
{
	return kvm_x86_ops->vm_alloc();
}

static inline void kvm_arch_free_vm(struct kvm *kvm)
{
	return kvm_x86_ops->vm_free(kvm);
}

#define __KVM_HAVE_ARCH_FLUSH_REMOTE_TLB
static inline int kvm_arch_flush_remote_tlb(struct kvm *kvm)
{
	if (kvm_x86_ops->tlb_remote_flush &&
	    !kvm_x86_ops->tlb_remote_flush(kvm))
		return 0;
	else
		return -ENOTSUPP;
}

int kvm_mmu_module_init(void);
void kvm_mmu_module_exit(void);

void kvm_mmu_destroy(struct kvm_vcpu *vcpu);
int kvm_mmu_create(struct kvm_vcpu *vcpu);
void kvm_mmu_init_vm(struct kvm *kvm);
void kvm_mmu_uninit_vm(struct kvm *kvm);
void kvm_mmu_set_mask_ptes(u64 user_mask, u64 accessed_mask,
		u64 dirty_mask, u64 nx_mask, u64 x_mask, u64 p_mask,
		u64 acc_track_mask, u64 me_mask);

void kvm_mmu_reset_context(struct kvm_vcpu *vcpu);
void kvm_mmu_slot_remove_write_access(struct kvm *kvm,
				      struct kvm_memory_slot *memslot);
void kvm_mmu_zap_collapsible_sptes(struct kvm *kvm,
				   const struct kvm_memory_slot *memslot);
void kvm_mmu_slot_leaf_clear_dirty(struct kvm *kvm,
				   struct kvm_memory_slot *memslot);
void kvm_mmu_slot_largepage_remove_write_access(struct kvm *kvm,
					struct kvm_memory_slot *memslot);
void kvm_mmu_slot_set_dirty(struct kvm *kvm,
			    struct kvm_memory_slot *memslot);
void kvm_mmu_clear_dirty_pt_masked(struct kvm *kvm,
				   struct kvm_memory_slot *slot,
				   gfn_t gfn_offset, unsigned long mask);
void kvm_mmu_zap_all(struct kvm *kvm);
void kvm_mmu_invalidate_mmio_sptes(struct kvm *kvm, u64 gen);
unsigned long kvm_mmu_calculate_default_mmu_pages(struct kvm *kvm);
void kvm_mmu_change_mmu_pages(struct kvm *kvm, unsigned long kvm_nr_mmu_pages);

int load_pdptrs(struct kvm_vcpu *vcpu, struct kvm_mmu *mmu, unsigned long cr3);
bool pdptrs_changed(struct kvm_vcpu *vcpu);

int emulator_write_phys(struct kvm_vcpu *vcpu, gpa_t gpa,
			  const void *val, int bytes);

struct kvm_irq_mask_notifier {
	void (*func)(struct kvm_irq_mask_notifier *kimn, bool masked);
	int irq;
	struct hlist_node link;
};

void kvm_register_irq_mask_notifier(struct kvm *kvm, int irq,
				    struct kvm_irq_mask_notifier *kimn);
void kvm_unregister_irq_mask_notifier(struct kvm *kvm, int irq,
				      struct kvm_irq_mask_notifier *kimn);
void kvm_fire_mask_notifiers(struct kvm *kvm, unsigned irqchip, unsigned pin,
			     bool mask);

extern bool tdp_enabled;

u64 vcpu_tsc_khz(struct kvm_vcpu *vcpu);

/* control of guest tsc rate supported? */
extern bool kvm_has_tsc_control;
/* maximum supported tsc_khz for guests */
extern u32  kvm_max_guest_tsc_khz;
/* number of bits of the fractional part of the TSC scaling ratio */
extern u8   kvm_tsc_scaling_ratio_frac_bits;
/* maximum allowed value of TSC scaling ratio */
extern u64  kvm_max_tsc_scaling_ratio;
/* 1ull << kvm_tsc_scaling_ratio_frac_bits */
extern u64  kvm_default_tsc_scaling_ratio;

extern u64 kvm_mce_cap_supported;

/*
 * EMULTYPE_NO_DECODE - Set when re-emulating an instruction (after completing
 *			userspace I/O) to indicate that the emulation context
 *			should be resued as is, i.e. skip initialization of
 *			emulation context, instruction fetch and decode.
 *
 * EMULTYPE_TRAP_UD - Set when emulating an intercepted #UD from hardware.
 *		      Indicates that only select instructions (tagged with
 *		      EmulateOnUD) should be emulated (to minimize the emulator
 *		      attack surface).  See also EMULTYPE_TRAP_UD_FORCED.
 *
 * EMULTYPE_SKIP - Set when emulating solely to skip an instruction, i.e. to
 *		   decode the instruction length.  For use *only* by
 *		   kvm_x86_ops->skip_emulated_instruction() implementations.
 *
 * EMULTYPE_ALLOW_RETRY - Set when the emulator should resume the guest to
 *			  retry native execution under certain conditions.
 *
 * EMULTYPE_TRAP_UD_FORCED - Set when emulating an intercepted #UD that was
 *			     triggered by KVM's magic "force emulation" prefix,
 *			     which is opt in via module param (off by default).
 *			     Bypasses EmulateOnUD restriction despite emulating
 *			     due to an intercepted #UD (see EMULTYPE_TRAP_UD).
 *			     Used to test the full emulator from userspace.
 *
 * EMULTYPE_VMWARE_GP - Set when emulating an intercepted #GP for VMware
 *			backdoor emulation, which is opt in via module param.
 *			VMware backoor emulation handles select instructions
 *			and reinjects the #GP for all other cases.
 */
#define EMULTYPE_NO_DECODE	    (1 << 0)
#define EMULTYPE_TRAP_UD	    (1 << 1)
#define EMULTYPE_SKIP		    (1 << 2)
#define EMULTYPE_ALLOW_RETRY	    (1 << 3)
#define EMULTYPE_TRAP_UD_FORCED	    (1 << 4)
#define EMULTYPE_VMWARE_GP	    (1 << 5)
int kvm_emulate_instruction(struct kvm_vcpu *vcpu, int emulation_type);
int kvm_emulate_instruction_from_buffer(struct kvm_vcpu *vcpu,
					void *insn, int insn_len);

void kvm_enable_efer_bits(u64);
bool kvm_valid_efer(struct kvm_vcpu *vcpu, u64 efer);
int __kvm_get_msr(struct kvm_vcpu *vcpu, u32 index, u64 *data, bool host_initiated);
int kvm_get_msr(struct kvm_vcpu *vcpu, u32 index, u64 *data);
int kvm_set_msr(struct kvm_vcpu *vcpu, u32 index, u64 data);
int kvm_emulate_rdmsr(struct kvm_vcpu *vcpu);
int kvm_emulate_wrmsr(struct kvm_vcpu *vcpu);

struct x86_emulate_ctxt;

int kvm_fast_pio(struct kvm_vcpu *vcpu, int size, unsigned short port, int in);
int kvm_emulate_cpuid(struct kvm_vcpu *vcpu);
int kvm_emulate_halt(struct kvm_vcpu *vcpu);
int kvm_vcpu_halt(struct kvm_vcpu *vcpu);
int kvm_emulate_wbinvd(struct kvm_vcpu *vcpu);

void kvm_get_segment(struct kvm_vcpu *vcpu, struct kvm_segment *var, int seg);
int kvm_load_segment_descriptor(struct kvm_vcpu *vcpu, u16 selector, int seg);
void kvm_vcpu_deliver_sipi_vector(struct kvm_vcpu *vcpu, u8 vector);

int kvm_task_switch(struct kvm_vcpu *vcpu, u16 tss_selector, int idt_index,
		    int reason, bool has_error_code, u32 error_code);

int kvm_set_cr0(struct kvm_vcpu *vcpu, unsigned long cr0);
int kvm_set_cr3(struct kvm_vcpu *vcpu, unsigned long cr3);
int kvm_set_cr4(struct kvm_vcpu *vcpu, unsigned long cr4);
int kvm_set_cr8(struct kvm_vcpu *vcpu, unsigned long cr8);
int kvm_set_dr(struct kvm_vcpu *vcpu, int dr, unsigned long val);
int kvm_get_dr(struct kvm_vcpu *vcpu, int dr, unsigned long *val);
unsigned long kvm_get_cr8(struct kvm_vcpu *vcpu);
void kvm_lmsw(struct kvm_vcpu *vcpu, unsigned long msw);
void kvm_get_cs_db_l_bits(struct kvm_vcpu *vcpu, int *db, int *l);
int kvm_set_xcr(struct kvm_vcpu *vcpu, u32 index, u64 xcr);

int kvm_get_msr_common(struct kvm_vcpu *vcpu, struct msr_data *msr);
int kvm_set_msr_common(struct kvm_vcpu *vcpu, struct msr_data *msr);

unsigned long kvm_get_rflags(struct kvm_vcpu *vcpu);
void kvm_set_rflags(struct kvm_vcpu *vcpu, unsigned long rflags);
bool kvm_rdpmc(struct kvm_vcpu *vcpu);

void kvm_queue_exception(struct kvm_vcpu *vcpu, unsigned nr);
void kvm_queue_exception_e(struct kvm_vcpu *vcpu, unsigned nr, u32 error_code);
void kvm_requeue_exception(struct kvm_vcpu *vcpu, unsigned nr);
void kvm_requeue_exception_e(struct kvm_vcpu *vcpu, unsigned nr, u32 error_code);
void kvm_inject_page_fault(struct kvm_vcpu *vcpu, struct x86_exception *fault);
int kvm_read_guest_page_mmu(struct kvm_vcpu *vcpu, struct kvm_mmu *mmu,
			    gfn_t gfn, void *data, int offset, int len,
			    u32 access);
bool kvm_require_cpl(struct kvm_vcpu *vcpu, int required_cpl);
bool kvm_require_dr(struct kvm_vcpu *vcpu, int dr);

static inline int __kvm_irq_line_state(unsigned long *irq_state,
				       int irq_source_id, int level)
{
	/* Logical OR for level trig interrupt */
	if (level)
		__set_bit(irq_source_id, irq_state);
	else
		__clear_bit(irq_source_id, irq_state);

	return !!(*irq_state);
}

#define KVM_MMU_ROOT_CURRENT		BIT(0)
#define KVM_MMU_ROOT_PREVIOUS(i)	BIT(1+i)
#define KVM_MMU_ROOTS_ALL		(~0UL)

int kvm_pic_set_irq(struct kvm_pic *pic, int irq, int irq_source_id, int level);
void kvm_pic_clear_all(struct kvm_pic *pic, int irq_source_id);

void kvm_inject_nmi(struct kvm_vcpu *vcpu);

int kvm_mmu_unprotect_page(struct kvm *kvm, gfn_t gfn);
int kvm_mmu_unprotect_page_virt(struct kvm_vcpu *vcpu, gva_t gva);
void __kvm_mmu_free_some_pages(struct kvm_vcpu *vcpu);
int kvm_mmu_load(struct kvm_vcpu *vcpu);
void kvm_mmu_unload(struct kvm_vcpu *vcpu);
void kvm_mmu_sync_roots(struct kvm_vcpu *vcpu);
void kvm_mmu_free_roots(struct kvm_vcpu *vcpu, struct kvm_mmu *mmu,
			ulong roots_to_free);
gpa_t translate_nested_gpa(struct kvm_vcpu *vcpu, gpa_t gpa, u32 access,
			   struct x86_exception *exception);
gpa_t kvm_mmu_gva_to_gpa_read(struct kvm_vcpu *vcpu, gva_t gva,
			      struct x86_exception *exception);
gpa_t kvm_mmu_gva_to_gpa_fetch(struct kvm_vcpu *vcpu, gva_t gva,
			       struct x86_exception *exception);
gpa_t kvm_mmu_gva_to_gpa_write(struct kvm_vcpu *vcpu, gva_t gva,
			       struct x86_exception *exception);
gpa_t kvm_mmu_gva_to_gpa_system(struct kvm_vcpu *vcpu, gva_t gva,
				struct x86_exception *exception);

void kvm_vcpu_deactivate_apicv(struct kvm_vcpu *vcpu);

int kvm_emulate_hypercall(struct kvm_vcpu *vcpu);

int kvm_mmu_page_fault(struct kvm_vcpu *vcpu, gva_t gva, u64 error_code,
		       void *insn, int insn_len);
void kvm_mmu_invlpg(struct kvm_vcpu *vcpu, gva_t gva);
void kvm_mmu_invpcid_gva(struct kvm_vcpu *vcpu, gva_t gva, unsigned long pcid);
void kvm_mmu_new_cr3(struct kvm_vcpu *vcpu, gpa_t new_cr3, bool skip_tlb_flush);

void kvm_enable_tdp(void);
void kvm_disable_tdp(void);

static inline gpa_t translate_gpa(struct kvm_vcpu *vcpu, gpa_t gpa, u32 access,
				  struct x86_exception *exception)
{
	return gpa;
}

static inline struct kvm_mmu_page *page_header(hpa_t shadow_page)
{
	struct page *page = pfn_to_page(shadow_page >> PAGE_SHIFT);

	return (struct kvm_mmu_page *)page_private(page);
}

static inline u16 kvm_read_ldt(void)
{
	u16 ldt;
	asm("sldt %0" : "=g"(ldt));
	return ldt;
}

static inline void kvm_load_ldt(u16 sel)
{
	asm("lldt %0" : : "rm"(sel));
}

#ifdef CONFIG_X86_64
static inline unsigned long read_msr(unsigned long msr)
{
	u64 value;

	rdmsrl(msr, value);
	return value;
}
#endif

static inline u32 get_rdx_init_val(void)
{
	return 0x600; /* P6 family */
}

static inline void kvm_inject_gp(struct kvm_vcpu *vcpu, u32 error_code)
{
	kvm_queue_exception_e(vcpu, GP_VECTOR, error_code);
}

#define TSS_IOPB_BASE_OFFSET 0x66
#define TSS_BASE_SIZE 0x68
#define TSS_IOPB_SIZE (65536 / 8)
#define TSS_REDIRECTION_SIZE (256 / 8)
#define RMODE_TSS_SIZE							\
	(TSS_BASE_SIZE + TSS_REDIRECTION_SIZE + TSS_IOPB_SIZE + 1)

enum {
	TASK_SWITCH_CALL = 0,
	TASK_SWITCH_IRET = 1,
	TASK_SWITCH_JMP = 2,
	TASK_SWITCH_GATE = 3,
};

#define HF_GIF_MASK		(1 << 0)
#define HF_HIF_MASK		(1 << 1)
#define HF_VINTR_MASK		(1 << 2)
#define HF_NMI_MASK		(1 << 3)
#define HF_IRET_MASK		(1 << 4)
#define HF_GUEST_MASK		(1 << 5) /* VCPU is in guest-mode */
#define HF_SMM_MASK		(1 << 6)
#define HF_SMM_INSIDE_NMI_MASK	(1 << 7)

#define __KVM_VCPU_MULTIPLE_ADDRESS_SPACE
#define KVM_ADDRESS_SPACE_NUM 2

#define kvm_arch_vcpu_memslots_id(vcpu) ((vcpu)->arch.hflags & HF_SMM_MASK ? 1 : 0)
#define kvm_memslots_for_spte_role(kvm, role) __kvm_memslots(kvm, (role).smm)

asmlinkage void kvm_spurious_fault(void);

/*
 * Hardware virtualization extension instructions may fault if a
 * reboot turns off virtualization while processes are running.
 * Usually after catching the fault we just panic; during reboot
 * instead the instruction is ignored.
 */
#define __kvm_handle_fault_on_reboot(insn)				\
	"666: \n\t"							\
	insn "\n\t"							\
	"jmp	668f \n\t"						\
	"667: \n\t"							\
	"call	kvm_spurious_fault \n\t"				\
	"668: \n\t"							\
	_ASM_EXTABLE(666b, 667b)

#define KVM_ARCH_WANT_MMU_NOTIFIER
int kvm_unmap_hva_range(struct kvm *kvm, unsigned long start, unsigned long end);
int kvm_age_hva(struct kvm *kvm, unsigned long start, unsigned long end);
int kvm_test_age_hva(struct kvm *kvm, unsigned long hva);
int kvm_set_spte_hva(struct kvm *kvm, unsigned long hva, pte_t pte);
int kvm_cpu_has_injectable_intr(struct kvm_vcpu *v);
int kvm_cpu_has_interrupt(struct kvm_vcpu *vcpu);
int kvm_arch_interrupt_allowed(struct kvm_vcpu *vcpu);
int kvm_cpu_get_interrupt(struct kvm_vcpu *v);
void kvm_vcpu_reset(struct kvm_vcpu *vcpu, bool init_event);
void kvm_vcpu_reload_apic_access_page(struct kvm_vcpu *vcpu);

int kvm_pv_send_ipi(struct kvm *kvm, unsigned long ipi_bitmap_low,
		    unsigned long ipi_bitmap_high, u32 min,
		    unsigned long icr, int op_64_bit);

void kvm_define_shared_msr(unsigned index, u32 msr);
int kvm_set_shared_msr(unsigned index, u64 val, u64 mask);

u64 kvm_scale_tsc(struct kvm_vcpu *vcpu, u64 tsc);
u64 kvm_read_l1_tsc(struct kvm_vcpu *vcpu, u64 host_tsc);

unsigned long kvm_get_linear_rip(struct kvm_vcpu *vcpu);
bool kvm_is_linear_rip(struct kvm_vcpu *vcpu, unsigned long linear_rip);

void kvm_make_mclock_inprogress_request(struct kvm *kvm);
void kvm_make_scan_ioapic_request(struct kvm *kvm);
void kvm_make_scan_ioapic_request_mask(struct kvm *kvm,
				       unsigned long *vcpu_bitmap);

void kvm_arch_async_page_not_present(struct kvm_vcpu *vcpu,
				     struct kvm_async_pf *work);
void kvm_arch_async_page_present(struct kvm_vcpu *vcpu,
				 struct kvm_async_pf *work);
void kvm_arch_async_page_ready(struct kvm_vcpu *vcpu,
			       struct kvm_async_pf *work);
bool kvm_arch_can_inject_async_page_present(struct kvm_vcpu *vcpu);
extern bool kvm_find_async_pf_gfn(struct kvm_vcpu *vcpu, gfn_t gfn);

int kvm_skip_emulated_instruction(struct kvm_vcpu *vcpu);
int kvm_complete_insn_gp(struct kvm_vcpu *vcpu, int err);
void __kvm_request_immediate_exit(struct kvm_vcpu *vcpu);

int kvm_is_in_guest(void);

int __x86_set_memory_region(struct kvm *kvm, int id, gpa_t gpa, u32 size);
int x86_set_memory_region(struct kvm *kvm, int id, gpa_t gpa, u32 size);
bool kvm_vcpu_is_reset_bsp(struct kvm_vcpu *vcpu);
bool kvm_vcpu_is_bsp(struct kvm_vcpu *vcpu);

bool kvm_intr_is_single_vcpu(struct kvm *kvm, struct kvm_lapic_irq *irq,
			     struct kvm_vcpu **dest_vcpu);

void kvm_set_msi_irq(struct kvm *kvm, struct kvm_kernel_irq_routing_entry *e,
		     struct kvm_lapic_irq *irq);

static inline bool kvm_irq_is_postable(struct kvm_lapic_irq *irq)
{
	/* We can only post Fixed and LowPrio IRQs */
	return (irq->delivery_mode == dest_Fixed ||
		irq->delivery_mode == dest_LowestPrio);
}

static inline void kvm_arch_vcpu_blocking(struct kvm_vcpu *vcpu)
{
	if (kvm_x86_ops->vcpu_blocking)
		kvm_x86_ops->vcpu_blocking(vcpu);
}

static inline void kvm_arch_vcpu_unblocking(struct kvm_vcpu *vcpu)
{
	if (kvm_x86_ops->vcpu_unblocking)
		kvm_x86_ops->vcpu_unblocking(vcpu);
}

static inline void kvm_arch_vcpu_block_finish(struct kvm_vcpu *vcpu) {}

static inline int kvm_cpu_get_apicid(int mps_cpu)
{
#ifdef CONFIG_X86_LOCAL_APIC
	return default_cpu_present_to_apicid(mps_cpu);
#else
	WARN_ON_ONCE(1);
	return BAD_APICID;
#endif
}

#define put_smstate(type, buf, offset, val)                      \
	*(type *)((buf) + (offset) - 0x7e00) = val

#define GET_SMSTATE(type, buf, offset)		\
	(*(type *)((buf) + (offset) - 0x7e00))

#endif /* _ASM_X86_KVM_HOST_H */