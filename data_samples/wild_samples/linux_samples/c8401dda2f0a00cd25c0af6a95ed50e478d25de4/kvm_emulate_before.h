struct x86_emulate_ctxt {
	const struct x86_emulate_ops *ops;

	/* Register state before/after emulation. */
	unsigned long eflags;
	unsigned long eip; /* eip before instruction emulation */
	/* Emulated execution mode, represented by an X86EMUL_MODE value. */
	enum x86emul_mode mode;

	/* interruptibility state, as a result of execution of STI or MOV SS */
	int interruptibility;

	bool perm_ok; /* do not check permissions if true */
	bool ud;	/* inject an #UD if host doesn't support insn */

	bool have_exception;
	struct x86_exception exception;

	/*
	 * decode cache
	 */

	/* current opcode length in bytes */
	u8 opcode_len;
	u8 b;
	u8 intercept;
	u8 op_bytes;
	u8 ad_bytes;
	struct operand src;
	struct operand src2;
	struct operand dst;
	int (*execute)(struct x86_emulate_ctxt *ctxt);
	int (*check_perm)(struct x86_emulate_ctxt *ctxt);
	/*
	 * The following six fields are cleared together,
	 * the rest are initialized unconditionally in x86_decode_insn
	 * or elsewhere
	 */
	bool rip_relative;
	u8 rex_prefix;
	u8 lock_prefix;
	u8 rep_prefix;
	/* bitmaps of registers in _regs[] that can be read */
	u32 regs_valid;
	/* bitmaps of registers in _regs[] that have been written */
	u32 regs_dirty;
	/* modrm */
	u8 modrm;
	u8 modrm_mod;
	u8 modrm_reg;
	u8 modrm_rm;
	u8 modrm_seg;
	u8 seg_override;
	u64 d;
	unsigned long _eip;
	struct operand memop;
	/* Fields above regs are cleared together. */
	unsigned long _regs[NR_VCPU_REGS];
	struct operand *memopp;
	struct fetch_cache fetch;
	struct read_cache io_read;
	struct read_cache mem_read;
};

/* Repeat String Operation Prefix */
#define REPE_PREFIX	0xf3
#define REPNE_PREFIX	0xf2

/* CPUID vendors */
#define X86EMUL_CPUID_VENDOR_AuthenticAMD_ebx 0x68747541
#define X86EMUL_CPUID_VENDOR_AuthenticAMD_ecx 0x444d4163
#define X86EMUL_CPUID_VENDOR_AuthenticAMD_edx 0x69746e65

#define X86EMUL_CPUID_VENDOR_AMDisbetterI_ebx 0x69444d41
#define X86EMUL_CPUID_VENDOR_AMDisbetterI_ecx 0x21726574
#define X86EMUL_CPUID_VENDOR_AMDisbetterI_edx 0x74656273

#define X86EMUL_CPUID_VENDOR_GenuineIntel_ebx 0x756e6547
#define X86EMUL_CPUID_VENDOR_GenuineIntel_ecx 0x6c65746e
#define X86EMUL_CPUID_VENDOR_GenuineIntel_edx 0x49656e69

enum x86_intercept_stage {
	X86_ICTP_NONE = 0,   /* Allow zero-init to not match anything */
	X86_ICPT_PRE_EXCEPT,
	X86_ICPT_POST_EXCEPT,
	X86_ICPT_POST_MEMACCESS,
};

enum x86_intercept {
	x86_intercept_none,
	x86_intercept_cr_read,
	x86_intercept_cr_write,
	x86_intercept_clts,
	x86_intercept_lmsw,
	x86_intercept_smsw,
	x86_intercept_dr_read,
	x86_intercept_dr_write,
	x86_intercept_lidt,
	x86_intercept_sidt,
	x86_intercept_lgdt,
	x86_intercept_sgdt,
	x86_intercept_lldt,
	x86_intercept_sldt,
	x86_intercept_ltr,
	x86_intercept_str,
	x86_intercept_rdtsc,
	x86_intercept_rdpmc,
	x86_intercept_pushf,
	x86_intercept_popf,
	x86_intercept_cpuid,
	x86_intercept_rsm,
	x86_intercept_iret,
	x86_intercept_intn,
	x86_intercept_invd,
	x86_intercept_pause,
	x86_intercept_hlt,
	x86_intercept_invlpg,
	x86_intercept_invlpga,
	x86_intercept_vmrun,
	x86_intercept_vmload,
	x86_intercept_vmsave,
	x86_intercept_vmmcall,
	x86_intercept_stgi,
	x86_intercept_clgi,
	x86_intercept_skinit,
	x86_intercept_rdtscp,
	x86_intercept_icebp,
	x86_intercept_wbinvd,
	x86_intercept_monitor,
	x86_intercept_mwait,
	x86_intercept_rdmsr,
	x86_intercept_wrmsr,
	x86_intercept_in,
	x86_intercept_ins,
	x86_intercept_out,
	x86_intercept_outs,

	nr_x86_intercepts
};

/* Host execution mode. */
#if defined(CONFIG_X86_32)
#define X86EMUL_MODE_HOST X86EMUL_MODE_PROT32
#elif defined(CONFIG_X86_64)
#define X86EMUL_MODE_HOST X86EMUL_MODE_PROT64
#endif

int x86_decode_insn(struct x86_emulate_ctxt *ctxt, void *insn, int insn_len);
bool x86_page_table_writing_insn(struct x86_emulate_ctxt *ctxt);
#define EMULATION_FAILED -1
#define EMULATION_OK 0
#define EMULATION_RESTART 1
#define EMULATION_INTERCEPTED 2
void init_decode_cache(struct x86_emulate_ctxt *ctxt);
int x86_emulate_insn(struct x86_emulate_ctxt *ctxt);
int emulator_task_switch(struct x86_emulate_ctxt *ctxt,
			 u16 tss_selector, int idt_index, int reason,
			 bool has_error_code, u32 error_code);
int emulate_int_real(struct x86_emulate_ctxt *ctxt, int irq);
void emulator_invalidate_register_cache(struct x86_emulate_ctxt *ctxt);
void emulator_writeback_register_cache(struct x86_emulate_ctxt *ctxt);
bool emulator_can_use_gpa(struct x86_emulate_ctxt *ctxt);

#endif /* _ASM_X86_KVM_X86_EMULATE_H */