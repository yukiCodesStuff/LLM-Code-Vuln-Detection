                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000      Clark Cooper <coopercc@users.sourceforge.net>
   Copyright (c) 2000-2005 Fred L. Drake, Jr. <fdrake@users.sourceforge.net>
   Copyright (c) 2001-2002 Greg Stein <gstein@users.sourceforge.net>
   Copyright (c) 2002-2016 Karl Waclawek <karl@waclawek.net>
   Copyright (c) 2016-2021 Sebastian Pipping <sebastian@pipping.org>
   Copyright (c) 2016      Cristian Rodríguez <crrodriguez@opensuse.org>
   Copyright (c) 2016      Thomas Beutlich <tc@tbeu.de>
   Copyright (c) 2017      Rhodri James <rhodri@wildebeest.org.uk>
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
  XML_ERROR_RESERVED_PREFIX_XMLNS,
  XML_ERROR_RESERVED_NAMESPACE_URI,
  /* Added in 2.2.1. */
  XML_ERROR_INVALID_ARGUMENT,
  /* Added in 2.3.0. */
  XML_ERROR_NO_BUFFER,
  /* Added in 2.4.0. */
  XML_ERROR_AMPLIFICATION_LIMIT_BREACH
};

enum XML_Content_Type {
  XML_CTYPE_EMPTY = 1,

   For internal entities (<!ENTITY foo "bar">), value will
   be non-NULL and systemId, publicID, and notationName will be NULL.
   The value string is NOT null-terminated; the length is provided in
   the value_length argument. Since it is legal to have zero-length
   values, do not use this argument to test for internal entities.

   For external entities, value will be NULL and systemId will be
   Otherwise it must return XML_STATUS_ERROR.

   If info does not describe a suitable encoding, then the parser will
   return an XML_ERROR_UNKNOWN_ENCODING error.
*/
typedef int(XMLCALL *XML_UnknownEncodingHandler)(void *encodingHandlerData,
                                                 const XML_Char *name,
                                                 XML_Encoding *info);
/* Returns the number of the attribute/value pairs passed in last call
   to the XML_StartElementHandler that were specified in the start-tag
   rather than defaulted. Each attribute/value pair counts as 2; thus
   this corresponds to an index into the atts array passed to the
   XML_StartElementHandler.  Returns -1 if parser == NULL.
*/
XMLPARSEAPI(int)
XML_GetSpecifiedAttributeCount(XML_Parser parser);
/* Returns the index of the ID attribute passed in the last call to
   XML_StartElementHandler, or -1 if there is no ID attribute or
   parser == NULL.  Each attribute/value pair counts as 2; thus this
   corresponds to an index into the atts array passed to the
   XML_StartElementHandler.
*/
XMLPARSEAPI(int)
XML_GetIdAttributeIndex(XML_Parser parser);
  XML_FEATURE_SIZEOF_XML_LCHAR,
  XML_FEATURE_NS,
  XML_FEATURE_LARGE_SIZE,
  XML_FEATURE_ATTR_INFO,
  /* Added in Expat 2.4.0. */
  XML_FEATURE_BILLION_LAUGHS_ATTACK_PROTECTION_MAXIMUM_AMPLIFICATION_DEFAULT,
  XML_FEATURE_BILLION_LAUGHS_ATTACK_PROTECTION_ACTIVATION_THRESHOLD_DEFAULT
  /* Additional features must be added to the end of this enum. */
};

typedef struct {
XMLPARSEAPI(const XML_Feature *)
XML_GetFeatureList(void);

#ifdef XML_DTD
/* Added in Expat 2.4.0. */
XMLPARSEAPI(XML_Bool)
XML_SetBillionLaughsAttackProtectionMaximumAmplification(
    XML_Parser parser, float maximumAmplificationFactor);

/* Added in Expat 2.4.0. */
XMLPARSEAPI(XML_Bool)
XML_SetBillionLaughsAttackProtectionActivationThreshold(
    XML_Parser parser, unsigned long long activationThresholdBytes);
#endif

/* Expat follows the semantic versioning convention.
   See http://semver.org.
*/
#define XML_MAJOR_VERSION 2
#define XML_MINOR_VERSION 4
#define XML_MICRO_VERSION 1

#ifdef __cplusplus
}
#endif