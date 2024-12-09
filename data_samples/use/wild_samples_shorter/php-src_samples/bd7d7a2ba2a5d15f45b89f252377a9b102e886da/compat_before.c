	if (URI) {
			/* Use libxml functions otherwise its memory deallocation is screwed up */
			*qualified = xmlStrdup(URI);
			*qualified = xmlStrncat(*qualified, parser->_ns_seperator, 1);
			*qualified = xmlStrncat(*qualified, name, xmlStrlen(name));
	} else {
		*qualified = xmlStrdup(name);
	}
	parser = (XML_Parser) emalloc(sizeof(struct _XML_Parser));
	memset(parser, 0, sizeof(struct _XML_Parser));
	parser->use_namespace = 0;
	parser->_ns_seperator = NULL;

	parser->parser = xmlCreatePushParserCtxt((xmlSAXHandlerPtr) &php_xml_compat_handlers, (void *) parser, NULL, 0, NULL);
	if (parser->parser == NULL) {
		efree(parser);
	if (sep != NULL) {
		parser->use_namespace = 1;
		parser->parser->sax2 = 1;
		parser->_ns_seperator = xmlStrdup(sep);
	} else {
		/* Reset flag as XML_SAX2_MAGIC is needed for xmlCreatePushParserCtxt 
		so must be set in the handlers */
		parser->parser->sax->initialized = 1;
XML_ParserFree(XML_Parser parser)
{
	if (parser->use_namespace) {
		if (parser->_ns_seperator) {
			xmlFree(parser->_ns_seperator);
		}
	}
	if (parser->parser->myDoc) {
		xmlFreeDoc(parser->parser->myDoc);