#include <libxml/tree.h>
#include <libxml/hash.h>

typedef xmlChar XML_Char;

typedef void (*XML_StartElementHandler)(void *, const XML_Char *, const XML_Char **);
typedef void (*XML_EndElementHandler)(void *, const XML_Char *);
typedef struct _XML_Parser {
	int use_namespace;

	xmlChar *_ns_seperator;

	void *user;
	xmlParserCtxtPtr parser;
