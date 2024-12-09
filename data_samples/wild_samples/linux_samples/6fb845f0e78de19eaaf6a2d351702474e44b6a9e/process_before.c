{
	bool is_mmips = IS_ENABLED(CONFIG_CPU_MICROMIPS);
	union mips_instruction insn, *ip, *ip_end;
	const unsigned int max_insns = 128;
	unsigned int last_insn_size = 0;
	unsigned int i;
	bool saw_jump = false;

	info->pc_offset = -1;
	info->frame_size = 0;

	ip = (void *)msk_isa16_mode((ulong)info->func);
	if (!ip)
		goto err;

	ip_end = (void *)ip + info->func_size;

	for (i = 0; i < max_insns && ip < ip_end; i++) {
		ip = (void *)ip + last_insn_size;
		if (is_mmips && mm_insn_16bit(ip->halfword[0])) {
			insn.word = ip->halfword[0] << 16;
			last_insn_size = 2;
		} else if (is_mmips) {
			insn.word = ip->halfword[0] << 16 | ip->halfword[1];
			last_insn_size = 4;
		} else {
			insn.word = ip->word;
			last_insn_size = 4;
		}

		if (!info->frame_size) {
			is_sp_move_ins(&insn, &info->frame_size);
			continue;
		} else if (!saw_jump && is_jump_ins(ip)) {
			/*
			 * If we see a jump instruction, we are finished
			 * with the frame save.
			 *
			 * Some functions can have a shortcut return at
			 * the beginning of the function, so don't start
			 * looking for jump instruction until we see the
			 * frame setup.
			 *
			 * The RA save instruction can get put into the
			 * delay slot of the jump instruction, so look
			 * at the next instruction, too.
			 */
			saw_jump = true;
			continue;
		}
		if (info->pc_offset == -1 &&
		    is_ra_save_ins(&insn, &info->pc_offset))
			break;
		if (saw_jump)
			break;
	}
	if (info->frame_size && info->pc_offset >= 0) /* nested */
		return 0;
	if (info->pc_offset < 0) /* leaf */
		return 1;
	/* prologue seems bogus... */
err:
	return -1;
}

static struct mips_frame_info schedule_mfi __read_mostly;

#ifdef CONFIG_KALLSYMS
static unsigned long get___schedule_addr(void)
{
	return kallsyms_lookup_name("__schedule");
}
#else
static unsigned long get___schedule_addr(void)
{
	union mips_instruction *ip = (void *)schedule;
	int max_insns = 8;
	int i;

	for (i = 0; i < max_insns; i++, ip++) {
		if (ip->j_format.opcode == j_op)
			return J_TARGET(ip, ip->j_format.target);
	}
	return 0;
}
#endif

static int __init frame_info_init(void)
{
	unsigned long size = 0;
#ifdef CONFIG_KALLSYMS
	unsigned long ofs;
#endif
	unsigned long addr;

	addr = get___schedule_addr();
	if (!addr)
		addr = (unsigned long)schedule;

#ifdef CONFIG_KALLSYMS
	kallsyms_lookup_size_offset(addr, &size, &ofs);
#endif
	schedule_mfi.func = (void *)addr;
	schedule_mfi.func_size = size;

	get_frame_info(&schedule_mfi);

	/*
	 * Without schedule() frame info, result given by
	 * thread_saved_pc() and get_wchan() are not reliable.
	 */
	if (schedule_mfi.pc_offset < 0)
		printk("Can't analyze schedule() prologue at %p\n", schedule);

	return 0;
}

arch_initcall(frame_info_init);

/*
 * Return saved PC of a blocked thread.
 */
static unsigned long thread_saved_pc(struct task_struct *tsk)
{
	struct thread_struct *t = &tsk->thread;

	/* New born processes are a special case */
	if (t->reg31 == (unsigned long) ret_from_fork)
		return t->reg31;
	if (schedule_mfi.pc_offset < 0)
		return 0;
	return ((unsigned long *)t->reg29)[schedule_mfi.pc_offset];
}


