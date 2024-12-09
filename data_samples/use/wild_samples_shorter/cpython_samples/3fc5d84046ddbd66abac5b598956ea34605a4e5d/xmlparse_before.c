/* f2d0ab6d1d4422a08cf1cf3bbdfba96b49dea42fb5ff4615e03a2a25c306e769 (2.2.8+)
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000-2017 Expat development team
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the

#ifdef _WIN32
/* force stdlib to define rand_s() */
#  define _CRT_RAND_S
#endif

#include <stddef.h>
#include <string.h> /* memset(), memcpy() */
#include <limits.h> /* UINT_MAX */
#include <stdio.h>  /* fprintf */
#include <stdlib.h> /* getenv, rand_s */

#ifdef _WIN32
#  define getpid GetCurrentProcessId
#else

#ifdef _WIN32
#  include "winconfig.h"
#elif defined(HAVE_EXPAT_CONFIG_H)
#  include <expat_config.h>
#endif /* ndef _WIN32 */

#include "ascii.h"
#include "expat.h"
#include "siphash.h"
    enabled.  For end user security, that is probably not what you want. \
    \
    Your options include: \
      * Linux + glibc >=2.25 (getrandom): HAVE_GETRANDOM, \
      * Linux + glibc <2.25 (syscall SYS_getrandom): HAVE_SYSCALL_GETRANDOM, \
      * BSD / macOS >=10.7 (arc4random_buf): HAVE_ARC4RANDOM_BUF, \
      * BSD / macOS <10.7 (arc4random): HAVE_ARC4RANDOM, \
      * libbsd (arc4random_buf): HAVE_ARC4RANDOM_BUF + HAVE_LIBBSD, \
      * libbsd (arc4random): HAVE_ARC4RANDOM + HAVE_LIBBSD, \
      * Linux / BSD / macOS (/dev/urandom): XML_DEV_URANDOM \
      * Windows (rand_s): _WIN32. \
    \
    If insist on not using any of these, bypass this error by defining \
    XML_POOR_ENTROPY; you have been warned. \
    \
#  define XmlGetInternalEncoding XmlGetUtf16InternalEncoding
#  define XmlGetInternalEncodingNS XmlGetUtf16InternalEncodingNS
#  define XmlEncode XmlUtf16Encode
/* Using pointer subtraction to convert to integer type. */
#  define MUST_CONVERT(enc, s)                                                 \
    (! (enc)->isUtf16 || (((char *)(s) - (char *)NULL) & 1))
typedef unsigned short ICHAR;
#else
#  define XML_ENCODE_MAX XML_UTF8_ENCODE_MAX
#  define XmlConvert XmlUtf8Convert
  XML_Bool betweenDecl; /* WFC: PE Between Declarations */
} OPEN_INTERNAL_ENTITY;

typedef enum XML_Error PTRCALL Processor(XML_Parser parser, const char *start,
                                         const char *end, const char **endPtr);

static Processor prologProcessor;
static enum XML_Error doProlog(XML_Parser parser, const ENCODING *enc,
                               const char *s, const char *end, int tok,
                               const char *next, const char **nextPtr,
                               XML_Bool haveMore, XML_Bool allowClosingDoctype);
static enum XML_Error processInternalEntity(XML_Parser parser, ENTITY *entity,
                                            XML_Bool betweenDecl);
static enum XML_Error doContent(XML_Parser parser, int startTagLevel,
                                const ENCODING *enc, const char *start,
                                const char *end, const char **endPtr,
                                XML_Bool haveMore);
static enum XML_Error doCdataSection(XML_Parser parser, const ENCODING *,
                                     const char **startPtr, const char *end,
                                     const char **nextPtr, XML_Bool haveMore);
#ifdef XML_DTD
static enum XML_Error doIgnoreSection(XML_Parser parser, const ENCODING *,
                                      const char **startPtr, const char *end,
                                      const char **nextPtr, XML_Bool haveMore);
static void freeBindings(XML_Parser parser, BINDING *bindings);
static enum XML_Error storeAtts(XML_Parser parser, const ENCODING *,
                                const char *s, TAG_NAME *tagNamePtr,
                                BINDING **bindingsPtr);
