
/* {{{ reference-handling for unserializer: var_* */
#define VAR_ENTRIES_MAX 1024
#define VAR_ENTRIES_DBG 0

typedef struct {
	zval *data[VAR_ENTRIES_MAX];
	long used_slots;
static inline void var_push(php_unserialize_data_t *var_hashx, zval **rval)
{
	var_entries *var_hash = (*var_hashx)->last;
#if VAR_ENTRIES_DBG
	fprintf(stderr, "var_push(%ld): %d\n", var_hash?var_hash->used_slots:-1L, Z_TYPE_PP(rval));
#endif

	if (!var_hash || var_hash->used_slots == VAR_ENTRIES_MAX) {
PHPAPI void var_push_dtor(php_unserialize_data_t *var_hashx, zval **rval)
{
	var_entries *var_hash = (*var_hashx)->last_dtor;
#if VAR_ENTRIES_DBG
	fprintf(stderr, "var_push_dtor(%ld): %d\n", var_hash?var_hash->used_slots:-1L, Z_TYPE_PP(rval));
#endif

	if (!var_hash || var_hash->used_slots == VAR_ENTRIES_MAX) {
	var_hash->data[var_hash->used_slots++] = *rval;
}

PHPAPI void var_push_dtor_no_addref(php_unserialize_data_t *var_hashx, zval **rval)
{
	var_entries *var_hash = (*var_hashx)->last_dtor;
#if VAR_ENTRIES_DBG
	fprintf(stderr, "var_push_dtor_no_addref(%ld): %d (%d)\n", var_hash?var_hash->used_slots:-1L, Z_TYPE_PP(rval), Z_REFCOUNT_PP(rval));
#endif

	if (!var_hash || var_hash->used_slots == VAR_ENTRIES_MAX) {
		var_hash = emalloc(sizeof(var_entries));
		var_hash->used_slots = 0;
		var_hash->next = 0;

		if (!(*var_hashx)->first_dtor) {
			(*var_hashx)->first_dtor = var_hash;
		} else {
			((var_entries *) (*var_hashx)->last_dtor)->next = var_hash;
		}

		(*var_hashx)->last_dtor = var_hash;
	}

	var_hash->data[var_hash->used_slots++] = *rval;
}

PHPAPI void var_replace(php_unserialize_data_t *var_hashx, zval *ozval, zval **nzval)
{
	long i;
	var_entries *var_hash = (*var_hashx)->first;
#if VAR_ENTRIES_DBG
	fprintf(stderr, "var_replace(%ld): %d\n", var_hash?var_hash->used_slots:-1L, Z_TYPE_PP(nzval));
#endif
	
	while (var_hash) {
static int var_access(php_unserialize_data_t *var_hashx, long id, zval ***store)
{
	var_entries *var_hash = (*var_hashx)->first;
#if VAR_ENTRIES_DBG
	fprintf(stderr, "var_access(%ld): %ld\n", var_hash?var_hash->used_slots:-1L, id);
#endif
		
	while (id >= VAR_ENTRIES_MAX && var_hash && var_hash->used_slots == VAR_ENTRIES_MAX) {
	void *next;
	long i;
	var_entries *var_hash = (*var_hashx)->first;
#if VAR_ENTRIES_DBG
	fprintf(stderr, "var_destroy(%ld)\n", var_hash?var_hash->used_slots:-1L);
#endif
	
	while (var_hash) {
#define YYMARKER marker


#line 234 "ext/standard/var_unserializer.re"




	
	

#line 461 "ext/standard/var_unserializer.c"
{
	YYCTYPE yych;
	static const unsigned char yybm[] = {
		  0,   0,   0,   0,   0,   0,   0,   0, 
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy95;
yy3:
#line 812 "ext/standard/var_unserializer.re"
	{ return 0; }
#line 523 "ext/standard/var_unserializer.c"
yy4:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy89;
	goto yy3;
	goto yy3;
yy14:
	++YYCURSOR;
#line 806 "ext/standard/var_unserializer.re"
	{
	/* this is the case where we have less data than planned */
	php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Unexpected end of serialized data");
	return 0; /* not sure if it should be 0 or 1 here? */
}
#line 572 "ext/standard/var_unserializer.c"
yy16:
	yych = *++YYCURSOR;
	goto yy3;
yy17:
	yych = *++YYCURSOR;
	if (yych != '"') goto yy18;
	++YYCURSOR;
#line 660 "ext/standard/var_unserializer.re"
	{
	size_t len, len2, len3, maxlen;
	long elements;
	char *class_name;

	do {
		/* Try to find class directly */
		BG(serialize_lock)++;
		if (zend_lookup_class(class_name, len2, &pce TSRMLS_CC) == SUCCESS) {
			BG(serialize_lock)--;
			if (EG(exception)) {
				efree(class_name);
				return 0;
			}
			ce = *pce;
			break;
		}
		BG(serialize_lock)--;

		if (EG(exception)) {
			efree(class_name);
			return 0;
		args[0] = &arg_func_name;
		MAKE_STD_ZVAL(arg_func_name);
		ZVAL_STRING(arg_func_name, class_name, 1);
		BG(serialize_lock)++;
		if (call_user_function_ex(CG(function_table), NULL, user_func, &retval_ptr, 1, args, 0, NULL TSRMLS_CC) != SUCCESS) {
			BG(serialize_lock)--;
			if (EG(exception)) {
				efree(class_name);
				zval_ptr_dtor(&user_func);
				zval_ptr_dtor(&arg_func_name);
			zval_ptr_dtor(&arg_func_name);
			break;
		}
		BG(serialize_lock)--;
		if (retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		}
		if (EG(exception)) {
	*p = YYCURSOR;

	if (custom_object) {
		int ret;

		ret = object_custom(UNSERIALIZE_PASSTHRU, ce);

		if (ret && incomplete_class) {
			php_store_class_name(*rval, class_name, len2);
		}

	return object_common2(UNSERIALIZE_PASSTHRU, elements);
}
#line 748 "ext/standard/var_unserializer.c"
yy25:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy18;
	yych = *++YYCURSOR;
	if (yych != '"') goto yy18;
	++YYCURSOR;
#line 652 "ext/standard/var_unserializer.re"
	{

	INIT_PZVAL(*rval);
	
	return object_common2(UNSERIALIZE_PASSTHRU,
			object_common1(UNSERIALIZE_PASSTHRU, ZEND_STANDARD_CLASS_DEF_PTR));
}
#line 781 "ext/standard/var_unserializer.c"
yy32:
	yych = *++YYCURSOR;
	if (yych == '+') goto yy33;
	if (yych <= '/') goto yy18;
	yych = *++YYCURSOR;
	if (yych != '{') goto yy18;
	++YYCURSOR;
#line 632 "ext/standard/var_unserializer.re"
	{
	long elements = parse_iv(start + 2);
	/* use iv() not uiv() in order to check data range */
	*p = YYCURSOR;

	return finish_nested_data(UNSERIALIZE_PASSTHRU);
}
#line 822 "ext/standard/var_unserializer.c"
yy39:
	yych = *++YYCURSOR;
	if (yych == '+') goto yy40;
	if (yych <= '/') goto yy18;
	yych = *++YYCURSOR;
	if (yych != '"') goto yy18;
	++YYCURSOR;
#line 603 "ext/standard/var_unserializer.re"
	{
	size_t len, maxlen;
	char *str;

	ZVAL_STRINGL(*rval, str, len, 0);
	return 1;
}
#line 872 "ext/standard/var_unserializer.c"
yy46:
	yych = *++YYCURSOR;
	if (yych == '+') goto yy47;
	if (yych <= '/') goto yy18;
	yych = *++YYCURSOR;
	if (yych != '"') goto yy18;
	++YYCURSOR;
#line 575 "ext/standard/var_unserializer.re"
	{
	size_t len, maxlen;
	char *str;

	ZVAL_STRINGL(*rval, str, len, 1);
	return 1;
}
#line 921 "ext/standard/var_unserializer.c"
yy53:
	yych = *++YYCURSOR;
	if (yych <= '/') {
		if (yych <= ',') {
	}
yy63:
	++YYCURSOR;
#line 565 "ext/standard/var_unserializer.re"
	{
#if SIZEOF_LONG == 4
use_double:
#endif
	ZVAL_DOUBLE(*rval, zend_strtod((const char *)start + 2, NULL));
	return 1;
}
#line 1019 "ext/standard/var_unserializer.c"
yy65:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy18;
	yych = *++YYCURSOR;
	if (yych != ';') goto yy18;
	++YYCURSOR;
#line 550 "ext/standard/var_unserializer.re"
	{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);


	return 1;
}
#line 1093 "ext/standard/var_unserializer.c"
yy76:
	yych = *++YYCURSOR;
	if (yych == 'N') goto yy73;
	goto yy18;
	if (yych <= '9') goto yy79;
	if (yych != ';') goto yy18;
	++YYCURSOR;
#line 523 "ext/standard/var_unserializer.re"
	{
#if SIZEOF_LONG == 4
	int digits = YYCURSOR - start - 3;

	ZVAL_LONG(*rval, parse_iv(start + 2));
	return 1;
}
#line 1147 "ext/standard/var_unserializer.c"
yy83:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy18;
	if (yych >= '2') goto yy18;
	yych = *++YYCURSOR;
	if (yych != ';') goto yy18;
	++YYCURSOR;
#line 516 "ext/standard/var_unserializer.re"
	{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_BOOL(*rval, parse_iv(start + 2));
	return 1;
}
#line 1162 "ext/standard/var_unserializer.c"
yy87:
	++YYCURSOR;
#line 509 "ext/standard/var_unserializer.re"
	{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_NULL(*rval);
	return 1;
}
#line 1172 "ext/standard/var_unserializer.c"
yy89:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy18;
	if (yych <= '9') goto yy91;
	if (yych != ';') goto yy18;
	++YYCURSOR;
#line 486 "ext/standard/var_unserializer.re"
	{
	long id;

 	*p = YYCURSOR;
	if (*rval == *rval_ref) return 0;

	if (*rval != NULL) {
		var_push_dtor_no_addref(var_hash, rval);
	}
	*rval = *rval_ref;
	Z_ADDREF_PP(rval);
	Z_UNSET_ISREF_PP(rval);
	
	return 1;
}
#line 1218 "ext/standard/var_unserializer.c"
yy95:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy18;
	if (yych <= '9') goto yy97;
	if (yych != ';') goto yy18;
	++YYCURSOR;
#line 465 "ext/standard/var_unserializer.re"
	{
	long id;

 	*p = YYCURSOR;
	
	return 1;
}
#line 1262 "ext/standard/var_unserializer.c"
}
#line 814 "ext/standard/var_unserializer.re"


	return 0;
}