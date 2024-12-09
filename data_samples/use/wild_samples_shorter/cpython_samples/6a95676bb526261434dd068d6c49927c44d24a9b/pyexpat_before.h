    enum XML_Status (*SetEncoding)(XML_Parser parser, const XML_Char *encoding);
    int (*DefaultUnknownEncodingHandler)(
        void *encodingHandlerData, const XML_Char *name, XML_Encoding *info);
    /* might be none for expat < 2.1.0 */
    int (*SetHashSalt)(XML_Parser parser, unsigned long hash_salt);
    /* always add new stuff to the end! */
};
