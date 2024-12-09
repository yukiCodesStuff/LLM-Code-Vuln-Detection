{
	if (!efi_enabled(EFI_BOOT))
		return;

	/* boot time idmap_pg_dir is incomplete, so fill in missing parts */
	efi_setup_idmap();
	early_memunmap(memmap.map, memmap.map_end - memmap.map);
}

static int __init remap_region(efi_memory_desc_t *md, void **new)
{
	u64 paddr, vaddr, npages, size;

	paddr = md->phys_addr;
	npages = md->num_pages;
	memrange_efi_to_native(&paddr, &npages);
	size = npages << PAGE_SHIFT;

	if (is_normal_ram(md))
		vaddr = (__force u64)ioremap_cache(paddr, size);
	else
		vaddr = (__force u64)ioremap(paddr, size);

	if (!vaddr) {
		pr_err("Unable to remap 0x%llx pages @ %p\n",
		       npages, (void *)paddr);
		return 0;
	}
	if (!efi_enabled(EFI_BOOT)) {
		pr_info("EFI services will not be available.\n");
		return -1;
	}

	mapsize = memmap.map_end - memmap.map;

	if (efi_runtime_disabled()) {
		pr_info("EFI runtime services will be disabled.\n");
		return -1;
	}