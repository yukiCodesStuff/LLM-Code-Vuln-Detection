/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: John Coggeshall <john@php.net>                               |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_tidy.h"

#if HAVE_TIDY

#include "php_ini.h"
#include "ext/standard/info.h"

#include "tidy.h"
#include "buffio.h"

/* compatibility with older versions of libtidy */
#ifndef TIDY_CALL
#define TIDY_CALL
#endif

/* {{{ ext/tidy macros
*/
#define FIX_BUFFER(bptr) do { if ((bptr)->size) { (bptr)->bp[(bptr)->size-1] = '\0'; } } while(0)

#define TIDY_SET_CONTEXT \
    zval *object = getThis();

#define TIDY_FETCH_OBJECT	\
	PHPTidyObj *obj;	\
	TIDY_SET_CONTEXT; \
	if (object) {	\
		if (zend_parse_parameters_none() == FAILURE) {	\
			return;	\
		}	\
	} else {	\
		if (zend_parse_method_parameters(ZEND_NUM_ARGS(), NULL, "O", &object, tidy_ce_doc) == FAILURE) {	\
			RETURN_FALSE;	\
		}	\
	}	\
	obj = Z_TIDY_P(object);	\

#define TIDY_FETCH_ONLY_OBJECT	\
	PHPTidyObj *obj;	\
	TIDY_SET_CONTEXT; \
	if (zend_parse_parameters_none() == FAILURE) {	\
		return;	\
	}	\
	obj = Z_TIDY_P(object);	\

#define TIDY_APPLY_CONFIG_ZVAL(_doc, _val) \
    if(_val) { \
        if(Z_TYPE_P(_val) == IS_ARRAY) { \
            _php_tidy_apply_config_array(_doc, Z_ARRVAL_P(_val)); \
        } else { \
            convert_to_string_ex(_val); \
            TIDY_OPEN_BASE_DIR_CHECK(Z_STRVAL_P(_val)); \
            switch (tidyLoadConfig(_doc, Z_STRVAL_P(_val))) { \
              case -1: \
                php_error_docref(NULL, E_WARNING, "Could not load configuration file '%s'", Z_STRVAL_P(_val)); \
                break; \
              case 1: \
                php_error_docref(NULL, E_NOTICE, "There were errors while parsing the configuration file '%s'", Z_STRVAL_P(_val)); \
                break; \
            } \
        } \
    }

