static struct ptdump_info efi_ptdump_info = {
	.mm		= &efi_mm,
	.markers	= (struct addr_marker[]){
		{ 0,		"UEFI runtime start" },
		{ DEFAULT_MAP_WINDOW_64, "UEFI runtime end" }
	},
	.base_addr	= 0,
};
