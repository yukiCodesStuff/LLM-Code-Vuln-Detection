	RETURN_LONG(php_stream_write(intern->u.file.stream, str, str_len));
} /* }}} */

/* {{{ proto bool SplFileObject::fstat()
   Stat() on a filehandle */
FileFunction(fstat)
/* }}} */
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_file_object_ftruncate, 0, 0, 1) 
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

	SPL_ME(SplFileObject, fgetss,         arginfo_file_object_fgetss,        ZEND_ACC_PUBLIC)
	SPL_ME(SplFileObject, fscanf,         arginfo_file_object_fscanf,        ZEND_ACC_PUBLIC)
	SPL_ME(SplFileObject, fwrite,         arginfo_file_object_fwrite,        ZEND_ACC_PUBLIC)
	SPL_ME(SplFileObject, fstat,          arginfo_splfileinfo_void,          ZEND_ACC_PUBLIC)
	SPL_ME(SplFileObject, ftruncate,      arginfo_file_object_ftruncate,     ZEND_ACC_PUBLIC)
	SPL_ME(SplFileObject, current,        arginfo_splfileinfo_void,          ZEND_ACC_PUBLIC)
	SPL_ME(SplFileObject, key,            arginfo_splfileinfo_void,          ZEND_ACC_PUBLIC)