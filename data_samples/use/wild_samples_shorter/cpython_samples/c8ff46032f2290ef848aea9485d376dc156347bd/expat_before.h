XML_SetParamEntityParsing(XML_Parser parser,
                          enum XML_ParamEntityParsing parsing);

/* If XML_Parse or XML_ParseBuffer have returned XML_STATUS_ERROR, then
   XML_GetErrorCode returns information about the error.
*/
XMLPARSEAPI(enum XML_Error)