static enum XML_Error addBinding(XML_Parser parser, PREFIX *prefix,
                                 const ATTRIBUTE_ID *attId, const XML_Char *uri,
                                 BINDING **bindingsPtr);
static int defineAttribute(ELEMENT_TYPE *type, ATTRIBUTE_ID *, XML_Bool isCdata,
                           XML_Parser parser);
static enum XML_Error storeAttributeValue(XML_Parser parser, const ENCODING *,
                                          XML_Bool isCdata, const char *,
                                          const char *, STRING_POOL *);
static enum XML_Error appendAttributeValue(XML_Parser parser, const ENCODING *,
                                           XML_Bool isCdata, const char *,
                                           const char *, STRING_POOL *);
static ATTRIBUTE_ID *getAttributeId(XML_Parser parser, const ENCODING *enc,
                                    const char *start, const char *end);
static int setElementTypePrefix(XML_Parser parser, ELEMENT_TYPE *);
static enum XML_Error storeEntityValue(XML_Parser parser, const ENCODING *enc,
                                       const char *start, const char *end);
static int reportProcessingInstruction(XML_Parser parser, const ENCODING *enc,
                                       const char *start, const char *end);
static int reportComment(XML_Parser parser, const ENCODING *enc,
                         const char *start, const char *end);

static void parserInit(XML_Parser parser, const XML_Char *encodingName);

#define poolStart(pool) ((pool)->start)
#define poolEnd(pool) ((pool)->ptr)
#define poolLength(pool) ((pool)->ptr - (pool)->start)
#define poolChop(pool) ((void)--(pool->ptr))
  enum XML_ParamEntityParsing m_paramEntityParsing;
#endif
  unsigned long m_hash_secret_salt;
};

#define MALLOC(parser, s) (parser->m_mem.malloc_fcn((s)))
#define REALLOC(parser, p, s) (parser->m_mem.realloc_fcn((p), (s)))

#ifdef _WIN32

/* Obtain entropy on Windows using the rand_s() function which
 * generates cryptographically secure random numbers.  Internally it
 * uses RtlGenRandom API which is present in Windows XP and later.
 */

static unsigned long
ENTROPY_DEBUG(const char *label, unsigned long entropy) {
  const char *const EXPAT_ENTROPY_DEBUG = getenv("EXPAT_ENTROPY_DEBUG");
  if (EXPAT_ENTROPY_DEBUG && ! strcmp(EXPAT_ENTROPY_DEBUG, "1")) {
    fprintf(stderr, "Entropy: %s --> 0x%0*lx (%lu bytes)\n", label,
            (int)sizeof(entropy) * 2, entropy, (unsigned long)sizeof(entropy));
  }
  return entropy;
}
  parser->m_paramEntityParsing = XML_PARAM_ENTITY_PARSING_NEVER;
#endif
  parser->m_hash_secret_salt = 0;
}

/* moves list of bindings to m_freeBindingList */
static void FASTCALL
  parser->m_useForeignDTD = useDTD;
  return XML_ERROR_NONE;
#else
  return XML_ERROR_FEATURE_REQUIRES_XML_DTD;
#endif
}

    int nLeftOver;
    enum XML_Status result;
    /* Detect overflow (a+b > MAX <==> b > MAX-a) */
    if (len > ((XML_Size)-1) / 2 - parser->m_parseEndByteIndex) {
      parser->m_errorCode = XML_ERROR_NO_MEMORY;
      parser->m_eventPtr = parser->m_eventEndPtr = NULL;
      parser->m_processor = errorProcessor;
      return XML_STATUS_ERROR;
    parser->m_errorCode = XML_ERROR_FINISHED;
    return XML_STATUS_ERROR;
  case XML_INITIALIZED:
    if (parser->m_parentParser == NULL && ! startParsing(parser)) {
      parser->m_errorCode = XML_ERROR_NO_MEMORY;
      return XML_STATUS_ERROR;
    }
  (void)offset;
  (void)size;
#endif /* defined XML_CONTEXT_BYTES */
  return (char *)0;
}

