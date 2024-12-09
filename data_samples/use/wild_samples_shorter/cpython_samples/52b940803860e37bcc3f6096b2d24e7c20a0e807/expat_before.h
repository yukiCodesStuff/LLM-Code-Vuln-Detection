#ifndef Expat_INCLUDED
#define Expat_INCLUDED 1

#ifdef __VMS
/*      0        1         2         3      0        1         2         3
        1234567890123456789012345678901     1234567890123456789012345678901 */
#define XML_SetProcessingInstructionHandler XML_SetProcessingInstrHandler
#define XML_SetUnparsedEntityDeclHandler    XML_SetUnparsedEntDeclHandler
#define XML_SetStartNamespaceDeclHandler    XML_SetStartNamespcDeclHandler
#define XML_SetExternalEntityRefHandlerArg  XML_SetExternalEntRefHandlerArg
#endif

#include <stdlib.h>
#include "expat_external.h"

#ifdef __cplusplus
typedef struct XML_ParserStruct *XML_Parser;

typedef unsigned char XML_Bool;
#define XML_TRUE   ((XML_Bool) 1)
#define XML_FALSE  ((XML_Bool) 0)

/* The XML_Status enum gives the possible return values for several
   API functions.  The preprocessor #defines are included so this
   stanza can be added to code that still needs to support older
typedef struct XML_cp XML_Content;

struct XML_cp {
  enum XML_Content_Type         type;
  enum XML_Content_Quant        quant;
  XML_Char *                    name;
  unsigned int                  numchildren;
  XML_Content *                 children;
};


/* This is called for an element declaration. See above for
   description of the model argument. It's the caller's responsibility
   to free model when finished with it.
*/
typedef void (XMLCALL *XML_ElementDeclHandler) (void *userData,
                                                const XML_Char *name,
                                                XML_Content *model);

XMLPARSEAPI(void)
XML_SetElementDeclHandler(XML_Parser parser,
                          XML_ElementDeclHandler eldecl);

/* The Attlist declaration handler is called for *each* attribute. So
   a single Attlist declaration with multiple attributes declared will
   generate multiple calls to this handler. The "default" parameter
   value will be NULL in the case of "#REQUIRED". If "isrequired" is
   true and default is non-NULL, then this is a "#FIXED" default.
*/
typedef void (XMLCALL *XML_AttlistDeclHandler) (
                                    void            *userData,
                                    const XML_Char  *elname,
                                    const XML_Char  *attname,
                                    const XML_Char  *att_type,
                                    const XML_Char  *dflt,
                                    int              isrequired);

XMLPARSEAPI(void)
XML_SetAttlistDeclHandler(XML_Parser parser,
                          XML_AttlistDeclHandler attdecl);

/* The XML declaration handler is called for *both* XML declarations
   and text declarations. The way to distinguish is that the version
   parameter will be NULL for text declarations. The encoding
   was no standalone parameter in the declaration, that it was given
   as no, or that it was given as yes.
*/
typedef void (XMLCALL *XML_XmlDeclHandler) (void           *userData,
                                            const XML_Char *version,
                                            const XML_Char *encoding,
                                            int             standalone);

XMLPARSEAPI(void)
XML_SetXmlDeclHandler(XML_Parser parser,
                      XML_XmlDeclHandler xmldecl);


typedef struct {
  void *(*malloc_fcn)(size_t size);
  void *(*realloc_fcn)(void *ptr, size_t size);
XMLPARSEAPI(XML_Parser)
XML_ParserCreateNS(const XML_Char *encoding, XML_Char namespaceSeparator);


/* Constructs a new parser using the memory management suite referred to
   by memsuite. If memsuite is NULL, then use the standard library memory
   suite. If namespaceSeparator is non-NULL it creates a parser with
   namespace processing as described above. The character pointed at

/* Prepare a parser object to be re-used.  This is particularly
   valuable when memory allocation overhead is disproportionately high,
   such as when a large number of small documents need to be parsed.
   All handlers are cleared from the parser, except for the
   unknownEncodingHandler. The parser's external state is re-initialized
   except for the values of ns and ns_triplets.

/* atts is array of name/value pairs, terminated by 0;
   names and values are 0 terminated.
*/
typedef void (XMLCALL *XML_StartElementHandler) (void *userData,
                                                 const XML_Char *name,
                                                 const XML_Char **atts);

typedef void (XMLCALL *XML_EndElementHandler) (void *userData,
                                               const XML_Char *name);


/* s is not 0 terminated. */
typedef void (XMLCALL *XML_CharacterDataHandler) (void *userData,
                                                  const XML_Char *s,
                                                  int len);

/* target and data are 0 terminated */
typedef void (XMLCALL *XML_ProcessingInstructionHandler) (
                                                void *userData,
                                                const XML_Char *target,
                                                const XML_Char *data);

/* data is 0 terminated */
typedef void (XMLCALL *XML_CommentHandler) (void *userData,
                                            const XML_Char *data);

typedef void (XMLCALL *XML_StartCdataSectionHandler) (void *userData);
typedef void (XMLCALL *XML_EndCdataSectionHandler) (void *userData);

/* This is called for any characters in the XML document for which
   there is no applicable handler.  This includes both characters that
   are part of markup which is of a kind that is not reported
   default handler: for example, a comment might be split between
   multiple calls.
*/
typedef void (XMLCALL *XML_DefaultHandler) (void *userData,
                                            const XML_Char *s,
                                            int len);

/* This is called for the start of the DOCTYPE declaration, before
   any DTD or internal subset is parsed.
*/
typedef void (XMLCALL *XML_StartDoctypeDeclHandler) (
                                            void *userData,
                                            const XML_Char *doctypeName,
                                            const XML_Char *sysid,
                                            const XML_Char *pubid,
                                            int has_internal_subset);

/* This is called for the start of the DOCTYPE declaration when the
   closing > is encountered, but after processing any external
   subset.
*/
typedef void (XMLCALL *XML_EndDoctypeDeclHandler)(void *userData);

/* This is called for entity declarations. The is_parameter_entity
   argument will be non-zero if the entity is a parameter entity, zero
   otherwise.
   Note that is_parameter_entity can't be changed to XML_Bool, since
   that would break binary compatibility.
*/
typedef void (XMLCALL *XML_EntityDeclHandler) (
                              void *userData,
                              const XML_Char *entityName,
                              int is_parameter_entity,
                              const XML_Char *value,
                              int value_length,
                              const XML_Char *base,
                              const XML_Char *systemId,
                              const XML_Char *publicId,
                              const XML_Char *notationName);

XMLPARSEAPI(void)
XML_SetEntityDeclHandler(XML_Parser parser,
                         XML_EntityDeclHandler handler);

/* OBSOLETE -- OBSOLETE -- OBSOLETE
   This handler has been superseded by the EntityDeclHandler above.
   It is provided here for backward compatibility.
   entityName, systemId and notationName arguments will never be
   NULL. The other arguments may be.
*/
typedef void (XMLCALL *XML_UnparsedEntityDeclHandler) (
                                    void *userData,
                                    const XML_Char *entityName,
                                    const XML_Char *base,
                                    const XML_Char *systemId,
                                    const XML_Char *publicId,
                                    const XML_Char *notationName);

/* This is called for a declaration of notation.  The base argument is
   whatever was set by XML_SetBase. The notationName will never be
   NULL.  The other arguments can be.
*/
typedef void (XMLCALL *XML_NotationDeclHandler) (
                                    void *userData,
                                    const XML_Char *notationName,
                                    const XML_Char *base,
                                    const XML_Char *systemId,
                                    const XML_Char *publicId);

/* When namespace processing is enabled, these are called once for
   each namespace declaration. The call to the start and end element
   handlers occur between the calls to the start and end namespace
   declaration handlers. For an xmlns attribute, prefix will be
   NULL.  For an xmlns="" attribute, uri will be NULL.
*/
typedef void (XMLCALL *XML_StartNamespaceDeclHandler) (
                                    void *userData,
                                    const XML_Char *prefix,
                                    const XML_Char *uri);

typedef void (XMLCALL *XML_EndNamespaceDeclHandler) (
                                    void *userData,
                                    const XML_Char *prefix);

/* This is called if the document is not standalone, that is, it has an
   external subset or a reference to a parameter entity, but does not
   have standalone="yes". If this handler returns XML_STATUS_ERROR,
   conditions above this handler will only be called if the referenced
   entity was actually read.
*/
typedef int (XMLCALL *XML_NotStandaloneHandler) (void *userData);

/* This is called for a reference to an external parsed general
   entity.  The referenced entity is not automatically parsed.  The
   application can parse it immediately or later using
   Note that unlike other handlers the first argument is the parser,
   not userData.
*/
typedef int (XMLCALL *XML_ExternalEntityRefHandler) (
                                    XML_Parser parser,
                                    const XML_Char *context,
                                    const XML_Char *base,
                                    const XML_Char *systemId,
                                    const XML_Char *publicId);

/* This is called in two situations:
   1) An entity reference is encountered for which no declaration
      has been read *and* this is not an error.
         the event would be out of sync with the reporting of the
         declarations or attribute values
*/
typedef void (XMLCALL *XML_SkippedEntityHandler) (
                                    void *userData,
                                    const XML_Char *entityName,
                                    int is_parameter_entity);

/* This structure is filled in by the XML_UnknownEncodingHandler to
   provide information to the parser about encodings that are unknown
   to the parser.
typedef struct {
  int map[256];
  void *data;
  int (XMLCALL *convert)(void *data, const char *s);
  void (XMLCALL *release)(void *data);
} XML_Encoding;

/* This is called for an encoding that is unknown to the parser.

   If info does not describe a suitable encoding, then the parser will
   return an XML_UNKNOWN_ENCODING error.
*/
typedef int (XMLCALL *XML_UnknownEncodingHandler) (
                                    void *encodingHandlerData,
                                    const XML_Char *name,
                                    XML_Encoding *info);

XMLPARSEAPI(void)
XML_SetElementHandler(XML_Parser parser,
                      XML_StartElementHandler start,
                      XML_EndElementHandler end);

XMLPARSEAPI(void)
XML_SetStartElementHandler(XML_Parser parser,
                           XML_StartElementHandler handler);

XMLPARSEAPI(void)
XML_SetEndElementHandler(XML_Parser parser,
                         XML_EndElementHandler handler);

XMLPARSEAPI(void)
XML_SetCharacterDataHandler(XML_Parser parser,
                            XML_CharacterDataHandler handler);
XML_SetProcessingInstructionHandler(XML_Parser parser,
                                    XML_ProcessingInstructionHandler handler);
XMLPARSEAPI(void)
XML_SetCommentHandler(XML_Parser parser,
                      XML_CommentHandler handler);

XMLPARSEAPI(void)
XML_SetCdataSectionHandler(XML_Parser parser,
                           XML_StartCdataSectionHandler start,
   default handler, or to the skipped entity handler, if one is set.
*/
XMLPARSEAPI(void)
XML_SetDefaultHandler(XML_Parser parser,
                      XML_DefaultHandler handler);

/* This sets the default handler but does not inhibit expansion of
   internal entities.  The entity reference will not be passed to the
   default handler.
*/
XMLPARSEAPI(void)
XML_SetDefaultHandlerExpand(XML_Parser parser,
                            XML_DefaultHandler handler);

XMLPARSEAPI(void)
XML_SetDoctypeDeclHandler(XML_Parser parser,
                          XML_StartDoctypeDeclHandler start,
                          XML_EndDoctypeDeclHandler end);

XMLPARSEAPI(void)
XML_SetStartDoctypeDeclHandler(XML_Parser parser,
                               XML_StartDoctypeDeclHandler start);

XMLPARSEAPI(void)
XML_SetEndDoctypeDeclHandler(XML_Parser parser,
                             XML_EndDoctypeDeclHandler end);

XMLPARSEAPI(void)
XML_SetUnparsedEntityDeclHandler(XML_Parser parser,
                                 XML_UnparsedEntityDeclHandler handler);

XMLPARSEAPI(void)
XML_SetNotationDeclHandler(XML_Parser parser,
                           XML_NotationDeclHandler handler);

XMLPARSEAPI(void)
XML_SetNamespaceDeclHandler(XML_Parser parser,
                            XML_StartNamespaceDeclHandler start,
   instead of the parser object.
*/
XMLPARSEAPI(void)
XML_SetExternalEntityRefHandlerArg(XML_Parser parser,
                                   void *arg);

XMLPARSEAPI(void)
XML_SetSkippedEntityHandler(XML_Parser parser,
                            XML_SkippedEntityHandler handler);
XMLPARSEAPI(enum XML_Error)
XML_UseForeignDTD(XML_Parser parser, XML_Bool useDTD);


/* Sets the base to be used for resolving relative URIs in system
   identifiers in declarations.  Resolving relative identifiers is
   left to the application: this value will be passed through as the
   base argument to the XML_ExternalEntityRefHandler,
   info->valueEnd - info->valueStart = 4 bytes.
*/
typedef struct {
  XML_Index  nameStart;  /* Offset to beginning of the attribute name. */
  XML_Index  nameEnd;    /* Offset after the attribute name's last byte. */
  XML_Index  valueStart; /* Offset to beginning of the attribute value. */
  XML_Index  valueEnd;   /* Offset after the attribute value's last byte. */
} XML_AttrInfo;

/* Returns an array of XML_AttrInfo structures for the attribute/value pairs
   passed in last call to the XML_StartElementHandler that were specified
   (resumable = 0) an already suspended parser. Some call-backs may
   still follow because they would otherwise get lost. Examples:
   - endElementHandler() for empty elements when stopped in
     startElementHandler(), 
   - endNameSpaceDeclHandler() when stopped in endElementHandler(), 
   and possibly others.

   Can be called from most handlers, including DTD related call-backs,
   except when parsing an external parameter entity and resumable != 0.
   Returns XML_STATUS_OK when successful, XML_STATUS_ERROR otherwise.
   Possible error codes: 
   - XML_ERROR_SUSPENDED: when suspending an already suspended parser.
   - XML_ERROR_FINISHED: when the parser has already finished.
   - XML_ERROR_SUSPEND_PE: when suspending while parsing an external PE.

   When resumable != 0 (true) then parsing is suspended, that is, 
   XML_Parse() and XML_ParseBuffer() return XML_STATUS_SUSPENDED. 
   Otherwise, parsing is aborted, that is, XML_Parse() and XML_ParseBuffer()
   return XML_STATUS_ERROR with error code XML_ERROR_ABORTED.

   *Note*:
   the externalEntityRefHandler() to call XML_StopParser() on the parent
   parser (recursively), if one wants to stop parsing altogether.

   When suspended, parsing can be resumed by calling XML_ResumeParser(). 
*/
XMLPARSEAPI(enum XML_Status)
XML_StopParser(XML_Parser parser, XML_Bool resumable);

/* Resumes parsing after it has been suspended with XML_StopParser().
   Must not be called from within a handler call-back. Returns same
   status codes as XML_Parse() or XML_ParseBuffer().
   Additional error code XML_ERROR_NOT_SUSPENDED possible.   

   *Note*:
   This must be called on the most deeply nested child parser instance
   first, and on its parent parser only after the child parser has finished,
XMLPARSEAPI(enum XML_Status)
XML_ResumeParser(XML_Parser parser);

enum XML_Parsing {
  XML_INITIALIZED,
  XML_PARSING,
  XML_FINISHED,
  XML_SUSPENDED
};

typedef struct {
  enum XML_Parsing parsing;
  XML_Bool finalBuffer;
   Otherwise returns a new XML_Parser object.
*/
XMLPARSEAPI(XML_Parser)
XML_ExternalEntityParserCreate(XML_Parser parser,
                               const XML_Char *context,
                               const XML_Char *encoding);

enum XML_ParamEntityParsing {
  XML_PARAM_ENTITY_PARSING_NEVER,
   Note: If parser == NULL, the function will do nothing and return 0.
*/
XMLPARSEAPI(int)
XML_SetHashSalt(XML_Parser parser,
                unsigned long hash_salt);

/* If XML_Parse or XML_ParseBuffer have returned XML_STATUS_ERROR, then
   XML_GetErrorCode returns information about the error.
*/
   be within the relevant markup.  When called outside of the callback
   functions, the position indicated will be just past the last parse
   event (regardless of whether there was an associated callback).
   
   They may also be called after returning from a call to XML_Parse
   or XML_ParseBuffer.  If the return value is XML_STATUS_ERROR then
   the location is the location of the character at which the error
   was detected; otherwise the location is the location of the last
   the handler that makes the call.
*/
XMLPARSEAPI(const char *)
XML_GetInputContext(XML_Parser parser,
                    int *offset,
                    int *size);

/* For backwards compatibility with previous versions. */
#define XML_GetErrorLineNumber   XML_GetCurrentLineNumber
#define XML_GetErrorColumnNumber XML_GetCurrentColumnNumber
#define XML_GetErrorByteIndex    XML_GetCurrentByteIndex

/* Frees the content model passed to the element declaration handler */
XMLPARSEAPI(void)
XML_FreeContentModel(XML_Parser parser, XML_Content *model);
};

typedef struct {
  enum XML_FeatureEnum  feature;
  const XML_LChar       *name;
  long int              value;
} XML_Feature;

XMLPARSEAPI(const XML_Feature *)
XML_GetFeatureList(void);


/* Expat follows the semantic versioning convention.
   See http://semver.org.
*/
#define XML_MAJOR_VERSION 2
#define XML_MINOR_VERSION 2
#define XML_MICRO_VERSION 7

#ifdef __cplusplus
}
#endif