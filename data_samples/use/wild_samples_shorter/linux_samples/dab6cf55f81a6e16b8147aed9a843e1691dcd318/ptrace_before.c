			unsigned long mask = PSW_MASK_USER;

			mask |= is_ri_task(child) ? PSW_MASK_RI : 0;
			if ((data & ~mask) != PSW_USER_BITS)
				return -EINVAL;
			if ((data & PSW_MASK_EA) && !(data & PSW_MASK_BA))
				return -EINVAL;
		}
		*(addr_t *)((addr_t) &task_pt_regs(child)->psw + addr) = data;


			mask |= is_ri_task(child) ? PSW32_MASK_RI : 0;
			/* Build a 64 bit psw mask from 31 bit mask. */
			if ((tmp & ~mask) != PSW32_USER_BITS)
				/* Invalid psw mask. */
				return -EINVAL;
			regs->psw.mask = (regs->psw.mask & ~PSW_MASK_USER) |
				(regs->psw.mask & PSW_MASK_BA) |
				(__u64)(tmp & mask) << 32;
		} else if (addr == (addr_t) &dummy32->regs.psw.addr) {