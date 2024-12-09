	return false;
}

static int check_per_event(unsigned short cause, unsigned long control,
			   struct pt_regs *regs)
{
	if (!(regs->psw.mask & PSW_MASK_PER))
		return 0;
	/* user space single step */
	if (control == 0)
		return 1;
	/* over indication for storage alteration */
	if ((control & 0x20200000) && (cause & 0x2000))
		return 1;
	if (cause & 0x8000) {
		/* all branches */
		if ((control & 0x80800000) == 0x80000000)
			return 1;
		/* branch into selected range */
		if (((control & 0x80800000) == 0x80800000) &&
		    regs->psw.addr >= current->thread.per_user.start &&
		    regs->psw.addr <= current->thread.per_user.end)
			return 1;
	}
	return 0;
}

int arch_uprobe_post_xol(struct arch_uprobe *auprobe, struct pt_regs *regs)
{
	int fixup = probe_get_fixup_type(auprobe->insn);
	struct uprobe_task *utask = current->utask;
		if (regs->psw.addr - utask->xol_vaddr == ilen)
			regs->psw.addr = utask->vaddr + ilen;
	}
	if (check_per_event(current->thread.per_event.cause,
			    current->thread.per_user.control, regs)) {
		/* fix per address */
		current->thread.per_event.address = utask->vaddr;
		/* trigger per event */
		set_pt_regs_flag(regs, PIF_PER_TRAP);
	}
	return 0;
}

int arch_uprobe_exception_notify(struct notifier_block *self, unsigned long val,
	clear_thread_flag(TIF_UPROBE_SINGLESTEP);
	regs->int_code = auprobe->saved_int_code;
	regs->psw.addr = current->utask->vaddr;
	current->thread.per_event.address = current->utask->vaddr;
}

unsigned long arch_uretprobe_hijack_return_addr(unsigned long trampoline,
						struct pt_regs *regs)
	__rc;						\
})

#define emu_store_ril(regs, ptr, input)			\
({							\
	unsigned int mask = sizeof(*(ptr)) - 1;		\
	__typeof__(ptr) __ptr = (ptr);			\
	int __rc = 0;					\
							\
	if (!test_facility(34))				\
		__rc = EMU_ILLEGAL_OP;			\
	else if ((u64 __force)__ptr & mask)		\
		__rc = EMU_SPECIFICATION;		\
	else if (put_user(*(input), __ptr))		\
		__rc = EMU_ADDRESSING;			\
	if (__rc == 0)					\
		sim_stor_event(regs, __ptr, mask + 1);	\
	__rc;						\
})

#define emu_cmp_ril(regs, ptr, cmp)			\
	s16 s16[4];
};

/*
 * If user per registers are setup to trace storage alterations and an
 * emulated store took place on a fitting address a user trap is generated.
 */
static void sim_stor_event(struct pt_regs *regs, void *addr, int len)
{
	if (!(regs->psw.mask & PSW_MASK_PER))
		return;
	if (!(current->thread.per_user.control & PER_EVENT_STORE))
		return;
	if ((void *)current->thread.per_user.start > (addr + len))
		return;
	if ((void *)current->thread.per_user.end < addr)
		return;
	current->thread.per_event.address = regs->psw.addr;
	current->thread.per_event.cause = PER_EVENT_STORE >> 16;
	set_pt_regs_flag(regs, PIF_PER_TRAP);
}

/*
 * pc relative instructions are emulated, since parameters may not be
 * accessible from the xol area due to range limitations.
 */
			rc = emu_load_ril((u32 __user *)uptr, &rx->u64);
			break;
		case 0x07: /* sthrl */
			rc = emu_store_ril(regs, (u16 __user *)uptr, &rx->u16[3]);
			break;
		case 0x0b: /* stgrl */
			rc = emu_store_ril(regs, (u64 __user *)uptr, &rx->u64);
			break;
		case 0x0f: /* strl */
			rc = emu_store_ril(regs, (u32 __user *)uptr, &rx->u32[1]);
			break;
		}
		break;
	case 0xc6: