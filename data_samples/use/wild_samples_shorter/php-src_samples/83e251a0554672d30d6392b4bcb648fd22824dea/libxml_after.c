#include <libxml/xmlsave.h>
#ifdef LIBXML_SCHEMAS_ENABLED
#include <libxml/relaxng.h>
#include <libxml/xmlschemas.h>
#endif

#include "php_libxml.h"

#endif
	REGISTER_LONG_CONSTANT("LIBXML_NOEMPTYTAG",	LIBXML_SAVE_NOEMPTYTAG,	CONST_CS | CONST_PERSISTENT);

	/* Schema validation options */
#if defined(LIBXML_SCHEMAS_ENABLED) && LIBXML_VERSION >= 20614
	REGISTER_LONG_CONSTANT("LIBXML_SCHEMA_CREATE",	XML_SCHEMA_VAL_VC_I_CREATE,	CONST_CS | CONST_PERSISTENT);
#endif

	/* Additional constants for use with loading html */
#if LIBXML_VERSION >= 20707
	REGISTER_LONG_CONSTANT("LIBXML_HTML_NOIMPLIED",	HTML_PARSE_NOIMPLIED,		CONST_CS | CONST_PERSISTENT);
#endif