#ifdef CONFIG_KALLSYMS
/* generic stack unwinding function */
unsigned long notrace unwind_stack_by_address(unsigned long stack_page,
					      unsigned long *sp,
					      unsigned long pc,
					      unsigned long *ra)
{
	unsigned long low, high, irq_stack_high;
	struct mips_frame_info info;
	unsigned long size, ofs;
	struct pt_regs *regs;
	int leaf;

	if (!stack_page)
		return 0;

	/*
	 * IRQ stacks start at IRQ_STACK_START
	 * task stacks at THREAD_SIZE - 32
	 */
	low = stack_page;
	if (!preemptible() && on_irq_stack(raw_smp_processor_id(), *sp)) {
		high = stack_page + IRQ_STACK_START;
		irq_stack_high = high;
	} else {
		high = stack_page + THREAD_SIZE - 32;
		irq_stack_high = 0;
	}

	/*
	 * If we reached the top of the interrupt stack, start unwinding
	 * the interrupted task stack.
	 */
	if (unlikely(*sp == irq_stack_high)) {
		unsigned long task_sp = *(unsigned long *)*sp;

		/*
		 * Check that the pointer saved in the IRQ stack head points to
		 * something within the stack of the current task
		 */
		if (!object_is_on_stack((void *)task_sp))
			return 0;

		/*
		 * Follow pointer to tasks kernel stack frame where interrupted
		 * state was saved.
		 */
		regs = (struct pt_regs *)task_sp;
		pc = regs->cp0_epc;
		if (!user_mode(regs) && __kernel_text_address(pc)) {
			*sp = regs->regs[29];
			*ra = regs->regs[31];
			return pc;
		}
		return 0;
	}
	if (!kallsyms_lookup_size_offset(pc, &size, &ofs))
		return 0;
	/*
	 * Return ra if an exception occurred at the first instruction
	 */
	if (unlikely(ofs == 0)) {
		pc = *ra;
		*ra = 0;
		return pc;
	}

	info.func = (void *)(pc - ofs);
	info.func_size = ofs;	/* analyze from start to ofs */
	leaf = get_frame_info(&info);
	if (leaf < 0)
		return 0;

	if (*sp < low || *sp + info.frame_size > high)
		return 0;

	if (leaf)
		/*
		 * For some extreme cases, get_frame_info() can
		 * consider wrongly a nested function as a leaf
		 * one. In that cases avoid to return always the
		 * same value.
		 */
		pc = pc != *ra ? *ra : 0;
	else
		pc = ((unsigned long *)(*sp))[info.pc_offset];

	*sp += info.frame_size;
	*ra = 0;
	return __kernel_text_address(pc) ? pc : 0;
}
EXPORT_SYMBOL(unwind_stack_by_address);

/* used by show_backtrace() */
unsigned long unwind_stack(struct task_struct *task, unsigned long *sp,
			   unsigned long pc, unsigned long *ra)
{
	unsigned long stack_page = 0;
	int cpu;

	for_each_possible_cpu(cpu) {
		if (on_irq_stack(cpu, *sp)) {
			stack_page = (unsigned long)irq_stack[cpu];
			break;
		}
	}

	if (!stack_page)
		stack_page = (unsigned long)task_stack_page(task);

	return unwind_stack_by_address(stack_page, sp, pc, ra);
}
#endif

/*
 * get_wchan - a maintenance nightmare^W^Wpain in the ass ...
 */
unsigned long get_wchan(struct task_struct *task)
{
	unsigned long pc = 0;
#ifdef CONFIG_KALLSYMS
	unsigned long sp;
	unsigned long ra = 0;
#endif

	if (!task || task == current || task->state == TASK_RUNNING)
		goto out;
	if (!task_stack_page(task))
		goto out;

	pc = thread_saved_pc(task);

#ifdef CONFIG_KALLSYMS
	sp = task->thread.reg29 + schedule_mfi.frame_size;

	while (in_sched_functions(pc))
		pc = unwind_stack(task, &sp, pc, &ra);
#endif

out:
	return pc;
}

unsigned long mips_stack_top(void)
{
	unsigned long top = TASK_SIZE & PAGE_MASK;

	/* One page for branch delay slot "emulation" */
	top -= PAGE_SIZE;

	/* Space for the VDSO, data page & GIC user page */
	top -= PAGE_ALIGN(current->thread.abi->vdso->size);
	top -= PAGE_SIZE;
	top -= mips_gic_present() ? PAGE_SIZE : 0;

	/* Space for cache colour alignment */
	if (cpu_has_dc_aliases)
		top -= shm_align_mask + 1;

	/* Space to randomize the VDSO base */
	if (current->flags & PF_RANDOMIZE)
		top -= VDSO_RANDOMIZE_SIZE;

	return top;
}

