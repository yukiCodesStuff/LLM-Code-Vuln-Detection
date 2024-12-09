
/* {{{ reference-handling for unserializer: var_* */
#define VAR_ENTRIES_MAX 1024

typedef struct {
	zval *data[VAR_ENTRIES_MAX];
	long used_slots;
static inline void var_push(php_unserialize_data_t *var_hashx, zval **rval)
{
	var_entries *var_hash = (*var_hashx)->last;
#if 0
	fprintf(stderr, "var_push(%ld): %d\n", var_hash?var_hash->used_slots:-1L, Z_TYPE_PP(rval));
#endif

	if (!var_hash || var_hash->used_slots == VAR_ENTRIES_MAX) {
PHPAPI void var_push_dtor(php_unserialize_data_t *var_hashx, zval **rval)
{
	var_entries *var_hash = (*var_hashx)->last_dtor;
#if 0
	fprintf(stderr, "var_push_dtor(%ld): %d\n", var_hash?var_hash->used_slots:-1L, Z_TYPE_PP(rval));
#endif

	if (!var_hash || var_hash->used_slots == VAR_ENTRIES_MAX) {
	var_hash->data[var_hash->used_slots++] = *rval;
}

PHPAPI void var_replace(php_unserialize_data_t *var_hashx, zval *ozval, zval **nzval)
{
	long i;
	var_entries *var_hash = (*var_hashx)->first;
#if 0
	fprintf(stderr, "var_replace(%ld): %d\n", var_hash?var_hash->used_slots:-1L, Z_TYPE_PP(nzval));
#endif
	
	while (var_hash) {
static int var_access(php_unserialize_data_t *var_hashx, long id, zval ***store)
{
	var_entries *var_hash = (*var_hashx)->first;
#if 0
	fprintf(stderr, "var_access(%ld): %ld\n", var_hash?var_hash->used_slots:-1L, id);
#endif
		
	while (id >= VAR_ENTRIES_MAX && var_hash && var_hash->used_slots == VAR_ENTRIES_MAX) {
	void *next;
	long i;
	var_entries *var_hash = (*var_hashx)->first;
#if 0
	fprintf(stderr, "var_destroy(%ld)\n", var_hash?var_hash->used_slots:-1L);
#endif
	
	while (var_hash) {
#define YYMARKER marker


#line 209 "ext/standard/var_unserializer.re"




	
	

#line 436 "ext/standard/var_unserializer.c"
{
	YYCTYPE yych;
	static const unsigned char yybm[] = {
		  0,   0,   0,   0,   0,   0,   0,   0, 
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy95;
yy3:
#line 785 "ext/standard/var_unserializer.re"
	{ return 0; }
#line 498 "ext/standard/var_unserializer.c"
yy4:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == ':') goto yy89;
	goto yy3;
	goto yy3;
yy14:
	++YYCURSOR;
#line 779 "ext/standard/var_unserializer.re"
	{
	/* this is the case where we have less data than planned */
	php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Unexpected end of serialized data");
	return 0; /* not sure if it should be 0 or 1 here? */
}
#line 547 "ext/standard/var_unserializer.c"
yy16:
	yych = *++YYCURSOR;
	goto yy3;
yy17:
	yych = *++YYCURSOR;
	if (yych != '"') goto yy18;
	++YYCURSOR;
#line 635 "ext/standard/var_unserializer.re"
	{
	size_t len, len2, len3, maxlen;
	long elements;
	char *class_name;

	do {
		/* Try to find class directly */
		BG(serialize_lock) = 1;
		if (zend_lookup_class(class_name, len2, &pce TSRMLS_CC) == SUCCESS) {
			BG(serialize_lock) = 0;
			if (EG(exception)) {
				efree(class_name);
				return 0;
			}
			ce = *pce;
			break;
		}
		BG(serialize_lock) = 0;

		if (EG(exception)) {
			efree(class_name);
			return 0;
		args[0] = &arg_func_name;
		MAKE_STD_ZVAL(arg_func_name);
		ZVAL_STRING(arg_func_name, class_name, 1);
		BG(serialize_lock) = 1;
		if (call_user_function_ex(CG(function_table), NULL, user_func, &retval_ptr, 1, args, 0, NULL TSRMLS_CC) != SUCCESS) {
			BG(serialize_lock) = 0;
			if (EG(exception)) {
				efree(class_name);
				zval_ptr_dtor(&user_func);
				zval_ptr_dtor(&arg_func_name);
			zval_ptr_dtor(&arg_func_name);
			break;
		}
		BG(serialize_lock) = 0;
		if (retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		}
		if (EG(exception)) {
	*p = YYCURSOR;

	if (custom_object) {
		int ret = object_custom(UNSERIALIZE_PASSTHRU, ce);

		if (ret && incomplete_class) {
			php_store_class_name(*rval, class_name, len2);
		}

	return object_common2(UNSERIALIZE_PASSTHRU, elements);
}
#line 721 "ext/standard/var_unserializer.c"
yy25:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy18;
	yych = *++YYCURSOR;
	if (yych != '"') goto yy18;
	++YYCURSOR;
#line 627 "ext/standard/var_unserializer.re"
	{

	INIT_PZVAL(*rval);
	
	return object_common2(UNSERIALIZE_PASSTHRU,
			object_common1(UNSERIALIZE_PASSTHRU, ZEND_STANDARD_CLASS_DEF_PTR));
}
#line 754 "ext/standard/var_unserializer.c"
yy32:
	yych = *++YYCURSOR;
	if (yych == '+') goto yy33;
	if (yych <= '/') goto yy18;
	yych = *++YYCURSOR;
	if (yych != '{') goto yy18;
	++YYCURSOR;
#line 607 "ext/standard/var_unserializer.re"
	{
	long elements = parse_iv(start + 2);
	/* use iv() not uiv() in order to check data range */
	*p = YYCURSOR;

	return finish_nested_data(UNSERIALIZE_PASSTHRU);
}
#line 795 "ext/standard/var_unserializer.c"
yy39:
	yych = *++YYCURSOR;
	if (yych == '+') goto yy40;
	if (yych <= '/') goto yy18;
	yych = *++YYCURSOR;
	if (yych != '"') goto yy18;
	++YYCURSOR;
#line 578 "ext/standard/var_unserializer.re"
	{
	size_t len, maxlen;
	char *str;

	ZVAL_STRINGL(*rval, str, len, 0);
	return 1;
}
#line 845 "ext/standard/var_unserializer.c"
yy46:
	yych = *++YYCURSOR;
	if (yych == '+') goto yy47;
	if (yych <= '/') goto yy18;
	yych = *++YYCURSOR;
	if (yych != '"') goto yy18;
	++YYCURSOR;
#line 550 "ext/standard/var_unserializer.re"
	{
	size_t len, maxlen;
	char *str;

	ZVAL_STRINGL(*rval, str, len, 1);
	return 1;
}
#line 894 "ext/standard/var_unserializer.c"
yy53:
	yych = *++YYCURSOR;
	if (yych <= '/') {
		if (yych <= ',') {
	}
yy63:
	++YYCURSOR;
#line 540 "ext/standard/var_unserializer.re"
	{
#if SIZEOF_LONG == 4
use_double:
#endif
	ZVAL_DOUBLE(*rval, zend_strtod((const char *)start + 2, NULL));
	return 1;
}
#line 992 "ext/standard/var_unserializer.c"
yy65:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy18;
	yych = *++YYCURSOR;
	if (yych != ';') goto yy18;
	++YYCURSOR;
#line 525 "ext/standard/var_unserializer.re"
	{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);


	return 1;
}
#line 1066 "ext/standard/var_unserializer.c"
yy76:
	yych = *++YYCURSOR;
	if (yych == 'N') goto yy73;
	goto yy18;
	if (yych <= '9') goto yy79;
	if (yych != ';') goto yy18;
	++YYCURSOR;
#line 498 "ext/standard/var_unserializer.re"
	{
#if SIZEOF_LONG == 4
	int digits = YYCURSOR - start - 3;

	ZVAL_LONG(*rval, parse_iv(start + 2));
	return 1;
}
#line 1120 "ext/standard/var_unserializer.c"
yy83:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy18;
	if (yych >= '2') goto yy18;
	yych = *++YYCURSOR;
	if (yych != ';') goto yy18;
	++YYCURSOR;
#line 491 "ext/standard/var_unserializer.re"
	{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_BOOL(*rval, parse_iv(start + 2));
	return 1;
}
#line 1135 "ext/standard/var_unserializer.c"
yy87:
	++YYCURSOR;
#line 484 "ext/standard/var_unserializer.re"
	{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_NULL(*rval);
	return 1;
}
#line 1145 "ext/standard/var_unserializer.c"
yy89:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy18;
	if (yych <= '9') goto yy91;
	if (yych != ';') goto yy18;
	++YYCURSOR;
#line 461 "ext/standard/var_unserializer.re"
	{
	long id;

 	*p = YYCURSOR;
	if (*rval == *rval_ref) return 0;

	if (*rval != NULL) {
		zval_ptr_dtor(rval);
	}
	*rval = *rval_ref;
	Z_ADDREF_PP(rval);
	Z_UNSET_ISREF_PP(rval);
	
	return 1;
}
#line 1191 "ext/standard/var_unserializer.c"
yy95:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy18;
	if (yych <= '9') goto yy97;
	if (yych != ';') goto yy18;
	++YYCURSOR;
#line 440 "ext/standard/var_unserializer.re"
	{
	long id;

 	*p = YYCURSOR;
	
	return 1;
}
#line 1235 "ext/standard/var_unserializer.c"
}
#line 787 "ext/standard/var_unserializer.re"


	return 0;
}