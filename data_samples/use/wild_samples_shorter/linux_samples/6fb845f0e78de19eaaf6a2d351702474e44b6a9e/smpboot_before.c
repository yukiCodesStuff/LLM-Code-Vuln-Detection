
	while ((dn = of_find_node_by_type(dn, "cpu"))) {
		hart = riscv_of_processor_hartid(dn);
		if (hart < 0) {
			of_node_put(dn);
			continue;
		}

		if (hart == cpuid_to_hartid_map(0)) {
			BUG_ON(found_boot_cpu);
			found_boot_cpu = 1;
			of_node_put(dn);
			continue;
		}

		cpuid_to_hartid_map(cpuid) = hart;
		set_cpu_possible(cpuid, true);
		set_cpu_present(cpuid, true);
		cpuid++;
		of_node_put(dn);
	}

	BUG_ON(!found_boot_cpu);
}