/*
 * Don't forget that the stack pointer must be aligned on a 8 bytes
 * boundary for 32-bits ABI and 16 bytes for 64-bits ABI.
 */
unsigned long arch_align_stack(unsigned long sp)
{
	if (!(current->personality & ADDR_NO_RANDOMIZE) && randomize_va_space)
		sp -= get_random_int() & ~PAGE_MASK;

	return sp & ALMASK;
}

static DEFINE_PER_CPU(call_single_data_t, backtrace_csd);
static struct cpumask backtrace_csd_busy;

static void handle_backtrace(void *info)
{
	nmi_cpu_backtrace(get_irq_regs());
	cpumask_clear_cpu(smp_processor_id(), &backtrace_csd_busy);
}

static void raise_backtrace(cpumask_t *mask)
{
	call_single_data_t *csd;
	int cpu;

	for_each_cpu(cpu, mask) {
		/*
		 * If we previously sent an IPI to the target CPU & it hasn't
		 * cleared its bit in the busy cpumask then it didn't handle
		 * our previous IPI & it's not safe for us to reuse the
		 * call_single_data_t.
		 */
		if (cpumask_test_and_set_cpu(cpu, &backtrace_csd_busy)) {
			pr_warn("Unable to send backtrace IPI to CPU%u - perhaps it hung?\n",
				cpu);
			continue;
		}

		csd = &per_cpu(backtrace_csd, cpu);
		csd->func = handle_backtrace;
		smp_call_function_single_async(cpu, csd);
	}
}

void arch_trigger_cpumask_backtrace(const cpumask_t *mask, bool exclude_self)
{
	nmi_trigger_cpumask_backtrace(mask, exclude_self, raise_backtrace);
}

int mips_get_process_fp_mode(struct task_struct *task)
{
	int value = 0;

	if (!test_tsk_thread_flag(task, TIF_32BIT_FPREGS))
		value |= PR_FP_MODE_FR;
	if (test_tsk_thread_flag(task, TIF_HYBRID_FPREGS))
		value |= PR_FP_MODE_FRE;

	return value;
}

static long prepare_for_fp_mode_switch(void *unused)
{
	/*
	 * This is icky, but we use this to simply ensure that all CPUs have
	 * context switched, regardless of whether they were previously running
	 * kernel or user code. This ensures that no CPU that a mode-switching
	 * program may execute on keeps its FPU enabled (& in the old mode)
	 * throughout the mode switch.
	 */
	return 0;
}

