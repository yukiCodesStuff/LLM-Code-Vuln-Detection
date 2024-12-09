struct kvm_s390_ucas_mapping {
	__u64 user_addr;
	__u64 vcpu_addr;
	__u64 length;
};
#define KVM_S390_UCAS_MAP        _IOW(KVMIO, 0x50, struct kvm_s390_ucas_mapping)
#define KVM_S390_UCAS_UNMAP      _IOW(KVMIO, 0x51, struct kvm_s390_ucas_mapping)
#define KVM_S390_VCPU_FAULT	 _IOW(KVMIO, 0x52, unsigned long)

/* Device model IOC */
#define KVM_CREATE_IRQCHIP        _IO(KVMIO,   0x60)
#define KVM_IRQ_LINE              _IOW(KVMIO,  0x61, struct kvm_irq_level)
#define KVM_GET_IRQCHIP           _IOWR(KVMIO, 0x62, struct kvm_irqchip)
#define KVM_SET_IRQCHIP           _IOR(KVMIO,  0x63, struct kvm_irqchip)
#define KVM_CREATE_PIT            _IO(KVMIO,   0x64)
#define KVM_GET_PIT               _IOWR(KVMIO, 0x65, struct kvm_pit_state)
#define KVM_SET_PIT               _IOR(KVMIO,  0x66, struct kvm_pit_state)
#define KVM_IRQ_LINE_STATUS       _IOWR(KVMIO, 0x67, struct kvm_irq_level)
#define KVM_REGISTER_COALESCED_MMIO \
			_IOW(KVMIO,  0x67, struct kvm_coalesced_mmio_zone)
#define KVM_UNREGISTER_COALESCED_MMIO \
			_IOW(KVMIO,  0x68, struct kvm_coalesced_mmio_zone)
#define KVM_ASSIGN_PCI_DEVICE     _IOR(KVMIO,  0x69, \
				       struct kvm_assigned_pci_dev)
#define KVM_SET_GSI_ROUTING       _IOW(KVMIO,  0x6a, struct kvm_irq_routing)
/* deprecated, replaced by KVM_ASSIGN_DEV_IRQ */
#define KVM_ASSIGN_IRQ            __KVM_DEPRECATED_VM_R_0x70
#define KVM_ASSIGN_DEV_IRQ        _IOW(KVMIO,  0x70, struct kvm_assigned_irq)
#define KVM_REINJECT_CONTROL      _IO(KVMIO,   0x71)
#define KVM_DEASSIGN_PCI_DEVICE   _IOW(KVMIO,  0x72, \
				       struct kvm_assigned_pci_dev)
#define KVM_ASSIGN_SET_MSIX_NR    _IOW(KVMIO,  0x73, \
				       struct kvm_assigned_msix_nr)
#define KVM_ASSIGN_SET_MSIX_ENTRY _IOW(KVMIO,  0x74, \
				       struct kvm_assigned_msix_entry)
#define KVM_DEASSIGN_DEV_IRQ      _IOW(KVMIO,  0x75, struct kvm_assigned_irq)
#define KVM_IRQFD                 _IOW(KVMIO,  0x76, struct kvm_irqfd)
#define KVM_CREATE_PIT2		  _IOW(KVMIO,  0x77, struct kvm_pit_config)
#define KVM_SET_BOOT_CPU_ID       _IO(KVMIO,   0x78)
#define KVM_IOEVENTFD             _IOW(KVMIO,  0x79, struct kvm_ioeventfd)
#define KVM_XEN_HVM_CONFIG        _IOW(KVMIO,  0x7a, struct kvm_xen_hvm_config)
#define KVM_SET_CLOCK             _IOW(KVMIO,  0x7b, struct kvm_clock_data)
#define KVM_GET_CLOCK             _IOR(KVMIO,  0x7c, struct kvm_clock_data)
/* Available with KVM_CAP_PIT_STATE2 */
#define KVM_GET_PIT2              _IOR(KVMIO,  0x9f, struct kvm_pit_state2)
#define KVM_SET_PIT2              _IOW(KVMIO,  0xa0, struct kvm_pit_state2)
/* Available with KVM_CAP_PPC_GET_PVINFO */
#define KVM_PPC_GET_PVINFO	  _IOW(KVMIO,  0xa1, struct kvm_ppc_pvinfo)
/* Available with KVM_CAP_TSC_CONTROL */
#define KVM_SET_TSC_KHZ           _IO(KVMIO,  0xa2)
#define KVM_GET_TSC_KHZ           _IO(KVMIO,  0xa3)
/* Available with KVM_CAP_PCI_2_3 */
#define KVM_ASSIGN_SET_INTX_MASK  _IOW(KVMIO,  0xa4, \
				       struct kvm_assigned_pci_dev)
