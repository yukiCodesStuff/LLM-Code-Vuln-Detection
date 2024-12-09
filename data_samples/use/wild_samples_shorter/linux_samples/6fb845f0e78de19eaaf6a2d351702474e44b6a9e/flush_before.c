		__clean_dcache_area_pou(kaddr, len);
		__flush_icache_all();
	} else {
		flush_icache_range(addr, addr + len);
	}
}

static void flush_ptrace_access(struct vm_area_struct *vma, struct page *page,