int mips_set_process_fp_mode(struct task_struct *task, unsigned int value)
{
	const unsigned int known_bits = PR_FP_MODE_FR | PR_FP_MODE_FRE;
	struct task_struct *t;
	struct cpumask process_cpus;
	int cpu;

	/* If nothing to change, return right away, successfully.  */
	if (value == mips_get_process_fp_mode(task))
		return 0;

	/* Only accept a mode change if 64-bit FP enabled for o32.  */
	if (!IS_ENABLED(CONFIG_MIPS_O32_FP64_SUPPORT))
		return -EOPNOTSUPP;

	/* And only for o32 tasks.  */
	if (IS_ENABLED(CONFIG_64BIT) && !test_thread_flag(TIF_32BIT_REGS))
		return -EOPNOTSUPP;

	/* Check the value is valid */
	if (value & ~known_bits)
		return -EOPNOTSUPP;

	/* Setting FRE without FR is not supported.  */
	if ((value & (PR_FP_MODE_FR | PR_FP_MODE_FRE)) == PR_FP_MODE_FRE)
		return -EOPNOTSUPP;

	/* Avoid inadvertently triggering emulation */
	if ((value & PR_FP_MODE_FR) && raw_cpu_has_fpu &&
	    !(raw_current_cpu_data.fpu_id & MIPS_FPIR_F64))
		return -EOPNOTSUPP;
	if ((value & PR_FP_MODE_FRE) && raw_cpu_has_fpu && !cpu_has_fre)
		return -EOPNOTSUPP;

	/* FR = 0 not supported in MIPS R6 */
	if (!(value & PR_FP_MODE_FR) && raw_cpu_has_fpu && cpu_has_mips_r6)
		return -EOPNOTSUPP;

	/* Indicate the new FP mode in each thread */
	for_each_thread(task, t) {
		/* Update desired FP register width */
		if (value & PR_FP_MODE_FR) {
			clear_tsk_thread_flag(t, TIF_32BIT_FPREGS);
		} else {
			set_tsk_thread_flag(t, TIF_32BIT_FPREGS);
			clear_tsk_thread_flag(t, TIF_MSA_CTX_LIVE);
		}

		/* Update desired FP single layout */
		if (value & PR_FP_MODE_FRE)
			set_tsk_thread_flag(t, TIF_HYBRID_FPREGS);
		else
			clear_tsk_thread_flag(t, TIF_HYBRID_FPREGS);
	}

	/*
	 * We need to ensure that all threads in the process have switched mode
	 * before returning, in order to allow userland to not worry about
	 * races. We can do this by forcing all CPUs that any thread in the
	 * process may be running on to schedule something else - in this case
	 * prepare_for_fp_mode_switch().
	 *
	 * We begin by generating a mask of all CPUs that any thread in the
	 * process may be running on.
	 */
	cpumask_clear(&process_cpus);
	for_each_thread(task, t)
		cpumask_set_cpu(task_cpu(t), &process_cpus);

	/*
	 * Now we schedule prepare_for_fp_mode_switch() on each of those CPUs.
	 *
	 * The CPUs may have rescheduled already since we switched mode or
	 * generated the cpumask, but that doesn't matter. If the task in this
	 * process is scheduled out then our scheduling
	 * prepare_for_fp_mode_switch() will simply be redundant. If it's
	 * scheduled in then it will already have picked up the new FP mode
	 * whilst doing so.
	 */
	get_online_cpus();
	for_each_cpu_and(cpu, &process_cpus, cpu_online_mask)
		work_on_cpu(cpu, prepare_for_fp_mode_switch, NULL);
	put_online_cpus();

	return 0;
}

#if defined(CONFIG_32BIT) || defined(CONFIG_MIPS32_O32)
void mips_dump_regs32(u32 *uregs, const struct pt_regs *regs)
{
	unsigned int i;

	for (i = MIPS32_EF_R1; i <= MIPS32_EF_R31; i++) {
		/* k0/k1 are copied as zero. */
		if (i == MIPS32_EF_R26 || i == MIPS32_EF_R27)
			uregs[i] = 0;
		else
			uregs[i] = regs->regs[i - MIPS32_EF_R0];
	}

	uregs[MIPS32_EF_LO] = regs->lo;
	uregs[MIPS32_EF_HI] = regs->hi;
	uregs[MIPS32_EF_CP0_EPC] = regs->cp0_epc;
	uregs[MIPS32_EF_CP0_BADVADDR] = regs->cp0_badvaddr;
	uregs[MIPS32_EF_CP0_STATUS] = regs->cp0_status;
	uregs[MIPS32_EF_CP0_CAUSE] = regs->cp0_cause;
}
#endif /* CONFIG_32BIT || CONFIG_MIPS32_O32 */

