{
	struct bndcsr *bndcsr;

	if (!cpu_feature_enabled(X86_FEATURE_MPX))
		return MPX_INVALID_BOUNDS_DIR;

	/*
	 * 32-bit binaries on 64-bit kernels are currently
	 * unsupported.
	 */
	if (IS_ENABLED(CONFIG_X86_64) && test_thread_flag(TIF_IA32))
		return MPX_INVALID_BOUNDS_DIR;
	/*
	 * The bounds directory pointer is stored in a register
	 * only accessible if we first do an xsave.
	 */
	fpu_save_init(&tsk->thread.fpu);
	bndcsr = get_xsave_addr(&tsk->thread.fpu.state->xsave, XSTATE_BNDCSR);
	if (!bndcsr)
		return MPX_INVALID_BOUNDS_DIR;

	/*
	 * Make sure the register looks valid by checking the
	 * enable bit.
	 */
	if (!(bndcsr->bndcfgu & MPX_BNDCFG_ENABLE_FLAG))
		return MPX_INVALID_BOUNDS_DIR;

	/*
	 * Lastly, mask off the low bits used for configuration
	 * flags, and return the address of the bounds table.
	 */
	return (void __user *)(unsigned long)
		(bndcsr->bndcfgu & MPX_BNDCFG_ADDR_MASK);
}

int mpx_enable_management(struct task_struct *tsk)
{
	void __user *bd_base = MPX_INVALID_BOUNDS_DIR;
	struct mm_struct *mm = tsk->mm;
	int ret = 0;

	/*
	 * runtime in the userspace will be responsible for allocation of
	 * the bounds directory. Then, it will save the base of the bounds
	 * directory into XSAVE/XRSTOR Save Area and enable MPX through
	 * XRSTOR instruction.
	 *
	 * fpu_xsave() is expected to be very expensive. Storing the bounds
	 * directory here means that we do not have to do xsave in the unmap
	 * path; we can just use mm->bd_addr instead.
	 */
	bd_base = task_get_bounds_dir(tsk);
	down_write(&mm->mmap_sem);
	mm->bd_addr = bd_base;
	if (mm->bd_addr == MPX_INVALID_BOUNDS_DIR)
		ret = -ENXIO;

	up_write(&mm->mmap_sem);
	return ret;
}

int mpx_disable_management(struct task_struct *tsk)
{
	struct mm_struct *mm = current->mm;

	if (!cpu_feature_enabled(X86_FEATURE_MPX))
		return -ENXIO;

	down_write(&mm->mmap_sem);
	mm->bd_addr = MPX_INVALID_BOUNDS_DIR;
	up_write(&mm->mmap_sem);
	return 0;
}

/*
 * With 32-bit mode, MPX_BT_SIZE_BYTES is 4MB, and the size of each
 * bounds table is 16KB. With 64-bit mode, MPX_BT_SIZE_BYTES is 2GB,
 * and the size of each bounds table is 4MB.
 */
static int allocate_bt(long __user *bd_entry)
{
	unsigned long expected_old_val = 0;
	unsigned long actual_old_val = 0;
	unsigned long bt_addr;
	int ret = 0;

	/*
	 * Carve the virtual space out of userspace for the new
	 * bounds table:
	 */
	bt_addr = mpx_mmap(MPX_BT_SIZE_BYTES);
	if (IS_ERR((void *)bt_addr))
		return PTR_ERR((void *)bt_addr);
	/*
	 * Set the valid flag (kinda like _PAGE_PRESENT in a pte)
	 */
	bt_addr = bt_addr | MPX_BD_ENTRY_VALID_FLAG;

	/*
	 * Go poke the address of the new bounds table in to the
	 * bounds directory entry out in userspace memory.  Note:
	 * we may race with another CPU instantiating the same table.
	 * In that case the cmpxchg will see an unexpected
	 * 'actual_old_val'.
	 *
	 * This can fault, but that's OK because we do not hold
	 * mmap_sem at this point, unlike some of the other part
	 * of the MPX code that have to pagefault_disable().
	 */
	ret = user_atomic_cmpxchg_inatomic(&actual_old_val, bd_entry,
					   expected_old_val, bt_addr);
	if (ret)
		goto out_unmap;

	/*
	 * The user_atomic_cmpxchg_inatomic() will only return nonzero
	 * for faults, *not* if the cmpxchg itself fails.  Now we must
	 * verify that the cmpxchg itself completed successfully.
	 */
	/*
	 * We expected an empty 'expected_old_val', but instead found
	 * an apparently valid entry.  Assume we raced with another
	 * thread to instantiate this table and desclare succecss.
	 */
	if (actual_old_val & MPX_BD_ENTRY_VALID_FLAG) {
		ret = 0;
		goto out_unmap;
	}