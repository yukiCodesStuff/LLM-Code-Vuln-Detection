{
	uint64_t mem_in_bytes = 0;
	pg_data_t *pgdat;
	int zone_type;

	*avail_size -= sizeof(struct crat_subtype_memory);
	if (*avail_size < 0)
		return -ENOMEM;

	memset(sub_type_hdr, 0, sizeof(struct crat_subtype_memory));

	/* Fill in subtype header data */
	sub_type_hdr->type = CRAT_SUBTYPE_MEMORY_AFFINITY;
	sub_type_hdr->length = sizeof(struct crat_subtype_memory);
	sub_type_hdr->flags = CRAT_SUBTYPE_FLAGS_ENABLED;

	/* Fill in Memory Subunit data */

	/* Unlike si_meminfo, si_meminfo_node is not exported. So
	 * the following lines are duplicated from si_meminfo_node
	 * function
	 */
	pgdat = NODE_DATA(numa_node_id);
	for (zone_type = 0; zone_type < MAX_NR_ZONES; zone_type++)
		mem_in_bytes += zone_managed_pages(&pgdat->node_zones[zone_type]);
	mem_in_bytes <<= PAGE_SHIFT;

	sub_type_hdr->length_low = lower_32_bits(mem_in_bytes);
	sub_type_hdr->length_high = upper_32_bits(mem_in_bytes);
	sub_type_hdr->proximity_domain = proximity_domain;

	return 0;
}

#ifdef CONFIG_X86_64
static int kfd_fill_iolink_info_for_cpu(int numa_node_id, int *avail_size,
				uint32_t *num_entries,
				struct crat_subtype_iolink *sub_type_hdr)
{
	int nid;
	struct cpuinfo_x86 *c = &cpu_data(0);
	uint8_t link_type;

	if (c->x86_vendor == X86_VENDOR_AMD)
		link_type = CRAT_IOLINK_TYPE_HYPERTRANSPORT;
	else
		link_type = CRAT_IOLINK_TYPE_QPI_1_1;

	*num_entries = 0;

	/* Create IO links from this node to other CPU nodes */
	for_each_online_node(nid) {
		if (nid == numa_node_id) /* node itself */
			continue;

		*avail_size -= sizeof(struct crat_subtype_iolink);
		if (*avail_size < 0)
			return -ENOMEM;

		memset(sub_type_hdr, 0, sizeof(struct crat_subtype_iolink));

		/* Fill in subtype header data */
		sub_type_hdr->type = CRAT_SUBTYPE_IOLINK_AFFINITY;
		sub_type_hdr->length = sizeof(struct crat_subtype_iolink);
		sub_type_hdr->flags = CRAT_SUBTYPE_FLAGS_ENABLED;

		/* Fill in IO link data */
		sub_type_hdr->proximity_domain_from = numa_node_id;
		sub_type_hdr->proximity_domain_to = nid;
		sub_type_hdr->io_interface_type = link_type;

		(*num_entries)++;
		sub_type_hdr++;
	}