#ifdef CONFIG_64BIT
void mips_dump_regs64(u64 *uregs, const struct pt_regs *regs)
{
	unsigned int i;

	for (i = MIPS64_EF_R1; i <= MIPS64_EF_R31; i++) {
		/* k0/k1 are copied as zero. */
		if (i == MIPS64_EF_R26 || i == MIPS64_EF_R27)
			uregs[i] = 0;
		else
			uregs[i] = regs->regs[i - MIPS64_EF_R0];
	}

	uregs[MIPS64_EF_LO] = regs->lo;
	uregs[MIPS64_EF_HI] = regs->hi;
	uregs[MIPS64_EF_CP0_EPC] = regs->cp0_epc;
	uregs[MIPS64_EF_CP0_BADVADDR] = regs->cp0_badvaddr;
	uregs[MIPS64_EF_CP0_STATUS] = regs->cp0_status;
	uregs[MIPS64_EF_CP0_CAUSE] = regs->cp0_cause;
}
#endif /* CONFIG_64BIT */
{
	bool is_mmips = IS_ENABLED(CONFIG_CPU_MICROMIPS);
	union mips_instruction insn, *ip, *ip_end;
	const unsigned int max_insns = 128;
	unsigned int last_insn_size = 0;
	unsigned int i;
	bool saw_jump = false;

	info->pc_offset = -1;
	info->frame_size = 0;

	ip = (void *)msk_isa16_mode((ulong)info->func);
	if (!ip)
		goto err;

	ip_end = (void *)ip + info->func_size;

	for (i = 0; i < max_insns && ip < ip_end; i++) {
		ip = (void *)ip + last_insn_size;
		if (is_mmips && mm_insn_16bit(ip->halfword[0])) {
			insn.word = ip->halfword[0] << 16;
			last_insn_size = 2;
		} else if (is_mmips) {
			insn.word = ip->halfword[0] << 16 | ip->halfword[1];
			last_insn_size = 4;
		} else {
			insn.word = ip->word;
			last_insn_size = 4;
		}

		if (!info->frame_size) {
			is_sp_move_ins(&insn, &info->frame_size);
			continue;
		} else if (!saw_jump && is_jump_ins(ip)) {
			/*
			 * If we see a jump instruction, we are finished
			 * with the frame save.
			 *
			 * Some functions can have a shortcut return at
			 * the beginning of the function, so don't start
			 * looking for jump instruction until we see the
			 * frame setup.
			 *
			 * The RA save instruction can get put into the
			 * delay slot of the jump instruction, so look
			 * at the next instruction, too.
			 */
			saw_jump = true;
			continue;
		}
		if (info->pc_offset == -1 &&
		    is_ra_save_ins(&insn, &info->pc_offset))
			break;
		if (saw_jump)
			break;
	}
	if (info->frame_size && info->pc_offset >= 0) /* nested */
		return 0;
	if (info->pc_offset < 0) /* leaf */
		return 1;
	/* prologue seems bogus... */
err:
	return -1;
}

static struct mips_frame_info schedule_mfi __read_mostly;

#ifdef CONFIG_KALLSYMS
static unsigned long get___schedule_addr(void)
{
	return kallsyms_lookup_name("__schedule");
}
#else
static unsigned long get___schedule_addr(void)
{
	union mips_instruction *ip = (void *)schedule;
	int max_insns = 8;
	int i;

	for (i = 0; i < max_insns; i++, ip++) {
		if (ip->j_format.opcode == j_op)
			return J_TARGET(ip, ip->j_format.target);
	}
	return 0;
}
#endif

static int __init frame_info_init(void)
{
	unsigned long size = 0;
#ifdef CONFIG_KALLSYMS
	unsigned long ofs;
#endif
	unsigned long addr;

	addr = get___schedule_addr();
	if (!addr)
		addr = (unsigned long)schedule;

#ifdef CONFIG_KALLSYMS
	kallsyms_lookup_size_offset(addr, &size, &ofs);
#endif
	schedule_mfi.func = (void *)addr;
	schedule_mfi.func_size = size;

	get_frame_info(&schedule_mfi);

	/*
	 * Without schedule() frame info, result given by
	 * thread_saved_pc() and get_wchan() are not reliable.
	 */
	if (schedule_mfi.pc_offset < 0)
		printk("Can't analyze schedule() prologue at %p\n", schedule);

	return 0;
}

arch_initcall(frame_info_init);

/*
 * Return saved PC of a blocked thread.
 */
static unsigned long thread_saved_pc(struct task_struct *tsk)
{
	struct thread_struct *t = &tsk->thread;

	/* New born processes are a special case */
	if (t->reg31 == (unsigned long) ret_from_fork)
		return t->reg31;
	if (schedule_mfi.pc_offset < 0)
		return 0;
	return ((unsigned long *)t->reg29)[schedule_mfi.pc_offset];
}