/* Available with KVM_CAP_SIGNAL_MSI */
#define KVM_SIGNAL_MSI            _IOW(KVMIO,  0xa5, struct kvm_msi)
/* Available with KVM_CAP_PPC_GET_SMMU_INFO */
#define KVM_PPC_GET_SMMU_INFO	  _IOR(KVMIO,  0xa6, struct kvm_ppc_smmu_info)
/* Available with KVM_CAP_PPC_ALLOC_HTAB */
#define KVM_PPC_ALLOCATE_HTAB	  _IOWR(KVMIO, 0xa7, __u32)
#define KVM_CREATE_SPAPR_TCE	  _IOW(KVMIO,  0xa8, struct kvm_create_spapr_tce)
#define KVM_CREATE_SPAPR_TCE_64	  _IOW(KVMIO,  0xa8, \
				       struct kvm_create_spapr_tce_64)
/* Available with KVM_CAP_RMA */
#define KVM_ALLOCATE_RMA	  _IOR(KVMIO,  0xa9, struct kvm_allocate_rma)
/* Available with KVM_CAP_PPC_HTAB_FD */
#define KVM_PPC_GET_HTAB_FD	  _IOW(KVMIO,  0xaa, struct kvm_get_htab_fd)
/* Available with KVM_CAP_ARM_SET_DEVICE_ADDR */
#define KVM_ARM_SET_DEVICE_ADDR	  _IOW(KVMIO,  0xab, struct kvm_arm_device_addr)
/* Available with KVM_CAP_PPC_RTAS */
#define KVM_PPC_RTAS_DEFINE_TOKEN _IOW(KVMIO,  0xac, struct kvm_rtas_token_args)
/* Available with KVM_CAP_SPAPR_RESIZE_HPT */
#define KVM_PPC_RESIZE_HPT_PREPARE _IOR(KVMIO, 0xad, struct kvm_ppc_resize_hpt)
#define KVM_PPC_RESIZE_HPT_COMMIT  _IOR(KVMIO, 0xae, struct kvm_ppc_resize_hpt)
/* Available with KVM_CAP_PPC_RADIX_MMU or KVM_CAP_PPC_HASH_MMU_V3 */
#define KVM_PPC_CONFIGURE_V3_MMU  _IOW(KVMIO,  0xaf, struct kvm_ppc_mmuv3_cfg)
/* Available with KVM_CAP_PPC_RADIX_MMU */
#define KVM_PPC_GET_RMMU_INFO	  _IOW(KVMIO,  0xb0, struct kvm_ppc_rmmu_info)
/* Available with KVM_CAP_PPC_GET_CPU_CHAR */
#define KVM_PPC_GET_CPU_CHAR	  _IOR(KVMIO,  0xb1, struct kvm_ppc_cpu_char)
/* Available with KVM_CAP_PMU_EVENT_FILTER */
#define KVM_SET_PMU_EVENT_FILTER  _IOW(KVMIO,  0xb2, struct kvm_pmu_event_filter)

/* ioctl for vm fd */
#define KVM_CREATE_DEVICE	  _IOWR(KVMIO,  0xe0, struct kvm_create_device)

/* ioctls for fds returned by KVM_CREATE_DEVICE */
#define KVM_SET_DEVICE_ATTR	  _IOW(KVMIO,  0xe1, struct kvm_device_attr)
#define KVM_GET_DEVICE_ATTR	  _IOW(KVMIO,  0xe2, struct kvm_device_attr)
#define KVM_HAS_DEVICE_ATTR	  _IOW(KVMIO,  0xe3, struct kvm_device_attr)

/*
 * ioctls for vcpu fds
 */
