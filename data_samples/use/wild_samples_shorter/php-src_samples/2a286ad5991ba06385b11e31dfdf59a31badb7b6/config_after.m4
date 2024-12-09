	Optimizer/zend_inference.c \
	Optimizer/zend_func_info.c \
	Optimizer/zend_call_graph.c \
	Optimizer/sccp.c \
	Optimizer/scdf.c \
	Optimizer/dce.c \
	Optimizer/compact_vars.c \
	Optimizer/zend_dump.c,
	shared,,-DZEND_ENABLE_STATIC_TSRMLS_CACHE=1,,yes)

  PHP_ADD_BUILD_DIR([$ext_builddir/Optimizer], 1)