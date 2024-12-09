	b	1f;						\
END_FTR_SECTION(0, 1);						\
	ld	r12,opal_tracepoint_refcount@toc(r2);		\
	cmpdi	r12,0;						\
	bne-	LABEL;						\
1:
