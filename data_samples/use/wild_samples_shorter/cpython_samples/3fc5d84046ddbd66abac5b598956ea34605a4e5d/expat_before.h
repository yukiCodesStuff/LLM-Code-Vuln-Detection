                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000-2017 Expat development team
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
  XML_ERROR_RESERVED_PREFIX_XMLNS,
  XML_ERROR_RESERVED_NAMESPACE_URI,
  /* Added in 2.2.1. */
  XML_ERROR_INVALID_ARGUMENT
};

enum XML_Content_Type {
  XML_CTYPE_EMPTY = 1,

   For internal entities (<!ENTITY foo "bar">), value will
   be non-NULL and systemId, publicID, and notationName will be NULL.
   The value string is NOT nul-terminated; the length is provided in
   the value_length argument. Since it is legal to have zero-length
   values, do not use this argument to test for internal entities.

   For external entities, value will be NULL and systemId will be
   Otherwise it must return XML_STATUS_ERROR.

   If info does not describe a suitable encoding, then the parser will
   return an XML_UNKNOWN_ENCODING error.
*/
typedef int(XMLCALL *XML_UnknownEncodingHandler)(void *encodingHandlerData,
                                                 const XML_Char *name,
                                                 XML_Encoding *info);
/* Returns the number of the attribute/value pairs passed in last call
   to the XML_StartElementHandler that were specified in the start-tag
   rather than defaulted. Each attribute/value pair counts as 2; thus
   this correspondds to an index into the atts array passed to the
   XML_StartElementHandler.  Returns -1 if parser == NULL.
*/
XMLPARSEAPI(int)
XML_GetSpecifiedAttributeCount(XML_Parser parser);
/* Returns the index of the ID attribute passed in the last call to
   XML_StartElementHandler, or -1 if there is no ID attribute or
   parser == NULL.  Each attribute/value pair counts as 2; thus this
   correspondds to an index into the atts array passed to the
   XML_StartElementHandler.
*/
XMLPARSEAPI(int)
XML_GetIdAttributeIndex(XML_Parser parser);
  XML_FEATURE_SIZEOF_XML_LCHAR,
  XML_FEATURE_NS,
  XML_FEATURE_LARGE_SIZE,
  XML_FEATURE_ATTR_INFO
  /* Additional features must be added to the end of this enum. */
};

typedef struct {
XMLPARSEAPI(const XML_Feature *)
XML_GetFeatureList(void);

/* Expat follows the semantic versioning convention.
   See http://semver.org.
*/
#define XML_MAJOR_VERSION 2
#define XML_MINOR_VERSION 2
#define XML_MICRO_VERSION 8

#ifdef __cplusplus
}
#endif