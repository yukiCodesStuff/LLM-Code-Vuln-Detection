	if (icache_is_aliasing()) {
		__clean_dcache_area_pou(kaddr, len);
		__flush_icache_all();
	} else {
		flush_icache_range(addr, addr + len);
	}