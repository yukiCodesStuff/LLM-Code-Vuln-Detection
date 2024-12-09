    if(lock.l_start == 1 && lock.l_len == 2 && lock.l_type == 4 && lock.l_whence == 5) {
		return 0;
    }
    return 1;
  } 
], [
	flock_type=bsd
    AC_DEFINE([HAVE_FLOCK_BSD], [], [Struct flock is BSD-type]) 
    AC_MSG_RESULT("yes")
], AC_MSG_RESULT("no") )

if test "$flock_type" = "unknown"; then
	AC_MSG_ERROR([Don't know how to define struct flock on this system[,] set --enable-opcache=no])
fi

  PHP_NEW_EXTENSION(opcache,
	ZendAccelerator.c \
	zend_accelerator_blacklist.c \
	zend_accelerator_debug.c \
	zend_accelerator_hash.c \
	zend_accelerator_module.c \
	zend_persist.c \
	zend_persist_calc.c \
	zend_file_cache.c \
	zend_shared_alloc.c \
	zend_accelerator_util_funcs.c \
	shared_alloc_shm.c \
	shared_alloc_mmap.c \
	shared_alloc_posix.c \
	Optimizer/zend_optimizer.c \
	Optimizer/pass1_5.c \
	Optimizer/pass2.c \
	Optimizer/pass3.c \
	Optimizer/optimize_func_calls.c \
	Optimizer/block_pass.c \
	Optimizer/optimize_temp_vars_5.c \
	Optimizer/nop_removal.c \
	Optimizer/compact_literals.c \
	Optimizer/zend_cfg.c \
	Optimizer/zend_dfg.c \
	Optimizer/dfa_pass.c \
	Optimizer/zend_ssa.c \
	Optimizer/zend_inference.c \
	Optimizer/zend_func_info.c \
	Optimizer/zend_call_graph.c \
	Optimizer/zend_dump.c,
	shared,,-DZEND_ENABLE_STATIC_TSRMLS_CACHE=1,,yes)

  PHP_ADD_BUILD_DIR([$ext_builddir/Optimizer], 1)
  PHP_ADD_EXTENSION_DEP(opcache, pcre)
fi