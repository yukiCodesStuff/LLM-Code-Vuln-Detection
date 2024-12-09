	return false;
}

int arch_uprobe_post_xol(struct arch_uprobe *auprobe, struct pt_regs *regs)
{
	int fixup = probe_get_fixup_type(auprobe->insn);
	struct uprobe_task *utask = current->utask;
		if (regs->psw.addr - utask->xol_vaddr == ilen)
			regs->psw.addr = utask->vaddr + ilen;
	}
	/* If per tracing was active generate trap */
	if (regs->psw.mask & PSW_MASK_PER)
		do_per_trap(regs);
	return 0;
}

int arch_uprobe_exception_notify(struct notifier_block *self, unsigned long val,
	clear_thread_flag(TIF_UPROBE_SINGLESTEP);
	regs->int_code = auprobe->saved_int_code;
	regs->psw.addr = current->utask->vaddr;
}

unsigned long arch_uretprobe_hijack_return_addr(unsigned long trampoline,
						struct pt_regs *regs)
	__rc;						\
})

#define emu_store_ril(ptr, input)			\
({							\
	unsigned int mask = sizeof(*(ptr)) - 1;		\
	int __rc = 0;					\
							\
	if (!test_facility(34))				\
		__rc = EMU_ILLEGAL_OP;			\
	else if ((u64 __force)ptr & mask)		\
		__rc = EMU_SPECIFICATION;		\
	else if (put_user(*(input), ptr))		\
		__rc = EMU_ADDRESSING;			\
	__rc;						\
})

#define emu_cmp_ril(regs, ptr, cmp)			\
	s16 s16[4];
};

/*
 * pc relative instructions are emulated, since parameters may not be
 * accessible from the xol area due to range limitations.
 */
			rc = emu_load_ril((u32 __user *)uptr, &rx->u64);
			break;
		case 0x07: /* sthrl */
			rc = emu_store_ril((u16 __user *)uptr, &rx->u16[3]);
			break;
		case 0x0b: /* stgrl */
			rc = emu_store_ril((u64 __user *)uptr, &rx->u64);
			break;
		case 0x0f: /* strl */
			rc = emu_store_ril((u32 __user *)uptr, &rx->u32[1]);
			break;
		}
		break;
	case 0xc6: