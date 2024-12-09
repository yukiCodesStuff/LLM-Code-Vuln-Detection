static inline long long php_date_llabs( long long i ) { return i >= 0 ? i : -i; }
#endif

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_date, 0, 0, 1)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, timestamp)

const zend_function_entry date_funcs_period[] = {
	PHP_ME(DatePeriod,                __construct,                 arginfo_date_period_construct, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_FE_END
};

static char* guess_timezone(const timelib_tzdb *tzdb TSRMLS_DC);
static int date_object_compare_date(zval *d1, zval *d2 TSRMLS_DC);
static HashTable *date_object_get_properties(zval *object TSRMLS_DC);
static HashTable *date_object_get_properties_interval(zval *object TSRMLS_DC);

zval *date_interval_read_property(zval *object, zval *member, int type TSRMLS_DC);
void date_interval_write_property(zval *object, zval *member, zval *value TSRMLS_DC);

/* {{{ Module struct */
zend_module_entry date_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	zend_class_implements(date_ce_period TSRMLS_CC, 1, zend_ce_traversable);
	memcpy(&date_object_handlers_period, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	date_object_handlers_period.clone_obj = date_object_clone_period;

#define REGISTER_PERIOD_CLASS_CONST_STRING(const_name, value) \
	zend_declare_class_constant_long(date_ce_period, const_name, sizeof(const_name)-1, value TSRMLS_CC);

	zval *zv;
	php_interval_obj     *intervalobj;


	intervalobj = (php_interval_obj *) zend_object_store_get_object(object TSRMLS_CC);

	props = zend_std_get_properties(object TSRMLS_CC);


#define PHP_DATE_INTERVAL_ADD_PROPERTY(n,f) \
	MAKE_STD_ZVAL(zv); \
	ZVAL_LONG(zv, intervalobj->diff->f); \
	zend_hash_update(props, n, strlen(n) + 1, &zv, sizeof(zval), NULL);

	PHP_DATE_INTERVAL_ADD_PROPERTY("y", y);
	PHP_DATE_INTERVAL_ADD_PROPERTY("m", m);
	PHP_DATE_INTERVAL_ADD_PROPERTY("h", h);
	PHP_DATE_INTERVAL_ADD_PROPERTY("i", i);
	PHP_DATE_INTERVAL_ADD_PROPERTY("s", s);
	PHP_DATE_INTERVAL_ADD_PROPERTY("invert", invert);
	if (intervalobj->diff->days != -99999) {
		PHP_DATE_INTERVAL_ADD_PROPERTY("days", days);
	} else {
		ZVAL_FALSE(zv);
		zend_hash_update(props, "days", 5, &zv, sizeof(zval), NULL);
	}

	return props;
}

	object_init_ex(object, pce);
	Z_SET_REFCOUNT_P(object, 1);
	Z_UNSET_ISREF_P(object);
	return object;
}

/* Helper function used to store the latest found warnings and errors while
}
/* }}} */

static long php_date_long_from_hash_element(HashTable *myht, char *element, size_t size)
{
	zval            **z_arg = NULL;

	if (zend_hash_find(myht, element, size + 1, (void**) &z_arg) == SUCCESS) {
		convert_to_long(*z_arg);
		return Z_LVAL_PP(z_arg);
	} else {
		return -1;
	}
}

static int php_date_interval_initialize_from_hash(zval **return_value, php_interval_obj **intobj, HashTable *myht TSRMLS_DC)
{
	(*intobj)->diff = timelib_rel_time_ctor();

	(*intobj)->diff->y = php_date_long_from_hash_element(myht, "y", 1);
	(*intobj)->diff->m = php_date_long_from_hash_element(myht, "m", 1);
	(*intobj)->diff->d = php_date_long_from_hash_element(myht, "d", 1);
	(*intobj)->diff->h = php_date_long_from_hash_element(myht, "h", 1);
	(*intobj)->diff->i = php_date_long_from_hash_element(myht, "i", 1);
	(*intobj)->diff->s = php_date_long_from_hash_element(myht, "s", 1);
	(*intobj)->diff->invert = php_date_long_from_hash_element(myht, "invert", 6);
	(*intobj)->diff->days = php_date_long_from_hash_element(myht, "days", 4);
	(*intobj)->initialized = 1;

	return 0;
}
	timelib_time_dtor(t2);
}
/* }}} */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4