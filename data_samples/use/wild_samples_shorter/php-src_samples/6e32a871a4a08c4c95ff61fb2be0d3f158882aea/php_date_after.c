static inline long long php_date_llabs( long long i ) { return i >= 0 ? i : -i; }
#endif

#ifdef PHP_WIN32
#define DATE_I64_BUF_LEN 65
# define DATE_I64A(i, s, len) _i64toa_s(i, s, len, 10)
# define DATE_A64I(i, s) i = _atoi64(s)
#else
#define DATE_I64_BUF_LEN 65
# define DATE_I64A(i, s, len) \
	do { \
		int st = snprintf(s, len, "%lld", i); \
		s[st] = '\0'; \
	} while (0);
# define DATE_A64I(i, s) i = atoll(s)
#endif

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_date, 0, 0, 1)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, timestamp)

const zend_function_entry date_funcs_period[] = {
	PHP_ME(DatePeriod,                __construct,                 arginfo_date_period_construct, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(DatePeriod,                __wakeup,                    NULL, ZEND_ACC_PUBLIC)
	PHP_ME(DatePeriod,                __set_state,                 NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

static char* guess_timezone(const timelib_tzdb *tzdb TSRMLS_DC);
static int date_object_compare_date(zval *d1, zval *d2 TSRMLS_DC);
static HashTable *date_object_get_properties(zval *object TSRMLS_DC);
static HashTable *date_object_get_properties_interval(zval *object TSRMLS_DC);
static HashTable *date_object_get_properties_period(zval *object TSRMLS_DC);

zval *date_interval_read_property(zval *object, zval *member, int type TSRMLS_DC);
void date_interval_write_property(zval *object, zval *member, zval *value TSRMLS_DC);
static zval *date_period_read_property(zval *object, zval *member, int type TSRMLS_DC);
static void date_period_write_property(zval *object, zval *member, zval *value TSRMLS_DC);

/* {{{ Module struct */
zend_module_entry date_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	zend_class_implements(date_ce_period TSRMLS_CC, 1, zend_ce_traversable);
	memcpy(&date_object_handlers_period, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	date_object_handlers_period.clone_obj = date_object_clone_period;
	date_object_handlers_period.get_properties = date_object_get_properties_period;
	date_object_handlers_period.get_property_ptr_ptr = NULL;
	date_object_handlers_period.read_property = date_period_read_property;
	date_object_handlers_period.write_property = date_period_write_property;

#define REGISTER_PERIOD_CLASS_CONST_STRING(const_name, value) \
	zend_declare_class_constant_long(date_ce_period, const_name, sizeof(const_name)-1, value TSRMLS_CC);

	zval *zv;
	php_interval_obj     *intervalobj;

	intervalobj = (php_interval_obj *) zend_object_store_get_object(object TSRMLS_CC);

	props = zend_std_get_properties(object TSRMLS_CC);


#define PHP_DATE_INTERVAL_ADD_PROPERTY(n,f) \
	MAKE_STD_ZVAL(zv); \
	ZVAL_LONG(zv, (long)intervalobj->diff->f); \
	zend_hash_update(props, n, strlen(n) + 1, &zv, sizeof(zval), NULL);

	PHP_DATE_INTERVAL_ADD_PROPERTY("y", y);
	PHP_DATE_INTERVAL_ADD_PROPERTY("m", m);
	PHP_DATE_INTERVAL_ADD_PROPERTY("h", h);
	PHP_DATE_INTERVAL_ADD_PROPERTY("i", i);
	PHP_DATE_INTERVAL_ADD_PROPERTY("s", s);
	PHP_DATE_INTERVAL_ADD_PROPERTY("weekday", weekday);
	PHP_DATE_INTERVAL_ADD_PROPERTY("weekday_behavior", weekday_behavior);
	PHP_DATE_INTERVAL_ADD_PROPERTY("first_last_day_of", first_last_day_of);
	PHP_DATE_INTERVAL_ADD_PROPERTY("invert", invert);
	if (intervalobj->diff->days != -99999) {
		PHP_DATE_INTERVAL_ADD_PROPERTY("days", days);
	} else {
		ZVAL_FALSE(zv);
		zend_hash_update(props, "days", 5, &zv, sizeof(zval), NULL);
	}
	PHP_DATE_INTERVAL_ADD_PROPERTY("special_type", special.type);
	PHP_DATE_INTERVAL_ADD_PROPERTY("special_amount", special.amount);
	PHP_DATE_INTERVAL_ADD_PROPERTY("have_weekday_relative", have_weekday_relative);
	PHP_DATE_INTERVAL_ADD_PROPERTY("have_special_relative", have_special_relative);

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


static int php_date_interval_initialize_from_hash(zval **return_value, php_interval_obj **intobj, HashTable *myht TSRMLS_DC)
{
	(*intobj)->diff = timelib_rel_time_ctor();

#define PHP_DATE_INTERVAL_READ_PROPERTY(element, member, itype, def) \
	do { \
		zval **z_arg = NULL; \
		if (zend_hash_find(myht, element, strlen(element) + 1, (void**) &z_arg) == SUCCESS) { \
			convert_to_long(*z_arg); \
			(*intobj)->diff->member = (itype)Z_LVAL_PP(z_arg); \
		} else { \
			(*intobj)->diff->member = (itype)def; \
		} \
	} while (0);

#define PHP_DATE_INTERVAL_READ_PROPERTY_I64(element, member) \
	do { \
		zval **z_arg = NULL; \
		if (zend_hash_find(myht, element, strlen(element) + 1, (void**) &z_arg) == SUCCESS) { \
			convert_to_string(*z_arg); \
			DATE_A64I((*intobj)->diff->member, Z_STRVAL_PP(z_arg)); \
		} else { \
			(*intobj)->diff->member = -1LL; \
		} \
	} while (0);

	PHP_DATE_INTERVAL_READ_PROPERTY("y", y, timelib_sll, -1)
	PHP_DATE_INTERVAL_READ_PROPERTY("m", m, timelib_sll, -1)
	PHP_DATE_INTERVAL_READ_PROPERTY("d", d, timelib_sll, -1)
	PHP_DATE_INTERVAL_READ_PROPERTY("h", h, timelib_sll, -1)
	PHP_DATE_INTERVAL_READ_PROPERTY("i", i, timelib_sll, -1)
	PHP_DATE_INTERVAL_READ_PROPERTY("s", s, timelib_sll, -1)
	PHP_DATE_INTERVAL_READ_PROPERTY("weekday", weekday, int, -1)
	PHP_DATE_INTERVAL_READ_PROPERTY("weekday_behavior", weekday_behavior, int, -1)
	PHP_DATE_INTERVAL_READ_PROPERTY("first_last_day_of", first_last_day_of, int, -1)
	PHP_DATE_INTERVAL_READ_PROPERTY("invert", invert, int, 0);
	PHP_DATE_INTERVAL_READ_PROPERTY_I64("days", days);
	PHP_DATE_INTERVAL_READ_PROPERTY("special_type", special.type, unsigned int, 0);
	PHP_DATE_INTERVAL_READ_PROPERTY_I64("special_amount", special.amount);
	PHP_DATE_INTERVAL_READ_PROPERTY("have_weekday_relative", have_weekday_relative, unsigned int, 0);
	PHP_DATE_INTERVAL_READ_PROPERTY("have_special_relative", have_special_relative, unsigned int, 0);
	(*intobj)->initialized = 1;

	return 0;
}
	timelib_time_dtor(t2);
}
/* }}} */


static HashTable *date_object_get_properties_period(zval *object TSRMLS_DC)
{
	HashTable		*props;
	zval			*zv;
	php_period_obj	*period_obj;

	period_obj = zend_object_store_get_object(object TSRMLS_CC);

	props = zend_std_get_properties(object TSRMLS_CC);

	if (!period_obj->start) {
		return props;
	}

	MAKE_STD_ZVAL(zv);
	if (period_obj->start) {
		php_date_obj *date_obj;
		object_init_ex(zv, date_ce_date);
		date_obj = zend_object_store_get_object(zv TSRMLS_CC);
		date_obj->time = timelib_time_clone(period_obj->start);
	} else {
		ZVAL_NULL(zv);
	}
	zend_hash_update(props, "start", sizeof("start"), &zv, sizeof(zv), NULL);

	MAKE_STD_ZVAL(zv);
	if (period_obj->current) {
		php_date_obj *date_obj;
		object_init_ex(zv, date_ce_date);
		date_obj = zend_object_store_get_object(zv TSRMLS_CC);
		date_obj->time = timelib_time_clone(period_obj->current);
	} else {
		ZVAL_NULL(zv);
	}
	zend_hash_update(props, "current", sizeof("current"), &zv, sizeof(zv), NULL);

	MAKE_STD_ZVAL(zv);
	if (period_obj->end) {
		php_date_obj *date_obj;
		object_init_ex(zv, date_ce_date);
		date_obj = zend_object_store_get_object(zv TSRMLS_CC);
		date_obj->time = timelib_time_clone(period_obj->end);
	} else {
		ZVAL_NULL(zv);
	}
	zend_hash_update(props, "end", sizeof("end"), &zv, sizeof(zv), NULL);

	MAKE_STD_ZVAL(zv);
	if (period_obj->interval) {
		php_interval_obj *interval_obj;
		object_init_ex(zv, date_ce_interval);
		interval_obj = zend_object_store_get_object(zv TSRMLS_CC);
		interval_obj->diff = timelib_rel_time_clone(period_obj->interval);
		interval_obj->initialized = 1;
	} else {
		ZVAL_NULL(zv);
	}
	zend_hash_update(props, "interval", sizeof("interval"), &zv, sizeof(zv), NULL);
	
	/* converted to larger type (int->long); must check when unserializing */
	MAKE_STD_ZVAL(zv);
	ZVAL_LONG(zv, (long) period_obj->recurrences);
	zend_hash_update(props, "recurrences", sizeof("recurrences"), &zv, sizeof(zv), NULL);

	MAKE_STD_ZVAL(zv);
	ZVAL_BOOL(zv, period_obj->include_start_date);
	zend_hash_update(props, "include_start_date", sizeof("include_start_date"), &zv, sizeof(zv), NULL);

	return props;
}

static int php_date_period_initialize_from_hash(php_period_obj *period_obj, HashTable *myht TSRMLS_DC)
{
	zval **ht_entry;

	/* this function does no rollback on error */

	if (zend_hash_find(myht, "start", sizeof("start"), (void**) &ht_entry) == SUCCESS) {
		if (Z_TYPE_PP(ht_entry) == IS_OBJECT && Z_OBJCE_PP(ht_entry) == date_ce_date) {
			php_date_obj *date_obj;
			date_obj = zend_object_store_get_object(*ht_entry TSRMLS_CC);
			period_obj->start = timelib_time_clone(date_obj->time);
		} else if (Z_TYPE_PP(ht_entry) != IS_NULL) {
			return 0;
		}
	} else {
		return 0;
	}

	if (zend_hash_find(myht, "end", sizeof("end"), (void**) &ht_entry) == SUCCESS) {
		if (Z_TYPE_PP(ht_entry) == IS_OBJECT && Z_OBJCE_PP(ht_entry) == date_ce_date) {
			php_date_obj *date_obj;
			date_obj = zend_object_store_get_object(*ht_entry TSRMLS_CC);
			period_obj->end = timelib_time_clone(date_obj->time);
		} else if (Z_TYPE_PP(ht_entry) != IS_NULL) {
			return 0;
		}
	} else {
		return 0;
	}

	if (zend_hash_find(myht, "current", sizeof("current"), (void**) &ht_entry) == SUCCESS) {
		if (Z_TYPE_PP(ht_entry) == IS_OBJECT && Z_OBJCE_PP(ht_entry) == date_ce_date) {
			php_date_obj *date_obj;
			date_obj = zend_object_store_get_object(*ht_entry TSRMLS_CC);
			period_obj->current = timelib_time_clone(date_obj->time);
		} else if (Z_TYPE_PP(ht_entry) != IS_NULL)  {
			return 0;
		}
	} else {
		return 0;
	}

	if (zend_hash_find(myht, "interval", sizeof("interval"), (void**) &ht_entry) == SUCCESS) {
		if (Z_TYPE_PP(ht_entry) == IS_OBJECT && Z_OBJCE_PP(ht_entry) == date_ce_interval) {
			php_interval_obj *interval_obj;
			interval_obj = zend_object_store_get_object(*ht_entry TSRMLS_CC);
			period_obj->interval = timelib_rel_time_clone(interval_obj->diff);
		} else { /* interval is required */
			return 0;
		}
	} else {
		return 0;
	}

	if (zend_hash_find(myht, "recurrences", sizeof("recurrences"), (void**) &ht_entry) == SUCCESS &&
			Z_TYPE_PP(ht_entry) == IS_LONG && Z_LVAL_PP(ht_entry) >= 0 && Z_LVAL_PP(ht_entry) <= INT_MAX) {
		period_obj->recurrences = Z_LVAL_PP(ht_entry);
	} else {
		return 0;
	}

	if (zend_hash_find(myht, "include_start_date", sizeof("include_start_date"), (void**) &ht_entry) == SUCCESS &&
			Z_TYPE_PP(ht_entry) == IS_BOOL) {
		period_obj->include_start_date = Z_BVAL_PP(ht_entry);
	} else {
		return 0;
	}

	period_obj->initialized = 1;
	
	return 1;
}

/* {{{ proto DatePeriod::__set_state()
*/
PHP_METHOD(DatePeriod, __set_state)
{
	php_period_obj   *period_obj;
	zval             *array;
	HashTable        *myht;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &array) == FAILURE) {
		RETURN_FALSE;
	}

	myht = Z_ARRVAL_P(array);
	
	object_init_ex(return_value, date_ce_period);
	period_obj = zend_object_store_get_object(return_value TSRMLS_CC);
	if (!php_date_period_initialize_from_hash(period_obj, myht TSRMLS_CC)) {
		php_error(E_ERROR, "Invalid serialization data for DatePeriod object");
	}
}
/* }}} */

/* {{{ proto DatePeriod::__wakeup()
*/
PHP_METHOD(DatePeriod, __wakeup)
{
	zval             *object = getThis();
	php_period_obj   *period_obj;
	HashTable        *myht;

	period_obj = zend_object_store_get_object(object TSRMLS_CC);

	myht = Z_OBJPROP_P(object);

	if (!php_date_period_initialize_from_hash(period_obj, myht TSRMLS_CC)) {
		php_error(E_ERROR, "Invalid serialization data for DatePeriod object");
	}
}
/* }}} */

/* {{{ date_period_read_property */
static zval *date_period_read_property(zval *object, zval *member, int type TSRMLS_DC)
{
	zval *zv;
	if (type != BP_VAR_IS && type != BP_VAR_R) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Retrieval of DatePeriod properties for modification is unsupported");
	}

	Z_OBJPROP_P(object); /* build properties hash table */

	zv = std_object_handlers.read_property(object, member, type TSRMLS_CC);
	if (Z_TYPE_P(zv) == IS_OBJECT && Z_OBJ_HANDLER_P(zv, clone_obj)) {
		/* defensive copy */
		zend_object_value zov = Z_OBJ_HANDLER_P(zv, clone_obj)(zv TSRMLS_CC);
		MAKE_STD_ZVAL(zv);
		Z_TYPE_P(zv) = IS_OBJECT;
		Z_OBJVAL_P(zv) = zov;
	}

	return zv;
}
/* }}} */

/* {{{ date_period_write_property */
static void date_period_write_property(zval *object, zval *member, zval *value TSRMLS_DC)
{
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Writing to DatePeriod properties is unsupported");
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4