#ifdef CONFIG_KALLSYMS
/* generic stack unwinding function */
unsigned long notrace unwind_stack_by_address(unsigned long stack_page,
					      unsigned long *sp,
					      unsigned long pc,
					      unsigned long *ra)
{
	unsigned long low, high, irq_stack_high;
	struct mips_frame_info info;
	unsigned long size, ofs;
	struct pt_regs *regs;
	int leaf;

	if (!stack_page)
		return 0;

	/*
	 * IRQ stacks start at IRQ_STACK_START
	 * task stacks at THREAD_SIZE - 32
	 */
	low = stack_page;
	if (!preemptible() && on_irq_stack(raw_smp_processor_id(), *sp)) {
		high = stack_page + IRQ_STACK_START;
		irq_stack_high = high;
	} else {
		high = stack_page + THREAD_SIZE - 32;
		irq_stack_high = 0;
	}

	/*
	 * If we reached the top of the interrupt stack, start unwinding
	 * the interrupted task stack.
	 */
	if (unlikely(*sp == irq_stack_high)) {
		unsigned long task_sp = *(unsigned long *)*sp;

		/*
		 * Check that the pointer saved in the IRQ stack head points to
		 * something within the stack of the current task
		 */
		if (!object_is_on_stack((void *)task_sp))
			return 0;

		/*
		 * Follow pointer to tasks kernel stack frame where interrupted
		 * state was saved.
		 */
		regs = (struct pt_regs *)task_sp;
		pc = regs->cp0_epc;
		if (!user_mode(regs) && __kernel_text_address(pc)) {
			*sp = regs->regs[29];
			*ra = regs->regs[31];
			return pc;
		}
		return 0;
	}
	if (!kallsyms_lookup_size_offset(pc, &size, &ofs))
		return 0;
	/*
	 * Return ra if an exception occurred at the first instruction
	 */
	if (unlikely(ofs == 0)) {
		pc = *ra;
		*ra = 0;
		return pc;
	}

	info.func = (void *)(pc - ofs);
	info.func_size = ofs;	/* analyze from start to ofs */
	leaf = get_frame_info(&info);
	if (leaf < 0)
		return 0;

	if (*sp < low || *sp + info.frame_size > high)
		return 0;

	if (leaf)
		/*
		 * For some extreme cases, get_frame_info() can
		 * consider wrongly a nested function as a leaf
		 * one. In that cases avoid to return always the
		 * same value.
		 */
		pc = pc != *ra ? *ra : 0;
	else
		pc = ((unsigned long *)(*sp))[info.pc_offset];

	*sp += info.frame_size;
	*ra = 0;
	return __kernel_text_address(pc) ? pc : 0;
}
EXPORT_SYMBOL(unwind_stack_by_address);

/* used by show_backtrace() */
unsigned long unwind_stack(struct task_struct *task, unsigned long *sp,
			   unsigned long pc, unsigned long *ra)
{
	unsigned long stack_page = 0;
	int cpu;

	for_each_possible_cpu(cpu) {
		if (on_irq_stack(cpu, *sp)) {
			stack_page = (unsigned long)irq_stack[cpu];
			break;
		}
	}

	if (!stack_page)
		stack_page = (unsigned long)task_stack_page(task);

	return unwind_stack_by_address(stack_page, sp, pc, ra);
}
#endif

/*
 * get_wchan - a maintenance nightmare^W^Wpain in the ass ...
 */
unsigned long get_wchan(struct task_struct *task)
{
	unsigned long pc = 0;
#ifdef CONFIG_KALLSYMS
	unsigned long sp;
	unsigned long ra = 0;
#endif

	if (!task || task == current || task->state == TASK_RUNNING)
		goto out;
	if (!task_stack_page(task))
		goto out;

	pc = thread_saved_pc(task);

#ifdef CONFIG_KALLSYMS
	sp = task->thread.reg29 + schedule_mfi.frame_size;

	while (in_sched_functions(pc))
		pc = unwind_stack(task, &sp, pc, &ra);
#endif

out:
	return pc;
}

unsigned long mips_stack_top(void)
{
	unsigned long top = TASK_SIZE & PAGE_MASK;

	/* One page for branch delay slot "emulation" */
	top -= PAGE_SIZE;

	/* Space for the VDSO, data page & GIC user page */
	top -= PAGE_ALIGN(current->thread.abi->vdso->size);
	top -= PAGE_SIZE;
	top -= mips_gic_present() ? PAGE_SIZE : 0;

	/* Space for cache colour alignment */
	if (cpu_has_dc_aliases)
		top -= shm_align_mask + 1;

	/* Space to randomize the VDSO base */
	if (current->flags & PF_RANDOMIZE)
		top -= VDSO_RANDOMIZE_SIZE;

	return top;
}