XML_Size XMLCALL
XML_GetCurrentLineNumber(XML_Parser parser) {
  /* Added in 2.2.5. */
  case XML_ERROR_INVALID_ARGUMENT: /* Constant added in 2.2.1, already */
    return XML_L("invalid argument");
  }
  return NULL;
}


const XML_Feature *XMLCALL
XML_GetFeatureList(void) {
  static const XML_Feature features[]
      = {{XML_FEATURE_SIZEOF_XML_CHAR, XML_L("sizeof(XML_Char)"),
          sizeof(XML_Char)},
         {XML_FEATURE_SIZEOF_XML_LCHAR, XML_L("sizeof(XML_LChar)"),
          sizeof(XML_LChar)},
#ifdef XML_UNICODE
         {XML_FEATURE_UNICODE, XML_L("XML_UNICODE"), 0},
#endif
#ifdef XML_UNICODE_WCHAR_T
         {XML_FEATURE_UNICODE_WCHAR_T, XML_L("XML_UNICODE_WCHAR_T"), 0},
#endif
#ifdef XML_DTD
         {XML_FEATURE_DTD, XML_L("XML_DTD"), 0},
#endif
#ifdef XML_CONTEXT_BYTES
         {XML_FEATURE_CONTEXT_BYTES, XML_L("XML_CONTEXT_BYTES"),
          XML_CONTEXT_BYTES},
#endif
#ifdef XML_MIN_SIZE
         {XML_FEATURE_MIN_SIZE, XML_L("XML_MIN_SIZE"), 0},
#endif
#ifdef XML_NS
         {XML_FEATURE_NS, XML_L("XML_NS"), 0},
#endif
#ifdef XML_LARGE_SIZE
         {XML_FEATURE_LARGE_SIZE, XML_L("XML_LARGE_SIZE"), 0},
#endif
#ifdef XML_ATTR_INFO
         {XML_FEATURE_ATTR_INFO, XML_L("XML_ATTR_INFO"), 0},
#endif
         {XML_FEATURE_END, NULL, 0}};

  return features;
}