#define KVM_RUN                   _IO(KVMIO,   0x80)
#define KVM_GET_REGS              _IOR(KVMIO,  0x81, struct kvm_regs)
#define KVM_SET_REGS              _IOW(KVMIO,  0x82, struct kvm_regs)
#define KVM_GET_SREGS             _IOR(KVMIO,  0x83, struct kvm_sregs)
#define KVM_SET_SREGS             _IOW(KVMIO,  0x84, struct kvm_sregs)
#define KVM_TRANSLATE             _IOWR(KVMIO, 0x85, struct kvm_translation)
#define KVM_INTERRUPT             _IOW(KVMIO,  0x86, struct kvm_interrupt)
/* KVM_DEBUG_GUEST is no longer supported, use KVM_SET_GUEST_DEBUG instead */
#define KVM_DEBUG_GUEST           __KVM_DEPRECATED_VCPU_W_0x87
#define KVM_GET_MSRS              _IOWR(KVMIO, 0x88, struct kvm_msrs)
#define KVM_SET_MSRS              _IOW(KVMIO,  0x89, struct kvm_msrs)
#define KVM_SET_CPUID             _IOW(KVMIO,  0x8a, struct kvm_cpuid)
#define KVM_SET_SIGNAL_MASK       _IOW(KVMIO,  0x8b, struct kvm_signal_mask)
#define KVM_GET_FPU               _IOR(KVMIO,  0x8c, struct kvm_fpu)
#define KVM_SET_FPU               _IOW(KVMIO,  0x8d, struct kvm_fpu)
#define KVM_GET_LAPIC             _IOR(KVMIO,  0x8e, struct kvm_lapic_state)
#define KVM_SET_LAPIC             _IOW(KVMIO,  0x8f, struct kvm_lapic_state)
#define KVM_SET_CPUID2            _IOW(KVMIO,  0x90, struct kvm_cpuid2)
#define KVM_GET_CPUID2            _IOWR(KVMIO, 0x91, struct kvm_cpuid2)
/* Available with KVM_CAP_VAPIC */
#define KVM_TPR_ACCESS_REPORTING  _IOWR(KVMIO, 0x92, struct kvm_tpr_access_ctl)
/* Available with KVM_CAP_VAPIC */
#define KVM_SET_VAPIC_ADDR        _IOW(KVMIO,  0x93, struct kvm_vapic_addr)
/* valid for virtual machine (for floating interrupt)_and_ vcpu */
#define KVM_S390_INTERRUPT        _IOW(KVMIO,  0x94, struct kvm_s390_interrupt)
/* store status for s390 */
#define KVM_S390_STORE_STATUS_NOADDR    (-1ul)
#define KVM_S390_STORE_STATUS_PREFIXED  (-2ul)
#define KVM_S390_STORE_STATUS	  _IOW(KVMIO,  0x95, unsigned long)
/* initial ipl psw for s390 */
#define KVM_S390_SET_INITIAL_PSW  _IOW(KVMIO,  0x96, struct kvm_s390_psw)
/* initial reset for s390 */
#define KVM_S390_INITIAL_RESET    _IO(KVMIO,   0x97)
#define KVM_GET_MP_STATE          _IOR(KVMIO,  0x98, struct kvm_mp_state)
#define KVM_SET_MP_STATE          _IOW(KVMIO,  0x99, struct kvm_mp_state)
/* Available with KVM_CAP_USER_NMI */
#define KVM_NMI                   _IO(KVMIO,   0x9a)
/* Available with KVM_CAP_SET_GUEST_DEBUG */
#define KVM_SET_GUEST_DEBUG       _IOW(KVMIO,  0x9b, struct kvm_guest_debug)
/* MCE for x86 */
#define KVM_X86_SETUP_MCE         _IOW(KVMIO,  0x9c, __u64)
#define KVM_X86_GET_MCE_CAP_SUPPORTED _IOR(KVMIO,  0x9d, __u64)
#define KVM_X86_SET_MCE           _IOW(KVMIO,  0x9e, struct kvm_x86_mce)
/* Available with KVM_CAP_VCPU_EVENTS */
#define KVM_GET_VCPU_EVENTS       _IOR(KVMIO,  0x9f, struct kvm_vcpu_events)
#define KVM_SET_VCPU_EVENTS       _IOW(KVMIO,  0xa0, struct kvm_vcpu_events)
/* Available with KVM_CAP_DEBUGREGS */
#define KVM_GET_DEBUGREGS         _IOR(KVMIO,  0xa1, struct kvm_debugregs)
#define KVM_SET_DEBUGREGS         _IOW(KVMIO,  0xa2, struct kvm_debugregs)
/*
 * vcpu version available with KVM_ENABLE_CAP
 * vm version available with KVM_CAP_ENABLE_CAP_VM
 */
