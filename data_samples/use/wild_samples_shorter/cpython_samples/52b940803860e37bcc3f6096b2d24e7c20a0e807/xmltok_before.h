#endif

/* The following token may be returned by XmlContentTok */
#define XML_TOK_TRAILING_RSQB -5 /* ] or ]] at the end of the scan; might be
                                    start of illegal ]]> sequence */
/* The following tokens may be returned by both XmlPrologTok and
   XmlContentTok.
*/
#define XML_TOK_NONE -4          /* The string to be scanned is empty */
#define XML_TOK_TRAILING_CR -3   /* A CR at the end of the scan;
                                    might be part of CRLF sequence */
#define XML_TOK_PARTIAL_CHAR -2  /* only part of a multibyte sequence */
#define XML_TOK_PARTIAL -1       /* only part of a token */
#define XML_TOK_INVALID 0

/* The following tokens are returned by XmlContentTok; some are also
   returned by XmlAttributeValueTok, XmlEntityTok, XmlCdataSectionTok.
#define XML_TOK_DATA_NEWLINE 7
#define XML_TOK_CDATA_SECT_OPEN 8
#define XML_TOK_ENTITY_REF 9
#define XML_TOK_CHAR_REF 10               /* numeric character reference */

/* The following tokens may be returned by both XmlPrologTok and
   XmlContentTok.
*/
#define XML_TOK_PI 11                     /* processing instruction */
#define XML_TOK_XML_DECL 12               /* XML decl or text decl */
#define XML_TOK_COMMENT 13
#define XML_TOK_BOM 14                    /* Byte order mark */

/* The following tokens are returned only by XmlPrologTok */
#define XML_TOK_PROLOG_S 15
#define XML_TOK_DECL_OPEN 16              /* <!foo */
#define XML_TOK_DECL_CLOSE 17             /* > */
#define XML_TOK_NAME 18
#define XML_TOK_NMTOKEN 19
#define XML_TOK_POUND_NAME 20             /* #name */
#define XML_TOK_OR 21                     /* | */
#define XML_TOK_PERCENT 22
#define XML_TOK_OPEN_PAREN 23
#define XML_TOK_CLOSE_PAREN 24
#define XML_TOK_OPEN_BRACKET 25
#define XML_TOK_INSTANCE_START 29

/* The following occur only in element type declarations */
#define XML_TOK_NAME_QUESTION 30          /* name? */
#define XML_TOK_NAME_ASTERISK 31          /* name* */
#define XML_TOK_NAME_PLUS 32              /* name+ */
#define XML_TOK_COND_SECT_OPEN 33         /* <![ */
#define XML_TOK_COND_SECT_CLOSE 34        /* ]]> */
#define XML_TOK_CLOSE_PAREN_QUESTION 35   /* )? */
#define XML_TOK_CLOSE_PAREN_ASTERISK 36   /* )* */
#define XML_TOK_CLOSE_PAREN_PLUS 37       /* )+ */
#define XML_TOK_COMMA 38

/* The following token is returned only by XmlAttributeValueTok */
#define XML_TOK_ATTRIBUTE_VALUE_S 39
#define XML_TOK_PREFIXED_NAME 41

#ifdef XML_DTD
#define XML_TOK_IGNORE_SECT 42
#endif /* XML_DTD */

#ifdef XML_DTD
#define XML_N_STATES 4
#else /* not XML_DTD */
#define XML_N_STATES 3
#endif /* not XML_DTD */

#define XML_PROLOG_STATE 0
#define XML_CONTENT_STATE 1
#define XML_CDATA_SECTION_STATE 2
#ifdef XML_DTD
#define XML_IGNORE_SECTION_STATE 3
#endif /* XML_DTD */

#define XML_N_LITERAL_TYPES 2
#define XML_ATTRIBUTE_VALUE_LITERAL 0
struct encoding;
typedef struct encoding ENCODING;

typedef int (PTRCALL *SCANNER)(const ENCODING *,
                               const char *,
                               const char *,
                               const char **);

enum XML_Convert_Result {
  XML_CONVERT_COMPLETED = 0,
  XML_CONVERT_INPUT_INCOMPLETE = 1,
  XML_CONVERT_OUTPUT_EXHAUSTED = 2  /* and therefore potentially input remaining as well */
};

struct encoding {
  SCANNER scanners[XML_N_STATES];
  SCANNER literalScanners[XML_N_LITERAL_TYPES];
  int (PTRCALL *nameMatchesAscii)(const ENCODING *,
                                  const char *,
                                  const char *,
                                  const char *);
  int (PTRFASTCALL *nameLength)(const ENCODING *, const char *);
  const char *(PTRFASTCALL *skipS)(const ENCODING *, const char *);
  int (PTRCALL *getAtts)(const ENCODING *enc,
                         const char *ptr,
                         int attsMax,
                         ATTRIBUTE *atts);
  int (PTRFASTCALL *charRefNumber)(const ENCODING *enc, const char *ptr);
  int (PTRCALL *predefinedEntityName)(const ENCODING *,
                                      const char *,
                                      const char *);
  void (PTRCALL *updatePosition)(const ENCODING *,
                                 const char *ptr,
                                 const char *end,
                                 POSITION *);
  int (PTRCALL *isPublicId)(const ENCODING *enc,
                            const char *ptr,
                            const char *end,
                            const char **badPtr);
  enum XML_Convert_Result (PTRCALL *utf8Convert)(const ENCODING *enc,
                              const char **fromP,
                              const char *fromLim,
                              char **toP,
                              const char *toLim);
  enum XML_Convert_Result (PTRCALL *utf16Convert)(const ENCODING *enc,
                               const char **fromP,
                               const char *fromLim,
                               unsigned short **toP,
                               const unsigned short *toLim);
  int minBytesPerChar;
  char isUtf8;
  char isUtf16;
};
   the prolog outside literals, comments and processing instructions.