/*
 * Don't forget that the stack pointer must be aligned on a 8 bytes
 * boundary for 32-bits ABI and 16 bytes for 64-bits ABI.
 */
unsigned long arch_align_stack(unsigned long sp)
{
	if (!(current->personality & ADDR_NO_RANDOMIZE) && randomize_va_space)
		sp -= get_random_int() & ~PAGE_MASK;

	return sp & ALMASK;
}

static DEFINE_PER_CPU(call_single_data_t, backtrace_csd);
static struct cpumask backtrace_csd_busy;

static void handle_backtrace(void *info)
{
	nmi_cpu_backtrace(get_irq_regs());
	cpumask_clear_cpu(smp_processor_id(), &backtrace_csd_busy);
}

static void raise_backtrace(cpumask_t *mask)
{
	call_single_data_t *csd;
	int cpu;

	for_each_cpu(cpu, mask) {
		/*
		 * If we previously sent an IPI to the target CPU & it hasn't
		 * cleared its bit in the busy cpumask then it didn't handle
		 * our previous IPI & it's not safe for us to reuse the
		 * call_single_data_t.
		 */
		if (cpumask_test_and_set_cpu(cpu, &backtrace_csd_busy)) {
			pr_warn("Unable to send backtrace IPI to CPU%u - perhaps it hung?\n",
				cpu);
			continue;
		}

		csd = &per_cpu(backtrace_csd, cpu);
		csd->func = handle_backtrace;
		smp_call_function_single_async(cpu, csd);
	}
}

void arch_trigger_cpumask_backtrace(const cpumask_t *mask, bool exclude_self)
{
	nmi_trigger_cpumask_backtrace(mask, exclude_self, raise_backtrace);
}

int mips_get_process_fp_mode(struct task_struct *task)
{
	int value = 0;

	if (!test_tsk_thread_flag(task, TIF_32BIT_FPREGS))
		value |= PR_FP_MODE_FR;
	if (test_tsk_thread_flag(task, TIF_HYBRID_FPREGS))
		value |= PR_FP_MODE_FRE;

	return value;
}

static long prepare_for_fp_mode_switch(void *unused)
{
	/*
	 * This is icky, but we use this to simply ensure that all CPUs have
	 * context switched, regardless of whether they were previously running
	 * kernel or user code. This ensures that no CPU that a mode-switching
	 * program may execute on keeps its FPU enabled (& in the old mode)
	 * throughout the mode switch.
	 */
	return 0;
}

