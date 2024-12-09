{
	return ucall_norets(UV_UNSHARE_ALL_PAGES);
}

static inline int uv_page_in(u64 lpid, u64 src_ra, u64 dst_gpa, u64 flags,
			     u64 page_shift)
{
	return ucall_norets(UV_PAGE_IN, lpid, src_ra, dst_gpa, flags,
			    page_shift);
}

static inline int uv_page_out(u64 lpid, u64 dst_ra, u64 src_gpa, u64 flags,
			      u64 page_shift)
{
	return ucall_norets(UV_PAGE_OUT, lpid, dst_ra, src_gpa, flags,
			    page_shift);
}

static inline int uv_register_mem_slot(u64 lpid, u64 start_gpa, u64 size,
				       u64 flags, u64 slotid)
{
	return ucall_norets(UV_REGISTER_MEM_SLOT, lpid, start_gpa,
			    size, flags, slotid);
}

static inline int uv_unregister_mem_slot(u64 lpid, u64 slotid)
{
	return ucall_norets(UV_UNREGISTER_MEM_SLOT, lpid, slotid);
}

static inline int uv_page_inval(u64 lpid, u64 gpa, u64 page_shift)
{
	return ucall_norets(UV_PAGE_INVAL, lpid, gpa, page_shift);
}

static inline int uv_svm_terminate(u64 lpid)
{
	return ucall_norets(UV_SVM_TERMINATE, lpid);
}

#endif	/* _ASM_POWERPC_ULTRAVISOR_H */