#define REGISTER_TIDY_CLASS(classname, name, parent, __flags) \
	{ \
		zend_class_entry ce; \
		INIT_CLASS_ENTRY(ce, # classname, tidy_funcs_ ## name); \
		ce.create_object = tidy_object_new_ ## name; \
		tidy_ce_ ## name = zend_register_internal_class_ex(&ce, parent); \
		tidy_ce_ ## name->ce_flags |= __flags;  \
		memcpy(&tidy_object_handlers_ ## name, zend_get_std_object_handlers(), sizeof(zend_object_handlers)); \
		tidy_object_handlers_ ## name.clone_obj = NULL; \
	}

#define TIDY_TAG_CONST(tag) REGISTER_LONG_CONSTANT("TIDY_TAG_" #tag, TidyTag_##tag, CONST_CS | CONST_PERSISTENT)
#define TIDY_NODE_CONST(name, type) REGISTER_LONG_CONSTANT("TIDY_NODETYPE_" #name, TidyNode_##type, CONST_CS | CONST_PERSISTENT)

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define ADD_PROPERTY_STRING(_table, _key, _string) \
	{ \
		zval tmp; \
		if (_string) { \
			ZVAL_STRING(&tmp, (char *)_string); \
		} else { \
			ZVAL_EMPTY_STRING(&tmp); \
		} \
		zend_hash_str_update(_table, #_key, sizeof(#_key) - 1, &tmp); \
	}

#define ADD_PROPERTY_STRINGL(_table, _key, _string, _len) \
   { \
       zval tmp; \
       if (_string) { \
           ZVAL_STRINGL(&tmp, (char *)_string, _len); \
       } else { \
           ZVAL_EMPTY_STRING(&tmp); \
       } \
       zend_hash_str_update(_table, #_key, sizeof(#_key) - 1, &tmp); \
   }

#define ADD_PROPERTY_LONG(_table, _key, _long) \
	{ \
		zval tmp; \
		ZVAL_LONG(&tmp, _long); \
		zend_hash_str_update(_table, #_key, sizeof(#_key) - 1, &tmp); \
	}

#define ADD_PROPERTY_NULL(_table, _key) \
	{ \
		zval tmp; \
		ZVAL_NULL(&tmp); \
		zend_hash_str_update(_table, #_key, sizeof(#_key) - 1, &tmp); \
	}

#define ADD_PROPERTY_BOOL(_table, _key, _bool) \
    { \
		zval tmp; \
		ZVAL_BOOL(&tmp, _bool); \
		zend_hash_str_update(_table, #_key, sizeof(#_key) - 1, &tmp); \
	}

#define TIDY_OPEN_BASE_DIR_CHECK(filename) \
if (php_check_open_basedir(filename)) { \
	RETURN_FALSE; \
} \

#define TIDY_SET_DEFAULT_CONFIG(_doc) \
	if (TG(default_config) && TG(default_config)[0]) { \
		if (tidyLoadConfig(_doc, TG(default_config)) < 0) { \
			php_error_docref(NULL, E_WARNING, "Unable to load Tidy configuration file at '%s'.", TG(default_config)); \
		} \
	}
/* }}} */

/* {{{ ext/tidy structs
*/
typedef struct _PHPTidyDoc PHPTidyDoc;
typedef struct _PHPTidyObj PHPTidyObj;

typedef enum {
	is_node,
	is_doc
} tidy_obj_type;

typedef enum {
	is_root_node,
	is_html_node,
	is_head_node,
	is_body_node
} tidy_base_nodetypes;

struct _PHPTidyDoc {
	TidyDoc			doc;
	TidyBuffer		*errbuf;
	unsigned int	ref_count;
	unsigned int    initialized:1;
};

struct _PHPTidyObj {
	TidyNode		node;
	tidy_obj_type	type;
	PHPTidyDoc		*ptdoc;
	zend_object		std;
};

static inline PHPTidyObj *php_tidy_fetch_object(zend_object *obj) {
	return (PHPTidyObj *)((char*)(obj) - XtOffsetOf(PHPTidyObj, std));
}

#define Z_TIDY_P(zv) php_tidy_fetch_object(Z_OBJ_P((zv)))
/* }}} */

/* {{{ ext/tidy prototypes
*/
static zend_string *php_tidy_file_to_mem(char *, zend_bool);
static void tidy_object_free_storage(zend_object *);
static zend_object *tidy_object_new_node(zend_class_entry *);
static zend_object *tidy_object_new_doc(zend_class_entry *);
static zval * tidy_instanciate(zend_class_entry *, zval *);
static int tidy_doc_cast_handler(zval *, zval *, int);
static int tidy_node_cast_handler(zval *, zval *, int);
static void tidy_doc_update_properties(PHPTidyObj *);
static void tidy_add_default_properties(PHPTidyObj *, tidy_obj_type);
static void *php_tidy_get_opt_val(PHPTidyDoc *, TidyOption, TidyOptionType *);
static void php_tidy_create_node(INTERNAL_FUNCTION_PARAMETERS, tidy_base_nodetypes);
static int _php_tidy_set_tidy_opt(TidyDoc, char *, zval *);
static int _php_tidy_apply_config_array(TidyDoc doc, HashTable *ht_options);
static void _php_tidy_register_nodetypes(INIT_FUNC_ARGS);
static void _php_tidy_register_tags(INIT_FUNC_ARGS);
static PHP_INI_MH(php_tidy_set_clean_output);
static void php_tidy_clean_output_start(const char *name, size_t name_len);
static php_output_handler *php_tidy_output_handler_init(const char *handler_name, size_t handler_name_len, size_t chunk_size, int flags);
static int php_tidy_output_handler(void **nothing, php_output_context *output_context);

static PHP_MINIT_FUNCTION(tidy);
static PHP_MSHUTDOWN_FUNCTION(tidy);
static PHP_RINIT_FUNCTION(tidy);
static PHP_MINFO_FUNCTION(tidy);

static PHP_FUNCTION(tidy_getopt);
static PHP_FUNCTION(tidy_parse_string);
static PHP_FUNCTION(tidy_parse_file);
static PHP_FUNCTION(tidy_clean_repair);
static PHP_FUNCTION(tidy_repair_string);
static PHP_FUNCTION(tidy_repair_file);
static PHP_FUNCTION(tidy_diagnose);
static PHP_FUNCTION(tidy_get_output);
static PHP_FUNCTION(tidy_get_error_buffer);
static PHP_FUNCTION(tidy_get_release);
static PHP_FUNCTION(tidy_get_config);
static PHP_FUNCTION(tidy_get_status);
static PHP_FUNCTION(tidy_get_html_ver);
#if HAVE_TIDYOPTGETDOC
static PHP_FUNCTION(tidy_get_opt_doc);
#endif
static PHP_FUNCTION(tidy_is_xhtml);
static PHP_FUNCTION(tidy_is_xml);
static PHP_FUNCTION(tidy_error_count);
static PHP_FUNCTION(tidy_warning_count);
static PHP_FUNCTION(tidy_access_count);
static PHP_FUNCTION(tidy_config_count);

static PHP_FUNCTION(tidy_get_root);
static PHP_FUNCTION(tidy_get_html);
static PHP_FUNCTION(tidy_get_head);
static PHP_FUNCTION(tidy_get_body);

static TIDY_DOC_METHOD(__construct);
static TIDY_DOC_METHOD(parseFile);
static TIDY_DOC_METHOD(parseString);

static TIDY_NODE_METHOD(hasChildren);
static TIDY_NODE_METHOD(hasSiblings);
static TIDY_NODE_METHOD(isComment);
static TIDY_NODE_METHOD(isHtml);
static TIDY_NODE_METHOD(isText);
static TIDY_NODE_METHOD(isJste);
static TIDY_NODE_METHOD(isAsp);
static TIDY_NODE_METHOD(isPhp);
static TIDY_NODE_METHOD(getParent);
static TIDY_NODE_METHOD(__construct);
/* }}} */

ZEND_DECLARE_MODULE_GLOBALS(tidy)

PHP_INI_BEGIN()
STD_PHP_INI_ENTRY("tidy.default_config",	"",		PHP_INI_SYSTEM,		OnUpdateString,				default_config,		zend_tidy_globals,	tidy_globals)
STD_PHP_INI_ENTRY("tidy.clean_output",		"0",	PHP_INI_USER,		php_tidy_set_clean_output,	clean_output,		zend_tidy_globals,	tidy_globals)
PHP_INI_END()

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tidy_parse_string, 0, 0, 1)
	ZEND_ARG_INFO(0, input)
	ZEND_ARG_INFO(0, config_options)
	ZEND_ARG_INFO(0, encoding)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_get_error_buffer, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_get_output, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tidy_parse_file, 0, 0, 1)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, config_options)
	ZEND_ARG_INFO(0, encoding)
	ZEND_ARG_INFO(0, use_include_path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_clean_repair, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tidy_repair_string, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, config_file)
	ZEND_ARG_INFO(0, encoding)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tidy_repair_file, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, config_file)
	ZEND_ARG_INFO(0, encoding)
	ZEND_ARG_INFO(0, use_include_path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_diagnose, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_get_release, 0)
ZEND_END_ARG_INFO()

#if HAVE_TIDYOPTGETDOC
ZEND_BEGIN_ARG_INFO_EX(arginfo_tidy_get_opt_doc, 0, 0, 2)
	ZEND_ARG_INFO(0, resource)
	ZEND_ARG_INFO(0, optname)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO(arginfo_tidy_get_config, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_get_status, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_get_html_ver, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_is_xhtml, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_is_xml, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_error_count, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_warning_count, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_access_count, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_config_count, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tidy_getopt, 0, 0, 1)
	ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_get_root, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_get_html, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_tidy_get_head, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tidy_get_body, 0, 0, 1)
	ZEND_ARG_INFO(0, tidy)
ZEND_END_ARG_INFO()
/* }}} */

static const zend_function_entry tidy_functions[] = {
	PHP_FE(tidy_getopt,             arginfo_tidy_getopt)
	PHP_FE(tidy_parse_string,       arginfo_tidy_parse_string)
	PHP_FE(tidy_parse_file,         arginfo_tidy_parse_file)
	PHP_FE(tidy_get_output,         arginfo_tidy_get_output)
	PHP_FE(tidy_get_error_buffer,   arginfo_tidy_get_error_buffer)
	PHP_FE(tidy_clean_repair,       arginfo_tidy_clean_repair)
	PHP_FE(tidy_repair_string,	arginfo_tidy_repair_string)
	PHP_FE(tidy_repair_file,	arginfo_tidy_repair_file)
	PHP_FE(tidy_diagnose,           arginfo_tidy_diagnose)
	PHP_FE(tidy_get_release,	arginfo_tidy_get_release)
	PHP_FE(tidy_get_config,		arginfo_tidy_get_config)
	PHP_FE(tidy_get_status,		arginfo_tidy_get_status)
	PHP_FE(tidy_get_html_ver,	arginfo_tidy_get_html_ver)
	PHP_FE(tidy_is_xhtml,		arginfo_tidy_is_xhtml)
	PHP_FE(tidy_is_xml,		arginfo_tidy_is_xml)
	PHP_FE(tidy_error_count,	arginfo_tidy_error_count)
	PHP_FE(tidy_warning_count,	arginfo_tidy_warning_count)
	PHP_FE(tidy_access_count,	arginfo_tidy_access_count)
	PHP_FE(tidy_config_count,	arginfo_tidy_config_count)
#if HAVE_TIDYOPTGETDOC
	PHP_FE(tidy_get_opt_doc,	arginfo_tidy_get_opt_doc)
#endif
	PHP_FE(tidy_get_root,		arginfo_tidy_get_root)
	PHP_FE(tidy_get_head,		arginfo_tidy_get_head)
	PHP_FE(tidy_get_html,		arginfo_tidy_get_html)
	PHP_FE(tidy_get_body,		arginfo_tidy_get_body)
	PHP_FE_END
};

static const zend_function_entry tidy_funcs_doc[] = {
	TIDY_METHOD_MAP(getOpt, tidy_getopt, NULL)
	TIDY_METHOD_MAP(cleanRepair, tidy_clean_repair, NULL)
	TIDY_DOC_ME(parseFile, NULL)
	TIDY_DOC_ME(parseString, NULL)
	TIDY_METHOD_MAP(repairString, tidy_repair_string, NULL)
	TIDY_METHOD_MAP(repairFile, tidy_repair_file, NULL)
	TIDY_METHOD_MAP(diagnose, tidy_diagnose, NULL)
	TIDY_METHOD_MAP(getRelease, tidy_get_release, NULL)
	TIDY_METHOD_MAP(getConfig, tidy_get_config, NULL)
	TIDY_METHOD_MAP(getStatus, tidy_get_status, NULL)
	TIDY_METHOD_MAP(getHtmlVer, tidy_get_html_ver, NULL)
#if HAVE_TIDYOPTGETDOC
	TIDY_METHOD_MAP(getOptDoc, tidy_get_opt_doc, NULL)
#endif
	TIDY_METHOD_MAP(isXhtml, tidy_is_xhtml, NULL)
	TIDY_METHOD_MAP(isXml, tidy_is_xml, NULL)
	TIDY_METHOD_MAP(root, tidy_get_root, NULL)
	TIDY_METHOD_MAP(head, tidy_get_head, NULL)
	TIDY_METHOD_MAP(html, tidy_get_html, NULL)
	TIDY_METHOD_MAP(body, tidy_get_body, NULL)
	TIDY_DOC_ME(__construct, NULL)
	PHP_FE_END
};

static const zend_function_entry tidy_funcs_node[] = {
	TIDY_NODE_ME(hasChildren, NULL)
	TIDY_NODE_ME(hasSiblings, NULL)
	TIDY_NODE_ME(isComment, NULL)
	TIDY_NODE_ME(isHtml, NULL)
	TIDY_NODE_ME(isText, NULL)
	TIDY_NODE_ME(isJste, NULL)
	TIDY_NODE_ME(isAsp, NULL)
	TIDY_NODE_ME(isPhp, NULL)
	TIDY_NODE_ME(getParent, NULL)
	TIDY_NODE_PRIVATE_ME(__construct, NULL)
	PHP_FE_END
};

static zend_class_entry *tidy_ce_doc, *tidy_ce_node;

static zend_object_handlers tidy_object_handlers_doc;
static zend_object_handlers tidy_object_handlers_node;

zend_module_entry tidy_module_entry = {
	STANDARD_MODULE_HEADER,
	"tidy",
	tidy_functions,
	PHP_MINIT(tidy),
	PHP_MSHUTDOWN(tidy),
	PHP_RINIT(tidy),
	NULL,
	PHP_MINFO(tidy),
	PHP_TIDY_VERSION,
	PHP_MODULE_GLOBALS(tidy),
	NULL,
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_TIDY
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(tidy)
#endif

static void* TIDY_CALL php_tidy_malloc(size_t len)
{
	return emalloc(len);
}