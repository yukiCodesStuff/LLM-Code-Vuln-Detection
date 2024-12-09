{
	phys_addr_t addr = (phys_addr_t)pfn << PAGE_SHIFT;

	return phys_addr_valid(addr + count - 1);
}