/* Initially tag->rawName always points into the parse buffer;
   for those TAG instances opened while the current parse buffer was
   processed, and not yet closed, we need to store tag->rawName in a more
   permanent location, since the parse buffer is about to be discarded.
static enum XML_Error PTRCALL
contentProcessor(XML_Parser parser, const char *start, const char *end,
                 const char **endPtr) {
  enum XML_Error result
      = doContent(parser, 0, parser->m_encoding, start, end, endPtr,
                  (XML_Bool)! parser->m_parsingStatus.finalBuffer);
  if (result == XML_ERROR_NONE) {
    if (! storeRawNames(parser))
      return XML_ERROR_NO_MEMORY;
  }
  int tok = XmlContentTok(parser->m_encoding, start, end, &next);
  switch (tok) {
  case XML_TOK_BOM:
    /* If we are at the end of the buffer, this would cause the next stage,
       i.e. externalEntityInitProcessor3, to pass control directly to
       doContent (by detecting XML_TOK_NONE) without processing any xml text
       declaration - causing the error XML_ERROR_MISPLACED_XML_PI in doContent.
  const char *next = start; /* XmlContentTok doesn't always set the last arg */
  parser->m_eventPtr = start;
  tok = XmlContentTok(parser->m_encoding, start, end, &next);
  parser->m_eventEndPtr = next;

  switch (tok) {
  case XML_TOK_XML_DECL: {
                               const char *end, const char **endPtr) {
  enum XML_Error result
      = doContent(parser, 1, parser->m_encoding, start, end, endPtr,
                  (XML_Bool)! parser->m_parsingStatus.finalBuffer);
  if (result == XML_ERROR_NONE) {
    if (! storeRawNames(parser))
      return XML_ERROR_NO_MEMORY;
  }
static enum XML_Error
doContent(XML_Parser parser, int startTagLevel, const ENCODING *enc,
          const char *s, const char *end, const char **nextPtr,
          XML_Bool haveMore) {
  /* save one level of indirection */
  DTD *const dtd = parser->m_dtd;

  const char **eventPP;
  for (;;) {
    const char *next = s; /* XmlContentTok doesn't always set the last arg */
    int tok = XmlContentTok(enc, s, end, &next);
    *eventEndPP = next;
    switch (tok) {
    case XML_TOK_TRAILING_CR:
      if (haveMore) {
      XML_Char ch = (XML_Char)XmlPredefinedEntityName(
          enc, s + enc->minBytesPerChar, next - enc->minBytesPerChar);
      if (ch) {
        if (parser->m_characterDataHandler)
          parser->m_characterDataHandler(parser->m_handlerArg, &ch, 1);
        else if (parser->m_defaultHandler)
          reportDefault(parser, enc, s, next);
      }
      tag->name.str = (XML_Char *)tag->buf;
      *toPtr = XML_T('\0');
      result = storeAtts(parser, enc, s, &(tag->name), &(tag->bindings));
      if (result)
        return result;
      if (parser->m_startElementHandler)
        parser->m_startElementHandler(parser->m_handlerArg, tag->name.str,
      if (! name.str)
        return XML_ERROR_NO_MEMORY;
      poolFinish(&parser->m_tempPool);
      result = storeAtts(parser, enc, s, &name, &bindings);
      if (result != XML_ERROR_NONE) {
        freeBindings(parser, bindings);
        return result;
      }
      /* END disabled code */
      else if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, next);
      result = doCdataSection(parser, enc, &next, end, nextPtr, haveMore);
      if (result != XML_ERROR_NONE)
        return result;
      else if (! next) {
        parser->m_processor = cdataSectionProcessor;
*/
static enum XML_Error
storeAtts(XML_Parser parser, const ENCODING *enc, const char *attStr,
          TAG_NAME *tagNamePtr, BINDING **bindingsPtr) {
  DTD *const dtd = parser->m_dtd; /* save one level of indirection */
  ELEMENT_TYPE *elementType;
  int nDefaultAtts;
  const XML_Char **appAtts; /* the attribute list for the application */
      /* normalize the attribute value */
      result = storeAttributeValue(
          parser, enc, isCdata, parser->m_atts[i].valuePtr,
          parser->m_atts[i].valueEnd, &parser->m_tempPool);
      if (result)
        return result;
      appAtts[attIndex] = poolStart(&parser->m_tempPool);
      poolFinish(&parser->m_tempPool);
static enum XML_Error PTRCALL
cdataSectionProcessor(XML_Parser parser, const char *start, const char *end,
                      const char **endPtr) {
  enum XML_Error result
      = doCdataSection(parser, parser->m_encoding, &start, end, endPtr,
                       (XML_Bool)! parser->m_parsingStatus.finalBuffer);
  if (result != XML_ERROR_NONE)
    return result;
  if (start) {
    if (parser->m_parentParser) { /* we are parsing an external entity */
*/
static enum XML_Error
doCdataSection(XML_Parser parser, const ENCODING *enc, const char **startPtr,
               const char *end, const char **nextPtr, XML_Bool haveMore) {
  const char *s = *startPtr;
  const char **eventPP;
  const char **eventEndPP;
  if (enc == parser->m_encoding) {
  *startPtr = NULL;

  for (;;) {
    const char *next;
    int tok = XmlCdataSectionTok(enc, s, end, &next);
    *eventEndPP = next;
    switch (tok) {
    case XML_TOK_CDATA_SECT_CLOSE:
      if (parser->m_endCdataSectionHandler)
static enum XML_Error
doIgnoreSection(XML_Parser parser, const ENCODING *enc, const char **startPtr,
                const char *end, const char **nextPtr, XML_Bool haveMore) {
  const char *next;
  int tok;
  const char *s = *startPtr;
  const char **eventPP;
  const char **eventEndPP;
  *eventPP = s;
  *startPtr = NULL;
  tok = XmlIgnoreSectionTok(enc, s, end, &next);
  *eventEndPP = next;
  switch (tok) {
  case XML_TOK_IGNORE_SECT:
    if (parser->m_defaultHandler)
  const char *versionend;
  const XML_Char *storedversion = NULL;
  int standalone = -1;
  if (! (parser->m_ns ? XmlParseXmlDeclNS : XmlParseXmlDecl)(
          isGeneralTextEntity, parser->m_encoding, s, next, &parser->m_eventPtr,
          &version, &versionend, &encodingName, &newEncoding, &standalone)) {
    if (isGeneralTextEntity)

  for (;;) {
    tok = XmlPrologTok(parser->m_encoding, start, end, &next);
    parser->m_eventEndPtr = next;
    if (tok <= 0) {
      if (! parser->m_parsingStatus.finalBuffer && tok != XML_TOK_INVALID) {
        *nextPtr = s;
        break;
      }
      /* found end of entity value - can store it now */
      return storeEntityValue(parser, parser->m_encoding, s, end);
    } else if (tok == XML_TOK_XML_DECL) {
      enum XML_Error result;
      result = processXmlDecl(parser, 0, start, next);
      if (result != XML_ERROR_NONE)
    */
    else if (tok == XML_TOK_BOM && next == end
             && ! parser->m_parsingStatus.finalBuffer) {
      *nextPtr = next;
      return XML_ERROR_NONE;
    }
    /* If we get this token, we have the start of what might be a
  }
  /* This would cause the next stage, i.e. doProlog to be passed XML_TOK_BOM.
     However, when parsing an external subset, doProlog will not accept a BOM
     as valid, and report a syntax error, so we have to skip the BOM
  */
  else if (tok == XML_TOK_BOM) {
    s = next;
    tok = XmlPrologTok(parser->m_encoding, s, end, &next);
  }

  parser->m_processor = prologProcessor;
  return doProlog(parser, parser->m_encoding, s, end, tok, next, nextPtr,
                  (XML_Bool)! parser->m_parsingStatus.finalBuffer, XML_TRUE);
}

static enum XML_Error PTRCALL
entityValueProcessor(XML_Parser parser, const char *s, const char *end,

  for (;;) {
    tok = XmlPrologTok(enc, start, end, &next);
    if (tok <= 0) {
      if (! parser->m_parsingStatus.finalBuffer && tok != XML_TOK_INVALID) {
        *nextPtr = s;
        return XML_ERROR_NONE;
        break;
      }
      /* found end of entity value - can store it now */
      return storeEntityValue(parser, enc, s, end);
    }
    start = next;
  }
}
  const char *next = s;
  int tok = XmlPrologTok(parser->m_encoding, s, end, &next);
  return doProlog(parser, parser->m_encoding, s, end, tok, next, nextPtr,
                  (XML_Bool)! parser->m_parsingStatus.finalBuffer, XML_TRUE);
}

static enum XML_Error
doProlog(XML_Parser parser, const ENCODING *enc, const char *s, const char *end,
         int tok, const char *next, const char **nextPtr, XML_Bool haveMore,
         XML_Bool allowClosingDoctype) {
#ifdef XML_DTD
  static const XML_Char externalSubsetName[] = {ASCII_HASH, '\0'};
#endif /* XML_DTD */
  static const XML_Char atypeCDATA[]
  static const XML_Char enumValueSep[] = {ASCII_PIPE, '\0'};
  static const XML_Char enumValueStart[] = {ASCII_LPAREN, '\0'};

  /* save one level of indirection */
  DTD *const dtd = parser->m_dtd;

  const char **eventPP;
      }
    }
    role = XmlTokenRole(&parser->m_prologState, tok, s, next, enc);
    switch (role) {
    case XML_ROLE_XML_DECL: {
      enum XML_Error result = processXmlDecl(parser, 0, s, next);
      if (result != XML_ERROR_NONE)
        const XML_Char *attVal;
        enum XML_Error result = storeAttributeValue(
            parser, enc, parser->m_declAttributeIsCdata,
            s + enc->minBytesPerChar, next - enc->minBytesPerChar, &dtd->pool);
        if (result)
          return result;
        attVal = poolStart(&dtd->pool);
        poolFinish(&dtd->pool);
      break;
    case XML_ROLE_ENTITY_VALUE:
      if (dtd->keepProcessing) {
        enum XML_Error result = storeEntityValue(
            parser, enc, s + enc->minBytesPerChar, next - enc->minBytesPerChar);
        if (parser->m_declEntity) {
          parser->m_declEntity->textPtr = poolStart(&dtd->entityValuePool);
          parser->m_declEntity->textLen
              = (int)(poolLength(&dtd->entityValuePool));
        if (parser->m_externalEntityRefHandler) {
          dtd->paramEntityRead = XML_FALSE;
          entity->open = XML_TRUE;
          if (! parser->m_externalEntityRefHandler(
                  parser->m_externalEntityRefHandlerArg, 0, entity->base,
                  entity->systemId, entity->publicId)) {
            entity->open = XML_FALSE;
            return XML_ERROR_EXTERNAL_ENTITY_HANDLING;
          }
          entity->open = XML_FALSE;
          handleDefault = XML_FALSE;
          if (! dtd->paramEntityRead) {
            dtd->keepProcessing = dtd->standalone;
  for (;;) {
    const char *next = NULL;
    int tok = XmlPrologTok(parser->m_encoding, s, end, &next);
    parser->m_eventEndPtr = next;
    switch (tok) {
    /* report partial linebreak - it might be the last token */
    case -XML_TOK_PROLOG_S:
      return XML_ERROR_NO_MEMORY;
  }
  entity->open = XML_TRUE;
  entity->processed = 0;
  openEntity->next = parser->m_openInternalEntities;
  parser->m_openInternalEntities = openEntity;
  openEntity->entity = entity;
  openEntity->betweenDecl = betweenDecl;
  openEntity->internalEventPtr = NULL;
  openEntity->internalEventEndPtr = NULL;
  textStart = (char *)entity->textPtr;
  textEnd = (char *)(entity->textPtr + entity->textLen);
  /* Set a safe default value in case 'next' does not get set */
  next = textStart;

#ifdef XML_DTD
    int tok
        = XmlPrologTok(parser->m_internalEncoding, textStart, textEnd, &next);
    result = doProlog(parser, parser->m_internalEncoding, textStart, textEnd,
                      tok, next, &next, XML_FALSE, XML_FALSE);
  } else
#endif /* XML_DTD */
    result = doContent(parser, parser->m_tagLevel, parser->m_internalEncoding,
                       textStart, textEnd, &next, XML_FALSE);

  if (result == XML_ERROR_NONE) {
    if (textEnd != next && parser->m_parsingStatus.parsing == XML_SUSPENDED) {
      entity->processed = (int)(next - textStart);
      parser->m_processor = internalEntityProcessor;
    } else {
      entity->open = XML_FALSE;
      parser->m_openInternalEntities = openEntity->next;
      /* put openEntity back in list of free instances */
      openEntity->next = parser->m_freeInternalEntities;
    return XML_ERROR_UNEXPECTED_STATE;

  entity = openEntity->entity;
  textStart = ((char *)entity->textPtr) + entity->processed;
  textEnd = (char *)(entity->textPtr + entity->textLen);
  /* Set a safe default value in case 'next' does not get set */
  next = textStart;

#ifdef XML_DTD
    int tok
        = XmlPrologTok(parser->m_internalEncoding, textStart, textEnd, &next);
    result = doProlog(parser, parser->m_internalEncoding, textStart, textEnd,
                      tok, next, &next, XML_FALSE, XML_TRUE);
  } else
#endif /* XML_DTD */
    result = doContent(parser, openEntity->startTagLevel,
                       parser->m_internalEncoding, textStart, textEnd, &next,
                       XML_FALSE);

  if (result != XML_ERROR_NONE)
    return result;
  else if (textEnd != next
           && parser->m_parsingStatus.parsing == XML_SUSPENDED) {
    entity->processed = (int)(next - (char *)entity->textPtr);
    return result;
  } else {
    entity->open = XML_FALSE;
    parser->m_openInternalEntities = openEntity->next;
    /* put openEntity back in list of free instances */
    openEntity->next = parser->m_freeInternalEntities;
    parser->m_processor = prologProcessor;
    tok = XmlPrologTok(parser->m_encoding, s, end, &next);
    return doProlog(parser, parser->m_encoding, s, end, tok, next, nextPtr,
                    (XML_Bool)! parser->m_parsingStatus.finalBuffer, XML_TRUE);
  } else
#endif /* XML_DTD */
  {
    parser->m_processor = contentProcessor;
    /* see externalEntityContentProcessor vs contentProcessor */
    return doContent(parser, parser->m_parentParser ? 1 : 0, parser->m_encoding,
                     s, end, nextPtr,
                     (XML_Bool)! parser->m_parsingStatus.finalBuffer);
  }
}

static enum XML_Error PTRCALL

static enum XML_Error
storeAttributeValue(XML_Parser parser, const ENCODING *enc, XML_Bool isCdata,
                    const char *ptr, const char *end, STRING_POOL *pool) {
  enum XML_Error result
      = appendAttributeValue(parser, enc, isCdata, ptr, end, pool);
  if (result)
    return result;
  if (! isCdata && poolLength(pool) && poolLastChar(pool) == 0x20)
    poolChop(pool);

static enum XML_Error
appendAttributeValue(XML_Parser parser, const ENCODING *enc, XML_Bool isCdata,
                     const char *ptr, const char *end, STRING_POOL *pool) {
  DTD *const dtd = parser->m_dtd; /* save one level of indirection */
  for (;;) {
    const char *next;
    int tok = XmlAttributeValueTok(enc, ptr, end, &next);
    switch (tok) {
    case XML_TOK_NONE:
      return XML_ERROR_NONE;
    case XML_TOK_INVALID:
      XML_Char ch = (XML_Char)XmlPredefinedEntityName(
          enc, ptr + enc->minBytesPerChar, next - enc->minBytesPerChar);
      if (ch) {
        if (! poolAppendChar(pool, ch))
          return XML_ERROR_NO_MEMORY;
        break;
      }
        enum XML_Error result;
        const XML_Char *textEnd = entity->textPtr + entity->textLen;
        entity->open = XML_TRUE;
        result = appendAttributeValue(parser, parser->m_internalEncoding,
                                      isCdata, (char *)entity->textPtr,
                                      (char *)textEnd, pool);
        entity->open = XML_FALSE;
        if (result)
          return result;
      }

static enum XML_Error
storeEntityValue(XML_Parser parser, const ENCODING *enc,
                 const char *entityTextPtr, const char *entityTextEnd) {
  DTD *const dtd = parser->m_dtd; /* save one level of indirection */
  STRING_POOL *pool = &(dtd->entityValuePool);
  enum XML_Error result = XML_ERROR_NONE;
#ifdef XML_DTD
  int oldInEntityValue = parser->m_prologState.inEntityValue;
  parser->m_prologState.inEntityValue = 1;
#endif /* XML_DTD */
  /* never return Null for the value argument in EntityDeclHandler,
     since this would indicate an external entity; therefore we
     have to make sure that entityValuePool.start is not null */
  }

  for (;;) {
    const char *next;
    int tok = XmlEntityValueTok(enc, entityTextPtr, entityTextEnd, &next);
    switch (tok) {
    case XML_TOK_PARAM_ENTITY_REF:
#ifdef XML_DTD
      if (parser->m_isParamEntity || enc != parser->m_encoding) {
          if (parser->m_externalEntityRefHandler) {
            dtd->paramEntityRead = XML_FALSE;
            entity->open = XML_TRUE;
            if (! parser->m_externalEntityRefHandler(
                    parser->m_externalEntityRefHandlerArg, 0, entity->base,
                    entity->systemId, entity->publicId)) {
              entity->open = XML_FALSE;
              result = XML_ERROR_EXTERNAL_ENTITY_HANDLING;
              goto endEntityValue;
            }
            entity->open = XML_FALSE;
            if (! dtd->paramEntityRead)
              dtd->keepProcessing = dtd->standalone;
          } else
            dtd->keepProcessing = dtd->standalone;
        } else {
          entity->open = XML_TRUE;
          result = storeEntityValue(
              parser, parser->m_internalEncoding, (char *)entity->textPtr,
              (char *)(entity->textPtr + entity->textLen));
          entity->open = XML_FALSE;
          if (result)
            goto endEntityValue;
        }
static void FASTCALL
hashTableIterInit(HASH_TABLE_ITER *iter, const HASH_TABLE *table) {
  iter->p = table->v;
  iter->end = iter->p + table->size;
}

static NAMED *FASTCALL
hashTableIterNext(HASH_TABLE_ITER *iter) {
  memcpy(result, s, charsRequired * sizeof(XML_Char));
  return result;
}