#define KVM_ENABLE_CAP            _IOW(KVMIO,  0xa3, struct kvm_enable_cap)
/* Available with KVM_CAP_XSAVE */
#define KVM_GET_XSAVE		  _IOR(KVMIO,  0xa4, struct kvm_xsave)
#define KVM_SET_XSAVE		  _IOW(KVMIO,  0xa5, struct kvm_xsave)
/* Available with KVM_CAP_XCRS */
#define KVM_GET_XCRS		  _IOR(KVMIO,  0xa6, struct kvm_xcrs)
#define KVM_SET_XCRS		  _IOW(KVMIO,  0xa7, struct kvm_xcrs)
/* Available with KVM_CAP_SW_TLB */
#define KVM_DIRTY_TLB		  _IOW(KVMIO,  0xaa, struct kvm_dirty_tlb)
/* Available with KVM_CAP_ONE_REG */
#define KVM_GET_ONE_REG		  _IOW(KVMIO,  0xab, struct kvm_one_reg)
#define KVM_SET_ONE_REG		  _IOW(KVMIO,  0xac, struct kvm_one_reg)
/* VM is being stopped by host */
#define KVM_KVMCLOCK_CTRL	  _IO(KVMIO,   0xad)
#define KVM_ARM_VCPU_INIT	  _IOW(KVMIO,  0xae, struct kvm_vcpu_init)
#define KVM_ARM_PREFERRED_TARGET  _IOR(KVMIO,  0xaf, struct kvm_vcpu_init)
#define KVM_GET_REG_LIST	  _IOWR(KVMIO, 0xb0, struct kvm_reg_list)
/* Available with KVM_CAP_S390_MEM_OP */
#define KVM_S390_MEM_OP		  _IOW(KVMIO,  0xb1, struct kvm_s390_mem_op)
/* Available with KVM_CAP_S390_SKEYS */
#define KVM_S390_GET_SKEYS      _IOW(KVMIO, 0xb2, struct kvm_s390_skeys)
#define KVM_S390_SET_SKEYS      _IOW(KVMIO, 0xb3, struct kvm_s390_skeys)
/* Available with KVM_CAP_S390_INJECT_IRQ */
#define KVM_S390_IRQ              _IOW(KVMIO,  0xb4, struct kvm_s390_irq)
/* Available with KVM_CAP_S390_IRQ_STATE */
#define KVM_S390_SET_IRQ_STATE	  _IOW(KVMIO, 0xb5, struct kvm_s390_irq_state)
#define KVM_S390_GET_IRQ_STATE	  _IOW(KVMIO, 0xb6, struct kvm_s390_irq_state)
/* Available with KVM_CAP_X86_SMM */
#define KVM_SMI                   _IO(KVMIO,   0xb7)
/* Available with KVM_CAP_S390_CMMA_MIGRATION */
#define KVM_S390_GET_CMMA_BITS      _IOWR(KVMIO, 0xb8, struct kvm_s390_cmma_log)
#define KVM_S390_SET_CMMA_BITS      _IOW(KVMIO, 0xb9, struct kvm_s390_cmma_log)
/* Memory Encryption Commands */
#define KVM_MEMORY_ENCRYPT_OP      _IOWR(KVMIO, 0xba, unsigned long)

struct kvm_enc_region {
	__u64 addr;
	__u64 size;
};

#define KVM_MEMORY_ENCRYPT_REG_REGION    _IOR(KVMIO, 0xbb, struct kvm_enc_region)
#define KVM_MEMORY_ENCRYPT_UNREG_REGION  _IOR(KVMIO, 0xbc, struct kvm_enc_region)

/* Available with KVM_CAP_HYPERV_EVENTFD */
#define KVM_HYPERV_EVENTFD        _IOW(KVMIO,  0xbd, struct kvm_hyperv_eventfd)

/* Available with KVM_CAP_NESTED_STATE */
#define KVM_GET_NESTED_STATE         _IOWR(KVMIO, 0xbe, struct kvm_nested_state)
#define KVM_SET_NESTED_STATE         _IOW(KVMIO,  0xbf, struct kvm_nested_state)

/* Available with KVM_CAP_MANUAL_DIRTY_LOG_PROTECT_2 */
#define KVM_CLEAR_DIRTY_LOG          _IOWR(KVMIO, 0xc0, struct kvm_clear_dirty_log)

/* Available with KVM_CAP_HYPERV_CPUID */
#define KVM_GET_SUPPORTED_HV_CPUID _IOWR(KVMIO, 0xc1, struct kvm_cpuid2)

/* Available with KVM_CAP_ARM_SVE */
#define KVM_ARM_VCPU_FINALIZE	  _IOW(KVMIO,  0xc2, int)

