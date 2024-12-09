#include <libxml/xmlsave.h>
#ifdef LIBXML_SCHEMAS_ENABLED
#include <libxml/relaxng.h>
#endif

#include "php_libxml.h"

#endif
	REGISTER_LONG_CONSTANT("LIBXML_NOEMPTYTAG",	LIBXML_SAVE_NOEMPTYTAG,	CONST_CS | CONST_PERSISTENT);

	/* Additional constants for use with loading html */
#if LIBXML_VERSION >= 20707
	REGISTER_LONG_CONSTANT("LIBXML_HTML_NOIMPLIED",	HTML_PARSE_NOIMPLIED,		CONST_CS | CONST_PERSISTENT);
#endif