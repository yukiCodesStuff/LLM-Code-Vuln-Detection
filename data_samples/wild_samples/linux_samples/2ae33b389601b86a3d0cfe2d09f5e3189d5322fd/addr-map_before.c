	for (i = 0, cs = 0; i < 4; i++) {
		u32 base = readl(ddr_window_cpu_base + DDR_BASE_CS_OFF(i));
		u32 size = readl(ddr_window_cpu_base + DDR_SIZE_CS_OFF(i));

		/*
		 * Chip select enabled?
		 */
		if (size & 1) {
			struct mbus_dram_window *w;

			w = &orion_mbus_dram_info.cs[cs++];
			w->cs_index = i;
			w->mbus_attr = 0xf & ~(1 << i);
			if (cfg->hw_io_coherency)
				w->mbus_attr |= ATTR_HW_COHERENCY;
			w->base = base & 0xffff0000;
			w->size = (size | 0x0000ffff) + 1;
		}