/* Secure Encrypted Virtualization command */
enum sev_cmd_id {
	/* Guest initialization commands */
	KVM_SEV_INIT = 0,
	KVM_SEV_ES_INIT,
	/* Guest launch commands */
	KVM_SEV_LAUNCH_START,
	KVM_SEV_LAUNCH_UPDATE_DATA,
	KVM_SEV_LAUNCH_UPDATE_VMSA,
	KVM_SEV_LAUNCH_SECRET,
	KVM_SEV_LAUNCH_MEASURE,
	KVM_SEV_LAUNCH_FINISH,
	/* Guest migration commands (outgoing) */
	KVM_SEV_SEND_START,
	KVM_SEV_SEND_UPDATE_DATA,
	KVM_SEV_SEND_UPDATE_VMSA,
	KVM_SEV_SEND_FINISH,
	/* Guest migration commands (incoming) */
	KVM_SEV_RECEIVE_START,
	KVM_SEV_RECEIVE_UPDATE_DATA,
	KVM_SEV_RECEIVE_UPDATE_VMSA,
	KVM_SEV_RECEIVE_FINISH,
	/* Guest status and debug commands */
	KVM_SEV_GUEST_STATUS,
	KVM_SEV_DBG_DECRYPT,
	KVM_SEV_DBG_ENCRYPT,
	/* Guest certificates commands */
	KVM_SEV_CERT_EXPORT,

	KVM_SEV_NR_MAX,
};

struct kvm_sev_cmd {
	__u32 id;
	__u64 data;
	__u32 error;
	__u32 sev_fd;
};

struct kvm_sev_launch_start {
	__u32 handle;
	__u32 policy;
	__u64 dh_uaddr;
	__u32 dh_len;
	__u64 session_uaddr;
	__u32 session_len;
};

struct kvm_sev_launch_update_data {
	__u64 uaddr;
	__u32 len;
};


struct kvm_sev_launch_secret {
	__u64 hdr_uaddr;
	__u32 hdr_len;
	__u64 guest_uaddr;
	__u32 guest_len;
	__u64 trans_uaddr;
	__u32 trans_len;
};

struct kvm_sev_launch_measure {
	__u64 uaddr;
	__u32 len;
};

struct kvm_sev_guest_status {
	__u32 handle;
	__u32 policy;
	__u32 state;
};

struct kvm_sev_dbg {
	__u64 src_uaddr;
	__u64 dst_uaddr;
	__u32 len;
};

#define KVM_DEV_ASSIGN_ENABLE_IOMMU	(1 << 0)
#define KVM_DEV_ASSIGN_PCI_2_3		(1 << 1)
#define KVM_DEV_ASSIGN_MASK_INTX	(1 << 2)

struct kvm_assigned_pci_dev {
	__u32 assigned_dev_id;
	__u32 busnr;
	__u32 devfn;
	__u32 flags;
	__u32 segnr;
	union {
		__u32 reserved[11];
	};
};

#define KVM_DEV_IRQ_HOST_INTX    (1 << 0)
#define KVM_DEV_IRQ_HOST_MSI     (1 << 1)
#define KVM_DEV_IRQ_HOST_MSIX    (1 << 2)

#define KVM_DEV_IRQ_GUEST_INTX   (1 << 8)
#define KVM_DEV_IRQ_GUEST_MSI    (1 << 9)
#define KVM_DEV_IRQ_GUEST_MSIX   (1 << 10)

#define KVM_DEV_IRQ_HOST_MASK	 0x00ff
#define KVM_DEV_IRQ_GUEST_MASK   0xff00

struct kvm_assigned_irq {
	__u32 assigned_dev_id;
	__u32 host_irq; /* ignored (legacy field) */
	__u32 guest_irq;
	__u32 flags;
	union {
		__u32 reserved[12];
	};
};

struct kvm_assigned_msix_nr {
	__u32 assigned_dev_id;
	__u16 entry_nr;
	__u16 padding;
};

#define KVM_MAX_MSIX_PER_DEV		256
struct kvm_assigned_msix_entry {
	__u32 assigned_dev_id;
	__u32 gsi;
	__u16 entry; /* The index of entry in the MSI-X table */
	__u16 padding[3];
};

#define KVM_X2APIC_API_USE_32BIT_IDS            (1ULL << 0)
#define KVM_X2APIC_API_DISABLE_BROADCAST_QUIRK  (1ULL << 1)

/* Available with KVM_CAP_ARM_USER_IRQ */

/* Bits for run->s.regs.device_irq_level */
#define KVM_ARM_DEV_EL1_VTIMER		(1 << 0)
#define KVM_ARM_DEV_EL1_PTIMER		(1 << 1)
#define KVM_ARM_DEV_PMU			(1 << 2)

struct kvm_hyperv_eventfd {
	__u32 conn_id;
	__s32 fd;
	__u32 flags;
	__u32 padding[3];
};

#define KVM_HYPERV_CONN_ID_MASK		0x00ffffff
#define KVM_HYPERV_EVENTFD_DEASSIGN	(1 << 0)

#endif /* __LINUX_KVM_H */