*/


#define XmlTok(enc, state, ptr, end, nextTokPtr) \
  (((enc)->scanners[state])(enc, ptr, end, nextTokPtr))

#define XmlPrologTok(enc, ptr, end, nextTokPtr) \
   XmlTok(enc, XML_PROLOG_STATE, ptr, end, nextTokPtr)

#define XmlContentTok(enc, ptr, end, nextTokPtr) \
   XmlTok(enc, XML_CONTENT_STATE, ptr, end, nextTokPtr)

#define XmlCdataSectionTok(enc, ptr, end, nextTokPtr) \
   XmlTok(enc, XML_CDATA_SECTION_STATE, ptr, end, nextTokPtr)

#ifdef XML_DTD

#define XmlIgnoreSectionTok(enc, ptr, end, nextTokPtr) \
   XmlTok(enc, XML_IGNORE_SECTION_STATE, ptr, end, nextTokPtr)

#endif /* XML_DTD */

/* This is used for performing a 2nd-level tokenization on the content
   of a literal that has already been returned by XmlTok.
*/
#define XmlLiteralTok(enc, literalType, ptr, end, nextTokPtr) \
  (((enc)->literalScanners[literalType])(enc, ptr, end, nextTokPtr))

#define XmlAttributeValueTok(enc, ptr, end, nextTokPtr) \
   XmlLiteralTok(enc, XML_ATTRIBUTE_VALUE_LITERAL, ptr, end, nextTokPtr)

#define XmlEntityValueTok(enc, ptr, end, nextTokPtr) \
   XmlLiteralTok(enc, XML_ENTITY_VALUE_LITERAL, ptr, end, nextTokPtr)

#define XmlNameMatchesAscii(enc, ptr1, end1, ptr2) \
  (((enc)->nameMatchesAscii)(enc, ptr1, end1, ptr2))

#define XmlNameLength(enc, ptr) \
  (((enc)->nameLength)(enc, ptr))

#define XmlSkipS(enc, ptr) \
  (((enc)->skipS)(enc, ptr))

#define XmlGetAttributes(enc, ptr, attsMax, atts) \
  (((enc)->getAtts)(enc, ptr, attsMax, atts))

#define XmlCharRefNumber(enc, ptr) \
  (((enc)->charRefNumber)(enc, ptr))

#define XmlPredefinedEntityName(enc, ptr, end) \
  (((enc)->predefinedEntityName)(enc, ptr, end))

#define XmlUpdatePosition(enc, ptr, end, pos) \
  (((enc)->updatePosition)(enc, ptr, end, pos))

#define XmlIsPublicId(enc, ptr, end, badPtr) \
  (((enc)->isPublicId)(enc, ptr, end, badPtr))

#define XmlUtf8Convert(enc, fromP, fromLim, toP, toLim) \
  (((enc)->utf8Convert)(enc, fromP, fromLim, toP, toLim))

#define XmlUtf16Convert(enc, fromP, fromLim, toP, toLim) \
  (((enc)->utf16Convert)(enc, fromP, fromLim, toP, toLim))

typedef struct {
  ENCODING initEnc;
  const ENCODING **encPtr;
} INIT_ENCODING;

int XmlParseXmlDecl(int isGeneralTextEntity,
                    const ENCODING *enc,
                    const char *ptr,
                    const char *end,
                    const char **badPtr,
                    const char **versionPtr,
                    const char **versionEndPtr,
                    const char **encodingNamePtr,
                    const ENCODING **namedEncodingPtr,
                    int *standalonePtr);

int XmlInitEncoding(INIT_ENCODING *, const ENCODING **, const char *name);
const ENCODING *XmlGetUtf8InternalEncoding(void);
const ENCODING *XmlGetUtf16InternalEncoding(void);
int FASTCALL XmlUtf16Encode(int charNumber, unsigned short *buf);
int XmlSizeOfUnknownEncoding(void);


typedef int (XMLCALL *CONVERTER) (void *userData, const char *p);

ENCODING *
XmlInitUnknownEncoding(void *mem,
                       int *table,
                       CONVERTER convert,
                       void *userData);

int XmlParseXmlDeclNS(int isGeneralTextEntity,
                      const ENCODING *enc,
                      const char *ptr,
                      const char *end,
                      const char **badPtr,
                      const char **versionPtr,
                      const char **versionEndPtr,
                      const char **encodingNamePtr,
                      const ENCODING **namedEncodingPtr,
                      int *standalonePtr);

int XmlInitEncodingNS(INIT_ENCODING *, const ENCODING **, const char *name);
const ENCODING *XmlGetUtf8InternalEncodingNS(void);
const ENCODING *XmlGetUtf16InternalEncodingNS(void);
ENCODING *
XmlInitUnknownEncodingNS(void *mem,
                         int *table,
                         CONVERTER convert,
                         void *userData);
#ifdef __cplusplus
}
#endif