int mips_set_process_fp_mode(struct task_struct *task, unsigned int value)
{
	const unsigned int known_bits = PR_FP_MODE_FR | PR_FP_MODE_FRE;
	struct task_struct *t;
	struct cpumask process_cpus;
	int cpu;

	/* If nothing to change, return right away, successfully.  */
	if (value == mips_get_process_fp_mode(task))
		return 0;

	/* Only accept a mode change if 64-bit FP enabled for o32.  */
	if (!IS_ENABLED(CONFIG_MIPS_O32_FP64_SUPPORT))
		return -EOPNOTSUPP;

	/* And only for o32 tasks.  */
	if (IS_ENABLED(CONFIG_64BIT) && !test_thread_flag(TIF_32BIT_REGS))
		return -EOPNOTSUPP;

	/* Check the value is valid */
	if (value & ~known_bits)
		return -EOPNOTSUPP;

	/* Setting FRE without FR is not supported.  */
	if ((value & (PR_FP_MODE_FR | PR_FP_MODE_FRE)) == PR_FP_MODE_FRE)
		return -EOPNOTSUPP;

	/* Avoid inadvertently triggering emulation */
	if ((value & PR_FP_MODE_FR) && raw_cpu_has_fpu &&
	    !(raw_current_cpu_data.fpu_id & MIPS_FPIR_F64))
		return -EOPNOTSUPP;
	if ((value & PR_FP_MODE_FRE) && raw_cpu_has_fpu && !cpu_has_fre)
		return -EOPNOTSUPP;

	/* FR = 0 not supported in MIPS R6 */
	if (!(value & PR_FP_MODE_FR) && raw_cpu_has_fpu && cpu_has_mips_r6)
		return -EOPNOTSUPP;

	/* Indicate the new FP mode in each thread */
	for_each_thread(task, t) {
		/* Update desired FP register width */
		if (value & PR_FP_MODE_FR) {
			clear_tsk_thread_flag(t, TIF_32BIT_FPREGS);
		} else {
			set_tsk_thread_flag(t, TIF_32BIT_FPREGS);
			clear_tsk_thread_flag(t, TIF_MSA_CTX_LIVE);
		}

		/* Update desired FP single layout */
		if (value & PR_FP_MODE_FRE)
			set_tsk_thread_flag(t, TIF_HYBRID_FPREGS);
		else
			clear_tsk_thread_flag(t, TIF_HYBRID_FPREGS);
	}

	/*
	 * We need to ensure that all threads in the process have switched mode
	 * before returning, in order to allow userland to not worry about
	 * races. We can do this by forcing all CPUs that any thread in the
	 * process may be running on to schedule something else - in this case
	 * prepare_for_fp_mode_switch().
	 *
	 * We begin by generating a mask of all CPUs that any thread in the
	 * process may be running on.
	 */
	cpumask_clear(&process_cpus);
	for_each_thread(task, t)
		cpumask_set_cpu(task_cpu(t), &process_cpus);

	/*
	 * Now we schedule prepare_for_fp_mode_switch() on each of those CPUs.
	 *
	 * The CPUs may have rescheduled already since we switched mode or
	 * generated the cpumask, but that doesn't matter. If the task in this
	 * process is scheduled out then our scheduling
	 * prepare_for_fp_mode_switch() will simply be redundant. If it's
	 * scheduled in then it will already have picked up the new FP mode
	 * whilst doing so.
	 */
	get_online_cpus();
	for_each_cpu_and(cpu, &process_cpus, cpu_online_mask)
		work_on_cpu(cpu, prepare_for_fp_mode_switch, NULL);
	put_online_cpus();

	return 0;
}

#if defined(CONFIG_32BIT) || defined(CONFIG_MIPS32_O32)
void mips_dump_regs32(u32 *uregs, const struct pt_regs *regs)
{
	unsigned int i;

	for (i = MIPS32_EF_R1; i <= MIPS32_EF_R31; i++) {
		/* k0/k1 are copied as zero. */
		if (i == MIPS32_EF_R26 || i == MIPS32_EF_R27)
			uregs[i] = 0;
		else
			uregs[i] = regs->regs[i - MIPS32_EF_R0];
	}

	uregs[MIPS32_EF_LO] = regs->lo;
	uregs[MIPS32_EF_HI] = regs->hi;
	uregs[MIPS32_EF_CP0_EPC] = regs->cp0_epc;
	uregs[MIPS32_EF_CP0_BADVADDR] = regs->cp0_badvaddr;
	uregs[MIPS32_EF_CP0_STATUS] = regs->cp0_status;
	uregs[MIPS32_EF_CP0_CAUSE] = regs->cp0_cause;
}
#endif /* CONFIG_32BIT || CONFIG_MIPS32_O32 */

#ifdef CONFIG_64BIT
void mips_dump_regs64(u64 *uregs, const struct pt_regs *regs)
{
	unsigned int i;

	for (i = MIPS64_EF_R1; i <= MIPS64_EF_R31; i++) {
		/* k0/k1 are copied as zero. */
		if (i == MIPS64_EF_R26 || i == MIPS64_EF_R27)
			uregs[i] = 0;
		else
			uregs[i] = regs->regs[i - MIPS64_EF_R0];
	}

	uregs[MIPS64_EF_LO] = regs->lo;
	uregs[MIPS64_EF_HI] = regs->hi;
	uregs[MIPS64_EF_CP0_EPC] = regs->cp0_epc;
	uregs[MIPS64_EF_CP0_BADVADDR] = regs->cp0_badvaddr;
	uregs[MIPS64_EF_CP0_STATUS] = regs->cp0_status;
	uregs[MIPS64_EF_CP0_CAUSE] = regs->cp0_cause;
}
#endif /* CONFIG_64BIT */