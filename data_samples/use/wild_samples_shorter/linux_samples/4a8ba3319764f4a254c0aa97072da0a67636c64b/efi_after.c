
	/* boot time idmap_pg_dir is incomplete, so fill in missing parts */
	efi_setup_idmap();
	early_memunmap(memmap.map, memmap.map_end - memmap.map);
}

static int __init remap_region(efi_memory_desc_t *md, void **new)
{
	}

	mapsize = memmap.map_end - memmap.map;

	if (efi_runtime_disabled()) {
		pr_info("EFI runtime services will be disabled.\n");
		return -1;