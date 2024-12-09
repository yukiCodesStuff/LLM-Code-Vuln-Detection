{
	struct device_node *np;
	const struct of_device_id *match;
	int type;

	/*
	 * The coherency fabric is needed:
	 * - For coherency between processors on Armada XP, so only
	 *   when SMP is enabled.
	 * - For coherency between the processor and I/O devices, but
	 *   this coherency requires many pre-requisites (write
	 *   allocate cache policy, shareable pages, SMP bit set) that
	 *   are only meant in SMP situations.
	 *
	 * Note that this means that on Armada 370, there is currently
	 * no way to use hardware I/O coherency, because even when
	 * CONFIG_SMP is enabled, is_smp() returns false due to the
	 * Armada 370 being a single-core processor. To lift this
	 * limitation, we would have to find a way to make the cache
	 * policy set to write-allocate (on all Armada SoCs), and to
	 * set the shareable attribute in page tables (on all Armada
	 * SoCs except the Armada 370). Unfortunately, such decisions
	 * are taken very early in the kernel boot process, at a point
	 * where we don't know yet on which SoC we are running.

	 */
	if (!is_smp())
		return COHERENCY_FABRIC_TYPE_NONE;

	np = of_find_matching_node_and_match(NULL, of_coherency_table, &match);
	if (!np)
		return COHERENCY_FABRIC_TYPE_NONE;

	type = (int) match->data;

	of_node_put(np);

	return type;
}

int coherency_available(void)
{
	return coherency_type() != COHERENCY_FABRIC_TYPE_NONE;
}

int __init coherency_init(void)
{
	int type = coherency_type();
	struct device_node *np;

	np = of_find_matching_node(NULL, of_coherency_table);

	if (type == COHERENCY_FABRIC_TYPE_ARMADA_370_XP)
		armada_370_coherency_init(np);
	else if (type == COHERENCY_FABRIC_TYPE_ARMADA_375 ||
		 type == COHERENCY_FABRIC_TYPE_ARMADA_380)
		armada_375_380_coherency_init(np);

	of_node_put(np);

	return 0;
}

static int __init coherency_late_init(void)
{
	if (coherency_available())
		bus_register_notifier(&platform_bus_type,
				      &mvebu_hwcc_nb);
	return 0;
}

postcore_initcall(coherency_late_init);

#if IS_ENABLED(CONFIG_PCI)
static int __init coherency_pci_init(void)
{
	if (coherency_available())
		bus_register_notifier(&pci_bus_type,
				       &mvebu_hwcc_pci_nb);
	return 0;
}

arch_initcall(coherency_pci_init);
#endif