{
	/* prepare control code if any */

	/*
        * If the kexec boot is the normal one, need to shutdown other cpus
        * into our wait loop and quiesce interrupts.
        * Otherwise, in the case of crashed mode (crashing_cpu >= 0),
        * stopping other CPUs and collecting their pt_regs is done before
        * using debugger IPI.
        */

	if (!kdump_in_progress())
		kexec_prepare_cpus();

	pr_debug("kexec: Starting switchover sequence.\n");

	/* switch to a staticly allocated stack.  Based on irq stack code.
	 * We setup preempt_count to avoid using VMX in memcpy.
	 * XXX: the task struct will likely be invalid once we do the copy!
	 */
	kexec_stack.thread_info.task = current_thread_info()->task;
	kexec_stack.thread_info.flags = 0;
	kexec_stack.thread_info.preempt_count = HARDIRQ_OFFSET;
	kexec_stack.thread_info.cpu = current_thread_info()->cpu;

	/* We need a static PACA, too; copy this CPU's PACA over and switch to
	 * it.  Also poison per_cpu_offset to catch anyone using non-static
	 * data.
	 */
	memcpy(&kexec_paca, get_paca(), sizeof(struct paca_struct));
	kexec_paca.data_offset = 0xedeaddeadeeeeeeeUL;
	paca = (struct paca_struct *)RELOC_HIDE(&kexec_paca, 0) -
		kexec_paca.paca_index;
	setup_paca(&kexec_paca);

	/* XXX: If anyone does 'dynamic lppacas' this will also need to be
	 * switched to a static version!
	 */

	/* Some things are best done in assembly.  Finding globals with
	 * a toc is easier in C, so pass in what we can.
	 */
	kexec_sequence(&kexec_stack, image->start, image,
			page_address(image->control_code_page),
			ppc_md.hpte_clear_all);
	/* NOTREACHED */
}

/* Values we need to export to the second kernel via the device tree. */
static unsigned long htab_base;
static unsigned long htab_size;

static struct property htab_base_prop = {
	.name = "linux,htab-base",
	.length = sizeof(unsigned long),
	.value = &htab_base,
};

static struct property htab_size_prop = {
	.name = "linux,htab-size",
	.length = sizeof(unsigned long),
	.value = &htab_size,
};

static int __init export_htab_values(void)
{
	struct device_node *node;
	struct property *prop;

	/* On machines with no htab htab_address is NULL */
	if (!htab_address)
		return -ENODEV;

	node = of_find_node_by_path("/chosen");
	if (!node)
		return -ENODEV;

	/* remove any stale propertys so ours can be found */
	prop = of_find_property(node, htab_base_prop.name, NULL);
	if (prop)
		of_remove_property(node, prop);
	prop = of_find_property(node, htab_size_prop.name, NULL);
	if (prop)
		of_remove_property(node, prop);

	htab_base = cpu_to_be64(__pa(htab_address));
	of_add_property(node, &htab_base_prop);
	htab_size = cpu_to_be64(htab_size_bytes);
	of_add_property(node, &htab_size_prop);

	of_node_put(node);
	return 0;
}
late_initcall(export_htab_values);