/* 8539b9040d9d901366a62560a064af7cb99811335784b363abc039c5b0ebc416 (2.4.1+)
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000      Clark Cooper <coopercc@users.sourceforge.net>
   Copyright (c) 2000-2006 Fred L. Drake, Jr. <fdrake@users.sourceforge.net>
   Copyright (c) 2001-2002 Greg Stein <gstein@users.sourceforge.net>
   Copyright (c) 2002-2016 Karl Waclawek <karl@waclawek.net>
   Copyright (c) 2005-2009 Steven Solie <ssolie@users.sourceforge.net>
   Copyright (c) 2016      Eric Rahm <erahm@mozilla.com>
   Copyright (c) 2016-2021 Sebastian Pipping <sebastian@pipping.org>
   Copyright (c) 2016      Gaurav <g.gupta@samsung.com>
   Copyright (c) 2016      Thomas Beutlich <tc@tbeu.de>
   Copyright (c) 2016      Gustavo Grieco <gustavo.grieco@imag.fr>
   Copyright (c) 2016      Pascal Cuoq <cuoq@trust-in-soft.com>
   Copyright (c) 2016      Ed Schouten <ed@nuxi.nl>
   Copyright (c) 2017-2018 Rhodri James <rhodri@wildebeest.org.uk>
   Copyright (c) 2017      Václav Slavík <vaclav@slavik.io>
   Copyright (c) 2017      Viktor Szakats <commit@vsz.me>
   Copyright (c) 2017      Chanho Park <chanho61.park@samsung.com>
   Copyright (c) 2017      Rolf Eike Beer <eike@sf-mail.de>
   Copyright (c) 2017      Hans Wennborg <hans@chromium.org>
   Copyright (c) 2018      Anton Maklakov <antmak.pub@gmail.com>
   Copyright (c) 2018      Benjamin Peterson <benjamin@python.org>
   Copyright (c) 2018      Marco Maggi <marco.maggi-ipsu@poste.it>
   Copyright (c) 2018      Mariusz Zaborski <oshogbo@vexillium.org>
   Copyright (c) 2019      David Loffredo <loffredo@steptools.com>
   Copyright (c) 2019-2020 Ben Wagner <bungeman@chromium.org>
   Copyright (c) 2019      Vadim Zeitlin <vadim@zeitlins.org>
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the

#ifdef _WIN32
/* force stdlib to define rand_s() */
#  if ! defined(_CRT_RAND_S)
#    define _CRT_RAND_S
#  endif
#endif

#include <stddef.h>
#include <string.h> /* memset(), memcpy() */
#include <limits.h> /* UINT_MAX */
#include <stdio.h>  /* fprintf */
#include <stdlib.h> /* getenv, rand_s */
#include <stdint.h> /* uintptr_t */
#include <math.h>   /* isnan */

#ifdef _WIN32
#  define getpid GetCurrentProcessId
#else

#ifdef _WIN32
#  include "winconfig.h"
#endif

#include <expat_config.h>

#include "ascii.h"
#include "expat.h"
#include "siphash.h"
    enabled.  For end user security, that is probably not what you want. \
    \
    Your options include: \
      * Linux >=3.17 + glibc >=2.25 (getrandom): HAVE_GETRANDOM, \
      * Linux >=3.17 + glibc (including <2.25) (syscall SYS_getrandom): HAVE_SYSCALL_GETRANDOM, \
      * BSD / macOS >=10.7 (arc4random_buf): HAVE_ARC4RANDOM_BUF, \
      * BSD / macOS (including <10.7) (arc4random): HAVE_ARC4RANDOM, \
      * libbsd (arc4random_buf): HAVE_ARC4RANDOM_BUF + HAVE_LIBBSD, \
      * libbsd (arc4random): HAVE_ARC4RANDOM + HAVE_LIBBSD, \
      * Linux (including <3.17) / BSD / macOS (including <10.7) (/dev/urandom): XML_DEV_URANDOM, \
      * Windows >=Vista (rand_s): _WIN32. \
    \
    If insist on not using any of these, bypass this error by defining \
    XML_POOR_ENTROPY; you have been warned. \
    \
#  define XmlGetInternalEncoding XmlGetUtf16InternalEncoding
#  define XmlGetInternalEncodingNS XmlGetUtf16InternalEncodingNS
#  define XmlEncode XmlUtf16Encode
#  define MUST_CONVERT(enc, s) (! (enc)->isUtf16 || (((uintptr_t)(s)) & 1))
typedef unsigned short ICHAR;
#else
#  define XML_ENCODE_MAX XML_UTF8_ENCODE_MAX
#  define XmlConvert XmlUtf8Convert
  XML_Bool betweenDecl; /* WFC: PE Between Declarations */
} OPEN_INTERNAL_ENTITY;

enum XML_Account {
  XML_ACCOUNT_DIRECT,           /* bytes directly passed to the Expat parser */
  XML_ACCOUNT_ENTITY_EXPANSION, /* intermediate bytes produced during entity
                                   expansion */
  XML_ACCOUNT_NONE              /* i.e. do not account, was accounted already */
};

#ifdef XML_DTD
typedef unsigned long long XmlBigCount;
typedef struct accounting {
  XmlBigCount countBytesDirect;
  XmlBigCount countBytesIndirect;
  int debugLevel;
  float maximumAmplificationFactor; // >=1.0
  unsigned long long activationThresholdBytes;
} ACCOUNTING;

typedef struct entity_stats {
  unsigned int countEverOpened;
  unsigned int currentDepth;
  unsigned int maximumDepthSeen;
  int debugLevel;
} ENTITY_STATS;
#endif /* XML_DTD */

typedef enum XML_Error PTRCALL Processor(XML_Parser parser, const char *start,
                                         const char *end, const char **endPtr);

static Processor prologProcessor;
static enum XML_Error doProlog(XML_Parser parser, const ENCODING *enc,
                               const char *s, const char *end, int tok,
                               const char *next, const char **nextPtr,
                               XML_Bool haveMore, XML_Bool allowClosingDoctype,
                               enum XML_Account account);
static enum XML_Error processInternalEntity(XML_Parser parser, ENTITY *entity,
                                            XML_Bool betweenDecl);
static enum XML_Error doContent(XML_Parser parser, int startTagLevel,
                                const ENCODING *enc, const char *start,
                                const char *end, const char **endPtr,
                                XML_Bool haveMore, enum XML_Account account);
static enum XML_Error doCdataSection(XML_Parser parser, const ENCODING *,
                                     const char **startPtr, const char *end,
                                     const char **nextPtr, XML_Bool haveMore,
                                     enum XML_Account account);
#ifdef XML_DTD
static enum XML_Error doIgnoreSection(XML_Parser parser, const ENCODING *,
                                      const char **startPtr, const char *end,
                                      const char **nextPtr, XML_Bool haveMore);
static void freeBindings(XML_Parser parser, BINDING *bindings);
static enum XML_Error storeAtts(XML_Parser parser, const ENCODING *,
                                const char *s, TAG_NAME *tagNamePtr,
                                BINDING **bindingsPtr,
                                enum XML_Account account);
static enum XML_Error addBinding(XML_Parser parser, PREFIX *prefix,
                                 const ATTRIBUTE_ID *attId, const XML_Char *uri,
                                 BINDING **bindingsPtr);
static int defineAttribute(ELEMENT_TYPE *type, ATTRIBUTE_ID *, XML_Bool isCdata,
                           XML_Parser parser);
static enum XML_Error storeAttributeValue(XML_Parser parser, const ENCODING *,
                                          XML_Bool isCdata, const char *,
                                          const char *, STRING_POOL *,
                                          enum XML_Account account);
static enum XML_Error appendAttributeValue(XML_Parser parser, const ENCODING *,
                                           XML_Bool isCdata, const char *,
                                           const char *, STRING_POOL *,
                                           enum XML_Account account);
static ATTRIBUTE_ID *getAttributeId(XML_Parser parser, const ENCODING *enc,
                                    const char *start, const char *end);
static int setElementTypePrefix(XML_Parser parser, ELEMENT_TYPE *);
static enum XML_Error storeEntityValue(XML_Parser parser, const ENCODING *enc,
                                       const char *start, const char *end,
                                       enum XML_Account account);
static int reportProcessingInstruction(XML_Parser parser, const ENCODING *enc,
                                       const char *start, const char *end);
static int reportComment(XML_Parser parser, const ENCODING *enc,
                         const char *start, const char *end);

static void parserInit(XML_Parser parser, const XML_Char *encodingName);

#ifdef XML_DTD
static float accountingGetCurrentAmplification(XML_Parser rootParser);
static void accountingReportStats(XML_Parser originParser, const char *epilog);
static void accountingOnAbort(XML_Parser originParser);
static void accountingReportDiff(XML_Parser rootParser,
                                 unsigned int levelsAwayFromRootParser,
                                 const char *before, const char *after,
                                 ptrdiff_t bytesMore, int source_line,
                                 enum XML_Account account);
static XML_Bool accountingDiffTolerated(XML_Parser originParser, int tok,
                                        const char *before, const char *after,
                                        int source_line,
                                        enum XML_Account account);

static void entityTrackingReportStats(XML_Parser parser, ENTITY *entity,
                                      const char *action, int sourceLine);
static void entityTrackingOnOpen(XML_Parser parser, ENTITY *entity,
                                 int sourceLine);
static void entityTrackingOnClose(XML_Parser parser, ENTITY *entity,
                                  int sourceLine);

static XML_Parser getRootParserOf(XML_Parser parser,
                                  unsigned int *outLevelDiff);
#endif /* XML_DTD */

static unsigned long getDebugLevel(const char *variableName,
                                   unsigned long defaultDebugLevel);

#define poolStart(pool) ((pool)->start)
#define poolEnd(pool) ((pool)->ptr)
#define poolLength(pool) ((pool)->ptr - (pool)->start)
#define poolChop(pool) ((void)--(pool->ptr))
  enum XML_ParamEntityParsing m_paramEntityParsing;
#endif
  unsigned long m_hash_secret_salt;
#ifdef XML_DTD
  ACCOUNTING m_accounting;
  ENTITY_STATS m_entity_stats;
#endif
};

#define MALLOC(parser, s) (parser->m_mem.malloc_fcn((s)))
#define REALLOC(parser, p, s) (parser->m_mem.realloc_fcn((p), (s)))

#ifdef _WIN32

/* Provide declaration of rand_s() for MinGW-32 (not 64, which has it),
   as it didn't declare it in its header prior to version 5.3.0 of its
   runtime package (mingwrt, containing stdlib.h).  The upstream fix
   was introduced at https://osdn.net/projects/mingw/ticket/39658 . */
#  if defined(__MINGW32__) && defined(__MINGW32_VERSION)                       \
      && __MINGW32_VERSION < 5003000L && ! defined(__MINGW64_VERSION_MAJOR)
__declspec(dllimport) int rand_s(unsigned int *);
#  endif

/* Obtain entropy on Windows using the rand_s() function which
 * generates cryptographically secure random numbers.  Internally it
 * uses RtlGenRandom API which is present in Windows XP and later.
 */

static unsigned long
ENTROPY_DEBUG(const char *label, unsigned long entropy) {
  if (getDebugLevel("EXPAT_ENTROPY_DEBUG", 0) >= 1u) {
    fprintf(stderr, "expat: Entropy: %s --> 0x%0*lx (%lu bytes)\n", label,
            (int)sizeof(entropy) * 2, entropy, (unsigned long)sizeof(entropy));
  }
  return entropy;
}
  parser->m_paramEntityParsing = XML_PARAM_ENTITY_PARSING_NEVER;
#endif
  parser->m_hash_secret_salt = 0;

#ifdef XML_DTD
  memset(&parser->m_accounting, 0, sizeof(ACCOUNTING));
  parser->m_accounting.debugLevel = getDebugLevel("EXPAT_ACCOUNTING_DEBUG", 0u);
  parser->m_accounting.maximumAmplificationFactor
      = EXPAT_BILLION_LAUGHS_ATTACK_PROTECTION_MAXIMUM_AMPLIFICATION_DEFAULT;
  parser->m_accounting.activationThresholdBytes
      = EXPAT_BILLION_LAUGHS_ATTACK_PROTECTION_ACTIVATION_THRESHOLD_DEFAULT;

  memset(&parser->m_entity_stats, 0, sizeof(ENTITY_STATS));
  parser->m_entity_stats.debugLevel = getDebugLevel("EXPAT_ENTITY_DEBUG", 0u);
#endif
}

/* moves list of bindings to m_freeBindingList */
static void FASTCALL
  parser->m_useForeignDTD = useDTD;
  return XML_ERROR_NONE;
#else
  UNUSED_P(useDTD);
  return XML_ERROR_FEATURE_REQUIRES_XML_DTD;
#endif
}

    int nLeftOver;
    enum XML_Status result;
    /* Detect overflow (a+b > MAX <==> b > MAX-a) */
    if ((XML_Size)len > ((XML_Size)-1) / 2 - parser->m_parseEndByteIndex) {
      parser->m_errorCode = XML_ERROR_NO_MEMORY;
      parser->m_eventPtr = parser->m_eventEndPtr = NULL;
      parser->m_processor = errorProcessor;
      return XML_STATUS_ERROR;
    parser->m_errorCode = XML_ERROR_FINISHED;
    return XML_STATUS_ERROR;
  case XML_INITIALIZED:
    /* Has someone called XML_GetBuffer successfully before? */
    if (! parser->m_bufferPtr) {
      parser->m_errorCode = XML_ERROR_NO_BUFFER;
      return XML_STATUS_ERROR;
    }

    if (parser->m_parentParser == NULL && ! startParsing(parser)) {
      parser->m_errorCode = XML_ERROR_NO_MEMORY;
      return XML_STATUS_ERROR;
    }
  (void)offset;
  (void)size;
#endif /* defined XML_CONTEXT_BYTES */
  return (const char *)0;
}

XML_Size XMLCALL
XML_GetCurrentLineNumber(XML_Parser parser) {
  /* Added in 2.2.5. */
  case XML_ERROR_INVALID_ARGUMENT: /* Constant added in 2.2.1, already */
    return XML_L("invalid argument");
    /* Added in 2.3.0. */
  case XML_ERROR_NO_BUFFER:
    return XML_L(
        "a successful prior call to function XML_GetBuffer is required");
  /* Added in 2.4.0. */
  case XML_ERROR_AMPLIFICATION_LIMIT_BREACH:
    return XML_L(
        "limit on input amplification factor (from DTD and entities) breached");
  }
  return NULL;
}


const XML_Feature *XMLCALL
XML_GetFeatureList(void) {
  static const XML_Feature features[] = {
      {XML_FEATURE_SIZEOF_XML_CHAR, XML_L("sizeof(XML_Char)"),
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
#ifdef XML_DTD
      /* Added in Expat 2.4.0. */
      {XML_FEATURE_BILLION_LAUGHS_ATTACK_PROTECTION_MAXIMUM_AMPLIFICATION_DEFAULT,
       XML_L("XML_BLAP_MAX_AMP"),
       (long int)
           EXPAT_BILLION_LAUGHS_ATTACK_PROTECTION_MAXIMUM_AMPLIFICATION_DEFAULT},
      {XML_FEATURE_BILLION_LAUGHS_ATTACK_PROTECTION_ACTIVATION_THRESHOLD_DEFAULT,
       XML_L("XML_BLAP_ACT_THRES"),
       EXPAT_BILLION_LAUGHS_ATTACK_PROTECTION_ACTIVATION_THRESHOLD_DEFAULT},
#endif
      {XML_FEATURE_END, NULL, 0}};

  return features;
}

#ifdef XML_DTD
XML_Bool XMLCALL
XML_SetBillionLaughsAttackProtectionMaximumAmplification(
    XML_Parser parser, float maximumAmplificationFactor) {
  if ((parser == NULL) || (parser->m_parentParser != NULL)
      || isnan(maximumAmplificationFactor)
      || (maximumAmplificationFactor < 1.0f)) {
    return XML_FALSE;
  }
  parser->m_accounting.maximumAmplificationFactor = maximumAmplificationFactor;
  return XML_TRUE;
}

XML_Bool XMLCALL
XML_SetBillionLaughsAttackProtectionActivationThreshold(
    XML_Parser parser, unsigned long long activationThresholdBytes) {
  if ((parser == NULL) || (parser->m_parentParser != NULL)) {
    return XML_FALSE;
  }
  parser->m_accounting.activationThresholdBytes = activationThresholdBytes;
  return XML_TRUE;
}
#endif /* XML_DTD */

/* Initially tag->rawName always points into the parse buffer;
   for those TAG instances opened while the current parse buffer was
   processed, and not yet closed, we need to store tag->rawName in a more
   permanent location, since the parse buffer is about to be discarded.
static enum XML_Error PTRCALL
contentProcessor(XML_Parser parser, const char *start, const char *end,
                 const char **endPtr) {
  enum XML_Error result = doContent(
      parser, 0, parser->m_encoding, start, end, endPtr,
      (XML_Bool)! parser->m_parsingStatus.finalBuffer, XML_ACCOUNT_DIRECT);
  if (result == XML_ERROR_NONE) {
    if (! storeRawNames(parser))
      return XML_ERROR_NO_MEMORY;
  }
  int tok = XmlContentTok(parser->m_encoding, start, end, &next);
  switch (tok) {
  case XML_TOK_BOM:
#ifdef XML_DTD
    if (! accountingDiffTolerated(parser, tok, start, next, __LINE__,
                                  XML_ACCOUNT_DIRECT)) {
      accountingOnAbort(parser);
      return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
    }
#endif /* XML_DTD */

    /* If we are at the end of the buffer, this would cause the next stage,
       i.e. externalEntityInitProcessor3, to pass control directly to
       doContent (by detecting XML_TOK_NONE) without processing any xml text
       declaration - causing the error XML_ERROR_MISPLACED_XML_PI in doContent.
  const char *next = start; /* XmlContentTok doesn't always set the last arg */
  parser->m_eventPtr = start;
  tok = XmlContentTok(parser->m_encoding, start, end, &next);
  /* Note: These bytes are accounted later in:
           - processXmlDecl
           - externalEntityContentProcessor
  */
  parser->m_eventEndPtr = next;

  switch (tok) {
  case XML_TOK_XML_DECL: {
                               const char *end, const char **endPtr) {
  enum XML_Error result
      = doContent(parser, 1, parser->m_encoding, start, end, endPtr,
                  (XML_Bool)! parser->m_parsingStatus.finalBuffer,
                  XML_ACCOUNT_ENTITY_EXPANSION);
  if (result == XML_ERROR_NONE) {
    if (! storeRawNames(parser))
      return XML_ERROR_NO_MEMORY;
  }
static enum XML_Error
doContent(XML_Parser parser, int startTagLevel, const ENCODING *enc,
          const char *s, const char *end, const char **nextPtr,
          XML_Bool haveMore, enum XML_Account account) {
  /* save one level of indirection */
  DTD *const dtd = parser->m_dtd;

  const char **eventPP;
  for (;;) {
    const char *next = s; /* XmlContentTok doesn't always set the last arg */
    int tok = XmlContentTok(enc, s, end, &next);
#ifdef XML_DTD
    const char *accountAfter
        = ((tok == XML_TOK_TRAILING_RSQB) || (tok == XML_TOK_TRAILING_CR))
              ? (haveMore ? s /* i.e. 0 bytes */ : end)
              : next;
    if (! accountingDiffTolerated(parser, tok, s, accountAfter, __LINE__,
                                  account)) {
      accountingOnAbort(parser);
      return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
    }
#endif
    *eventEndPP = next;
    switch (tok) {
    case XML_TOK_TRAILING_CR:
      if (haveMore) {
      XML_Char ch = (XML_Char)XmlPredefinedEntityName(
          enc, s + enc->minBytesPerChar, next - enc->minBytesPerChar);
      if (ch) {
#ifdef XML_DTD
        /* NOTE: We are replacing 4-6 characters original input for 1 character
         *       so there is no amplification and hence recording without
         *       protection. */
        accountingDiffTolerated(parser, tok, (char *)&ch,
                                ((char *)&ch) + sizeof(XML_Char), __LINE__,
                                XML_ACCOUNT_ENTITY_EXPANSION);
#endif /* XML_DTD */
        if (parser->m_characterDataHandler)
          parser->m_characterDataHandler(parser->m_handlerArg, &ch, 1);
        else if (parser->m_defaultHandler)
          reportDefault(parser, enc, s, next);
      }
      tag->name.str = (XML_Char *)tag->buf;
      *toPtr = XML_T('\0');
      result
          = storeAtts(parser, enc, s, &(tag->name), &(tag->bindings), account);
      if (result)
        return result;
      if (parser->m_startElementHandler)
        parser->m_startElementHandler(parser->m_handlerArg, tag->name.str,
      if (! name.str)
        return XML_ERROR_NO_MEMORY;
      poolFinish(&parser->m_tempPool);
      result = storeAtts(parser, enc, s, &name, &bindings,
                         XML_ACCOUNT_NONE /* token spans whole start tag */);
      if (result != XML_ERROR_NONE) {
        freeBindings(parser, bindings);
        return result;
      }
      /* END disabled code */
      else if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, next);
      result
          = doCdataSection(parser, enc, &next, end, nextPtr, haveMore, account);
      if (result != XML_ERROR_NONE)
        return result;
      else if (! next) {
        parser->m_processor = cdataSectionProcessor;
*/
static enum XML_Error
storeAtts(XML_Parser parser, const ENCODING *enc, const char *attStr,
          TAG_NAME *tagNamePtr, BINDING **bindingsPtr,
          enum XML_Account account) {
  DTD *const dtd = parser->m_dtd; /* save one level of indirection */
  ELEMENT_TYPE *elementType;
  int nDefaultAtts;
  const XML_Char **appAtts; /* the attribute list for the application */
      /* normalize the attribute value */
      result = storeAttributeValue(
          parser, enc, isCdata, parser->m_atts[i].valuePtr,
          parser->m_atts[i].valueEnd, &parser->m_tempPool, account);
      if (result)
        return result;
      appAtts[attIndex] = poolStart(&parser->m_tempPool);
      poolFinish(&parser->m_tempPool);
static enum XML_Error PTRCALL
cdataSectionProcessor(XML_Parser parser, const char *start, const char *end,
                      const char **endPtr) {
  enum XML_Error result = doCdataSection(
      parser, parser->m_encoding, &start, end, endPtr,
      (XML_Bool)! parser->m_parsingStatus.finalBuffer, XML_ACCOUNT_DIRECT);
  if (result != XML_ERROR_NONE)
    return result;
  if (start) {
    if (parser->m_parentParser) { /* we are parsing an external entity */
*/
static enum XML_Error
doCdataSection(XML_Parser parser, const ENCODING *enc, const char **startPtr,
               const char *end, const char **nextPtr, XML_Bool haveMore,
               enum XML_Account account) {
  const char *s = *startPtr;
  const char **eventPP;
  const char **eventEndPP;
  if (enc == parser->m_encoding) {
  *startPtr = NULL;

  for (;;) {
    const char *next = s; /* in case of XML_TOK_NONE or XML_TOK_PARTIAL */
    int tok = XmlCdataSectionTok(enc, s, end, &next);
#ifdef XML_DTD
    if (! accountingDiffTolerated(parser, tok, s, next, __LINE__, account)) {
      accountingOnAbort(parser);
      return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
    }
#else
    UNUSED_P(account);
#endif
    *eventEndPP = next;
    switch (tok) {
    case XML_TOK_CDATA_SECT_CLOSE:
      if (parser->m_endCdataSectionHandler)
static enum XML_Error
doIgnoreSection(XML_Parser parser, const ENCODING *enc, const char **startPtr,
                const char *end, const char **nextPtr, XML_Bool haveMore) {
  const char *next = *startPtr; /* in case of XML_TOK_NONE or XML_TOK_PARTIAL */
  int tok;
  const char *s = *startPtr;
  const char **eventPP;
  const char **eventEndPP;
  *eventPP = s;
  *startPtr = NULL;
  tok = XmlIgnoreSectionTok(enc, s, end, &next);
#  ifdef XML_DTD
  if (! accountingDiffTolerated(parser, tok, s, next, __LINE__,
                                XML_ACCOUNT_DIRECT)) {
    accountingOnAbort(parser);
    return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
  }
#  endif
  *eventEndPP = next;
  switch (tok) {
  case XML_TOK_IGNORE_SECT:
    if (parser->m_defaultHandler)
  const char *versionend;
  const XML_Char *storedversion = NULL;
  int standalone = -1;

#ifdef XML_DTD
  if (! accountingDiffTolerated(parser, XML_TOK_XML_DECL, s, next, __LINE__,
                                XML_ACCOUNT_DIRECT)) {
    accountingOnAbort(parser);
    return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
  }
#endif

  if (! (parser->m_ns ? XmlParseXmlDeclNS : XmlParseXmlDecl)(
          isGeneralTextEntity, parser->m_encoding, s, next, &parser->m_eventPtr,
          &version, &versionend, &encodingName, &newEncoding, &standalone)) {
    if (isGeneralTextEntity)

  for (;;) {
    tok = XmlPrologTok(parser->m_encoding, start, end, &next);
    /* Note: Except for XML_TOK_BOM below, these bytes are accounted later in:
             - storeEntityValue
             - processXmlDecl
    */
    parser->m_eventEndPtr = next;
    if (tok <= 0) {
      if (! parser->m_parsingStatus.finalBuffer && tok != XML_TOK_INVALID) {
        *nextPtr = s;
        break;
      }
      /* found end of entity value - can store it now */
      return storeEntityValue(parser, parser->m_encoding, s, end,
                              XML_ACCOUNT_DIRECT);
    } else if (tok == XML_TOK_XML_DECL) {
      enum XML_Error result;
      result = processXmlDecl(parser, 0, start, next);
      if (result != XML_ERROR_NONE)
    */
    else if (tok == XML_TOK_BOM && next == end
             && ! parser->m_parsingStatus.finalBuffer) {
#  ifdef XML_DTD
      if (! accountingDiffTolerated(parser, tok, s, next, __LINE__,
                                    XML_ACCOUNT_DIRECT)) {
        accountingOnAbort(parser);
        return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
      }
#  endif

      *nextPtr = next;
      return XML_ERROR_NONE;
    }
    /* If we get this token, we have the start of what might be a
  }
  /* This would cause the next stage, i.e. doProlog to be passed XML_TOK_BOM.
     However, when parsing an external subset, doProlog will not accept a BOM
     as valid, and report a syntax error, so we have to skip the BOM, and
     account for the BOM bytes.
  */
  else if (tok == XML_TOK_BOM) {
    if (! accountingDiffTolerated(parser, tok, s, next, __LINE__,
                                  XML_ACCOUNT_DIRECT)) {
      accountingOnAbort(parser);
      return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
    }

    s = next;
    tok = XmlPrologTok(parser->m_encoding, s, end, &next);
  }

  parser->m_processor = prologProcessor;
  return doProlog(parser, parser->m_encoding, s, end, tok, next, nextPtr,
                  (XML_Bool)! parser->m_parsingStatus.finalBuffer, XML_TRUE,
                  XML_ACCOUNT_DIRECT);
}

static enum XML_Error PTRCALL
entityValueProcessor(XML_Parser parser, const char *s, const char *end,

  for (;;) {
    tok = XmlPrologTok(enc, start, end, &next);
    /* Note: These bytes are accounted later in:
             - storeEntityValue
    */
    if (tok <= 0) {
      if (! parser->m_parsingStatus.finalBuffer && tok != XML_TOK_INVALID) {
        *nextPtr = s;
        return XML_ERROR_NONE;
        break;
      }
      /* found end of entity value - can store it now */
      return storeEntityValue(parser, enc, s, end, XML_ACCOUNT_DIRECT);
    }
    start = next;
  }
}
  const char *next = s;
  int tok = XmlPrologTok(parser->m_encoding, s, end, &next);
  return doProlog(parser, parser->m_encoding, s, end, tok, next, nextPtr,
                  (XML_Bool)! parser->m_parsingStatus.finalBuffer, XML_TRUE,
                  XML_ACCOUNT_DIRECT);
}

static enum XML_Error
doProlog(XML_Parser parser, const ENCODING *enc, const char *s, const char *end,
         int tok, const char *next, const char **nextPtr, XML_Bool haveMore,
         XML_Bool allowClosingDoctype, enum XML_Account account) {
#ifdef XML_DTD
  static const XML_Char externalSubsetName[] = {ASCII_HASH, '\0'};
#endif /* XML_DTD */
  static const XML_Char atypeCDATA[]
  static const XML_Char enumValueSep[] = {ASCII_PIPE, '\0'};
  static const XML_Char enumValueStart[] = {ASCII_LPAREN, '\0'};

#ifndef XML_DTD
  UNUSED_P(account);
#endif

  /* save one level of indirection */
  DTD *const dtd = parser->m_dtd;

  const char **eventPP;
      }
    }
    role = XmlTokenRole(&parser->m_prologState, tok, s, next, enc);
#ifdef XML_DTD
    switch (role) {
    case XML_ROLE_INSTANCE_START: // bytes accounted in contentProcessor
    case XML_ROLE_XML_DECL:       // bytes accounted in processXmlDecl
    case XML_ROLE_TEXT_DECL:      // bytes accounted in processXmlDecl
      break;
    default:
      if (! accountingDiffTolerated(parser, tok, s, next, __LINE__, account)) {
        accountingOnAbort(parser);
        return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
      }
    }
#endif
    switch (role) {
    case XML_ROLE_XML_DECL: {
      enum XML_Error result = processXmlDecl(parser, 0, s, next);
      if (result != XML_ERROR_NONE)
        const XML_Char *attVal;
        enum XML_Error result = storeAttributeValue(
            parser, enc, parser->m_declAttributeIsCdata,
            s + enc->minBytesPerChar, next - enc->minBytesPerChar, &dtd->pool,
            XML_ACCOUNT_NONE);
        if (result)
          return result;
        attVal = poolStart(&dtd->pool);
        poolFinish(&dtd->pool);
      break;
    case XML_ROLE_ENTITY_VALUE:
      if (dtd->keepProcessing) {
        enum XML_Error result
            = storeEntityValue(parser, enc, s + enc->minBytesPerChar,
                               next - enc->minBytesPerChar, XML_ACCOUNT_NONE);
        if (parser->m_declEntity) {
          parser->m_declEntity->textPtr = poolStart(&dtd->entityValuePool);
          parser->m_declEntity->textLen
              = (int)(poolLength(&dtd->entityValuePool));
        if (parser->m_externalEntityRefHandler) {
          dtd->paramEntityRead = XML_FALSE;
          entity->open = XML_TRUE;
          entityTrackingOnOpen(parser, entity, __LINE__);
          if (! parser->m_externalEntityRefHandler(
                  parser->m_externalEntityRefHandlerArg, 0, entity->base,
                  entity->systemId, entity->publicId)) {
            entityTrackingOnClose(parser, entity, __LINE__);
            entity->open = XML_FALSE;
            return XML_ERROR_EXTERNAL_ENTITY_HANDLING;
          }
          entityTrackingOnClose(parser, entity, __LINE__);
          entity->open = XML_FALSE;
          handleDefault = XML_FALSE;
          if (! dtd->paramEntityRead) {
            dtd->keepProcessing = dtd->standalone;
  for (;;) {
    const char *next = NULL;
    int tok = XmlPrologTok(parser->m_encoding, s, end, &next);
#ifdef XML_DTD
    if (! accountingDiffTolerated(parser, tok, s, next, __LINE__,
                                  XML_ACCOUNT_DIRECT)) {
      accountingOnAbort(parser);
      return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
    }
#endif
    parser->m_eventEndPtr = next;
    switch (tok) {
    /* report partial linebreak - it might be the last token */
    case -XML_TOK_PROLOG_S:
      return XML_ERROR_NO_MEMORY;
  }
  entity->open = XML_TRUE;
#ifdef XML_DTD
  entityTrackingOnOpen(parser, entity, __LINE__);
#endif
  entity->processed = 0;
  openEntity->next = parser->m_openInternalEntities;
  parser->m_openInternalEntities = openEntity;
  openEntity->entity = entity;
  openEntity->betweenDecl = betweenDecl;
  openEntity->internalEventPtr = NULL;
  openEntity->internalEventEndPtr = NULL;
  textStart = (const char *)entity->textPtr;
  textEnd = (const char *)(entity->textPtr + entity->textLen);
  /* Set a safe default value in case 'next' does not get set */
  next = textStart;

#ifdef XML_DTD
    int tok
        = XmlPrologTok(parser->m_internalEncoding, textStart, textEnd, &next);
    result = doProlog(parser, parser->m_internalEncoding, textStart, textEnd,
                      tok, next, &next, XML_FALSE, XML_FALSE,
                      XML_ACCOUNT_ENTITY_EXPANSION);
  } else
#endif /* XML_DTD */
    result = doContent(parser, parser->m_tagLevel, parser->m_internalEncoding,
                       textStart, textEnd, &next, XML_FALSE,
                       XML_ACCOUNT_ENTITY_EXPANSION);

  if (result == XML_ERROR_NONE) {
    if (textEnd != next && parser->m_parsingStatus.parsing == XML_SUSPENDED) {
      entity->processed = (int)(next - textStart);
      parser->m_processor = internalEntityProcessor;
    } else {
#ifdef XML_DTD
      entityTrackingOnClose(parser, entity, __LINE__);
#endif /* XML_DTD */
      entity->open = XML_FALSE;
      parser->m_openInternalEntities = openEntity->next;
      /* put openEntity back in list of free instances */
      openEntity->next = parser->m_freeInternalEntities;
    return XML_ERROR_UNEXPECTED_STATE;

  entity = openEntity->entity;
  textStart = ((const char *)entity->textPtr) + entity->processed;
  textEnd = (const char *)(entity->textPtr + entity->textLen);
  /* Set a safe default value in case 'next' does not get set */
  next = textStart;

#ifdef XML_DTD
    int tok
        = XmlPrologTok(parser->m_internalEncoding, textStart, textEnd, &next);
    result = doProlog(parser, parser->m_internalEncoding, textStart, textEnd,
                      tok, next, &next, XML_FALSE, XML_TRUE,
                      XML_ACCOUNT_ENTITY_EXPANSION);
  } else
#endif /* XML_DTD */
    result = doContent(parser, openEntity->startTagLevel,
                       parser->m_internalEncoding, textStart, textEnd, &next,
                       XML_FALSE, XML_ACCOUNT_ENTITY_EXPANSION);

  if (result != XML_ERROR_NONE)
    return result;
  else if (textEnd != next
           && parser->m_parsingStatus.parsing == XML_SUSPENDED) {
    entity->processed = (int)(next - (const char *)entity->textPtr);
    return result;
  } else {
#ifdef XML_DTD
    entityTrackingOnClose(parser, entity, __LINE__);
#endif
    entity->open = XML_FALSE;
    parser->m_openInternalEntities = openEntity->next;
    /* put openEntity back in list of free instances */
    openEntity->next = parser->m_freeInternalEntities;
    parser->m_processor = prologProcessor;
    tok = XmlPrologTok(parser->m_encoding, s, end, &next);
    return doProlog(parser, parser->m_encoding, s, end, tok, next, nextPtr,
                    (XML_Bool)! parser->m_parsingStatus.finalBuffer, XML_TRUE,
                    XML_ACCOUNT_DIRECT);
  } else
#endif /* XML_DTD */
  {
    parser->m_processor = contentProcessor;
    /* see externalEntityContentProcessor vs contentProcessor */
    return doContent(parser, parser->m_parentParser ? 1 : 0, parser->m_encoding,
                     s, end, nextPtr,
                     (XML_Bool)! parser->m_parsingStatus.finalBuffer,
                     XML_ACCOUNT_DIRECT);
  }
}

static enum XML_Error PTRCALL

static enum XML_Error
storeAttributeValue(XML_Parser parser, const ENCODING *enc, XML_Bool isCdata,
                    const char *ptr, const char *end, STRING_POOL *pool,
                    enum XML_Account account) {
  enum XML_Error result
      = appendAttributeValue(parser, enc, isCdata, ptr, end, pool, account);
  if (result)
    return result;
  if (! isCdata && poolLength(pool) && poolLastChar(pool) == 0x20)
    poolChop(pool);

static enum XML_Error
appendAttributeValue(XML_Parser parser, const ENCODING *enc, XML_Bool isCdata,
                     const char *ptr, const char *end, STRING_POOL *pool,
                     enum XML_Account account) {
  DTD *const dtd = parser->m_dtd; /* save one level of indirection */
#ifndef XML_DTD
  UNUSED_P(account);
#endif

  for (;;) {
    const char *next
        = ptr; /* XmlAttributeValueTok doesn't always set the last arg */
    int tok = XmlAttributeValueTok(enc, ptr, end, &next);
#ifdef XML_DTD
    if (! accountingDiffTolerated(parser, tok, ptr, next, __LINE__, account)) {
      accountingOnAbort(parser);
      return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
    }
#endif
    switch (tok) {
    case XML_TOK_NONE:
      return XML_ERROR_NONE;
    case XML_TOK_INVALID:
      XML_Char ch = (XML_Char)XmlPredefinedEntityName(
          enc, ptr + enc->minBytesPerChar, next - enc->minBytesPerChar);
      if (ch) {
#ifdef XML_DTD
        /* NOTE: We are replacing 4-6 characters original input for 1 character
         *       so there is no amplification and hence recording without
         *       protection. */
        accountingDiffTolerated(parser, tok, (char *)&ch,
                                ((char *)&ch) + sizeof(XML_Char), __LINE__,
                                XML_ACCOUNT_ENTITY_EXPANSION);
#endif /* XML_DTD */
        if (! poolAppendChar(pool, ch))
          return XML_ERROR_NO_MEMORY;
        break;
      }
        enum XML_Error result;
        const XML_Char *textEnd = entity->textPtr + entity->textLen;
        entity->open = XML_TRUE;
#ifdef XML_DTD
        entityTrackingOnOpen(parser, entity, __LINE__);
#endif
        result = appendAttributeValue(parser, parser->m_internalEncoding,
                                      isCdata, (const char *)entity->textPtr,
                                      (const char *)textEnd, pool,
                                      XML_ACCOUNT_ENTITY_EXPANSION);
#ifdef XML_DTD
        entityTrackingOnClose(parser, entity, __LINE__);
#endif
        entity->open = XML_FALSE;
        if (result)
          return result;
      }

static enum XML_Error
storeEntityValue(XML_Parser parser, const ENCODING *enc,
                 const char *entityTextPtr, const char *entityTextEnd,
                 enum XML_Account account) {
  DTD *const dtd = parser->m_dtd; /* save one level of indirection */
  STRING_POOL *pool = &(dtd->entityValuePool);
  enum XML_Error result = XML_ERROR_NONE;
#ifdef XML_DTD
  int oldInEntityValue = parser->m_prologState.inEntityValue;
  parser->m_prologState.inEntityValue = 1;
#else
  UNUSED_P(account);
#endif /* XML_DTD */
  /* never return Null for the value argument in EntityDeclHandler,
     since this would indicate an external entity; therefore we
     have to make sure that entityValuePool.start is not null */
  }

  for (;;) {
    const char *next
        = entityTextPtr; /* XmlEntityValueTok doesn't always set the last arg */
    int tok = XmlEntityValueTok(enc, entityTextPtr, entityTextEnd, &next);

#ifdef XML_DTD
    if (! accountingDiffTolerated(parser, tok, entityTextPtr, next, __LINE__,
                                  account)) {
      accountingOnAbort(parser);
      result = XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
      goto endEntityValue;
    }
#endif

    switch (tok) {
    case XML_TOK_PARAM_ENTITY_REF:
#ifdef XML_DTD
      if (parser->m_isParamEntity || enc != parser->m_encoding) {
          if (parser->m_externalEntityRefHandler) {
            dtd->paramEntityRead = XML_FALSE;
            entity->open = XML_TRUE;
            entityTrackingOnOpen(parser, entity, __LINE__);
            if (! parser->m_externalEntityRefHandler(
                    parser->m_externalEntityRefHandlerArg, 0, entity->base,
                    entity->systemId, entity->publicId)) {
              entityTrackingOnClose(parser, entity, __LINE__);
              entity->open = XML_FALSE;
              result = XML_ERROR_EXTERNAL_ENTITY_HANDLING;
              goto endEntityValue;
            }
            entityTrackingOnClose(parser, entity, __LINE__);
            entity->open = XML_FALSE;
            if (! dtd->paramEntityRead)
              dtd->keepProcessing = dtd->standalone;
          } else
            dtd->keepProcessing = dtd->standalone;
        } else {
          entity->open = XML_TRUE;
          entityTrackingOnOpen(parser, entity, __LINE__);
          result = storeEntityValue(
              parser, parser->m_internalEncoding, (const char *)entity->textPtr,
              (const char *)(entity->textPtr + entity->textLen),
              XML_ACCOUNT_ENTITY_EXPANSION);
          entityTrackingOnClose(parser, entity, __LINE__);
          entity->open = XML_FALSE;
          if (result)
            goto endEntityValue;
        }
static void FASTCALL
hashTableIterInit(HASH_TABLE_ITER *iter, const HASH_TABLE *table) {
  iter->p = table->v;
  iter->end = iter->p ? iter->p + table->size : NULL;
}

static NAMED *FASTCALL
hashTableIterNext(HASH_TABLE_ITER *iter) {
  memcpy(result, s, charsRequired * sizeof(XML_Char));
  return result;
}

#ifdef XML_DTD

static float
accountingGetCurrentAmplification(XML_Parser rootParser) {
  const XmlBigCount countBytesOutput
      = rootParser->m_accounting.countBytesDirect
        + rootParser->m_accounting.countBytesIndirect;
  const float amplificationFactor
      = rootParser->m_accounting.countBytesDirect
            ? (countBytesOutput
               / (float)(rootParser->m_accounting.countBytesDirect))
            : 1.0f;
  assert(! rootParser->m_parentParser);
  return amplificationFactor;
}

static void
accountingReportStats(XML_Parser originParser, const char *epilog) {
  const XML_Parser rootParser = getRootParserOf(originParser, NULL);
  assert(! rootParser->m_parentParser);

  if (rootParser->m_accounting.debugLevel < 1) {
    return;
  }

  const float amplificationFactor
      = accountingGetCurrentAmplification(rootParser);
  fprintf(stderr,
          "expat: Accounting(%p): Direct " EXPAT_FMT_ULL(
              "10") ", indirect " EXPAT_FMT_ULL("10") ", amplification %8.2f%s",
          (void *)rootParser, rootParser->m_accounting.countBytesDirect,
          rootParser->m_accounting.countBytesIndirect,
          (double)amplificationFactor, epilog);
}

static void
accountingOnAbort(XML_Parser originParser) {
  accountingReportStats(originParser, " ABORTING\n");
}

static void
accountingReportDiff(XML_Parser rootParser,
                     unsigned int levelsAwayFromRootParser, const char *before,
                     const char *after, ptrdiff_t bytesMore, int source_line,
                     enum XML_Account account) {
  assert(! rootParser->m_parentParser);

  fprintf(stderr,
          " (+" EXPAT_FMT_PTRDIFF_T("6") " bytes %s|%d, xmlparse.c:%d) %*s\"",
          bytesMore, (account == XML_ACCOUNT_DIRECT) ? "DIR" : "EXP",
          levelsAwayFromRootParser, source_line, 10, "");

  const char ellipis[] = "[..]";
  const size_t ellipsisLength = sizeof(ellipis) /* because compile-time */ - 1;
  const unsigned int contextLength = 10;

  /* Note: Performance is of no concern here */
  const char *walker = before;
  if ((rootParser->m_accounting.debugLevel >= 3)
      || (after - before)
             <= (ptrdiff_t)(contextLength + ellipsisLength + contextLength)) {
    for (; walker < after; walker++) {
      fprintf(stderr, "%s", unsignedCharToPrintable(walker[0]));
    }
  } else {
    for (; walker < before + contextLength; walker++) {
      fprintf(stderr, "%s", unsignedCharToPrintable(walker[0]));
    }
    fprintf(stderr, ellipis);
    walker = after - contextLength;
    for (; walker < after; walker++) {
      fprintf(stderr, "%s", unsignedCharToPrintable(walker[0]));
    }
  }
  fprintf(stderr, "\"\n");
}

static XML_Bool
accountingDiffTolerated(XML_Parser originParser, int tok, const char *before,
                        const char *after, int source_line,
                        enum XML_Account account) {
  /* Note: We need to check the token type *first* to be sure that
   *       we can even access variable <after>, safely.
   *       E.g. for XML_TOK_NONE <after> may hold an invalid pointer. */
  switch (tok) {
  case XML_TOK_INVALID:
  case XML_TOK_PARTIAL:
  case XML_TOK_PARTIAL_CHAR:
  case XML_TOK_NONE:
    return XML_TRUE;
  }

  if (account == XML_ACCOUNT_NONE)
    return XML_TRUE; /* because these bytes have been accounted for, already */

  unsigned int levelsAwayFromRootParser;
  const XML_Parser rootParser
      = getRootParserOf(originParser, &levelsAwayFromRootParser);
  assert(! rootParser->m_parentParser);

  const int isDirect
      = (account == XML_ACCOUNT_DIRECT) && (originParser == rootParser);
  const ptrdiff_t bytesMore = after - before;

  XmlBigCount *const additionTarget
      = isDirect ? &rootParser->m_accounting.countBytesDirect
                 : &rootParser->m_accounting.countBytesIndirect;

  /* Detect and avoid integer overflow */
  if (*additionTarget > (XmlBigCount)(-1) - (XmlBigCount)bytesMore)
    return XML_FALSE;
  *additionTarget += bytesMore;

  const XmlBigCount countBytesOutput
      = rootParser->m_accounting.countBytesDirect
        + rootParser->m_accounting.countBytesIndirect;
  const float amplificationFactor
      = accountingGetCurrentAmplification(rootParser);
  const XML_Bool tolerated
      = (countBytesOutput < rootParser->m_accounting.activationThresholdBytes)
        || (amplificationFactor
            <= rootParser->m_accounting.maximumAmplificationFactor);

  if (rootParser->m_accounting.debugLevel >= 2) {
    accountingReportStats(rootParser, "");
    accountingReportDiff(rootParser, levelsAwayFromRootParser, before, after,
                         bytesMore, source_line, account);
  }

  return tolerated;
}

unsigned long long
testingAccountingGetCountBytesDirect(XML_Parser parser) {
  if (! parser)
    return 0;
  return parser->m_accounting.countBytesDirect;
}

unsigned long long
testingAccountingGetCountBytesIndirect(XML_Parser parser) {
  if (! parser)
    return 0;
  return parser->m_accounting.countBytesIndirect;
}

static void
entityTrackingReportStats(XML_Parser rootParser, ENTITY *entity,
                          const char *action, int sourceLine) {
  assert(! rootParser->m_parentParser);
  if (rootParser->m_entity_stats.debugLevel < 1)
    return;

#  if defined(XML_UNICODE)
  const char *const entityName = "[..]";
#  else
  const char *const entityName = entity->name;
#  endif

  fprintf(
      stderr,
      "expat: Entities(%p): Count %9d, depth %2d/%2d %*s%s%s; %s length %d (xmlparse.c:%d)\n",
      (void *)rootParser, rootParser->m_entity_stats.countEverOpened,
      rootParser->m_entity_stats.currentDepth,
      rootParser->m_entity_stats.maximumDepthSeen,
      (rootParser->m_entity_stats.currentDepth - 1) * 2, "",
      entity->is_param ? "%" : "&", entityName, action, entity->textLen,
      sourceLine);
}

static void
entityTrackingOnOpen(XML_Parser originParser, ENTITY *entity, int sourceLine) {
  const XML_Parser rootParser = getRootParserOf(originParser, NULL);
  assert(! rootParser->m_parentParser);

  rootParser->m_entity_stats.countEverOpened++;
  rootParser->m_entity_stats.currentDepth++;
  if (rootParser->m_entity_stats.currentDepth
      > rootParser->m_entity_stats.maximumDepthSeen) {
    rootParser->m_entity_stats.maximumDepthSeen++;
  }

  entityTrackingReportStats(rootParser, entity, "OPEN ", sourceLine);
}

static void
entityTrackingOnClose(XML_Parser originParser, ENTITY *entity, int sourceLine) {
  const XML_Parser rootParser = getRootParserOf(originParser, NULL);
  assert(! rootParser->m_parentParser);

  entityTrackingReportStats(rootParser, entity, "CLOSE", sourceLine);
  rootParser->m_entity_stats.currentDepth--;
}

static XML_Parser
getRootParserOf(XML_Parser parser, unsigned int *outLevelDiff) {
  XML_Parser rootParser = parser;
  unsigned int stepsTakenUpwards = 0;
  while (rootParser->m_parentParser) {
    rootParser = rootParser->m_parentParser;
    stepsTakenUpwards++;
  }
  assert(! rootParser->m_parentParser);
  if (outLevelDiff != NULL) {
    *outLevelDiff = stepsTakenUpwards;
  }
  return rootParser;
}

const char *
unsignedCharToPrintable(unsigned char c) {
  switch (c) {
  case 0:
    return "\\0";
  case 1:
    return "\\x1";
  case 2:
    return "\\x2";
  case 3:
    return "\\x3";
  case 4:
    return "\\x4";
  case 5:
    return "\\x5";
  case 6:
    return "\\x6";
  case 7:
    return "\\x7";
  case 8:
    return "\\x8";
  case 9:
    return "\\t";
  case 10:
    return "\\n";
  case 11:
    return "\\xB";
  case 12:
    return "\\xC";
  case 13:
    return "\\r";
  case 14:
    return "\\xE";
  case 15:
    return "\\xF";
  case 16:
    return "\\x10";
  case 17:
    return "\\x11";
  case 18:
    return "\\x12";
  case 19:
    return "\\x13";
  case 20:
    return "\\x14";
  case 21:
    return "\\x15";
  case 22:
    return "\\x16";
  case 23:
    return "\\x17";
  case 24:
    return "\\x18";
  case 25:
    return "\\x19";
  case 26:
    return "\\x1A";
  case 27:
    return "\\x1B";
  case 28:
    return "\\x1C";
  case 29:
    return "\\x1D";
  case 30:
    return "\\x1E";
  case 31:
    return "\\x1F";
  case 32:
    return " ";
  case 33:
    return "!";
  case 34:
    return "\\\"";
  case 35:
    return "#";
  case 36:
    return "$";
  case 37:
    return "%";
  case 38:
    return "&";
  case 39:
    return "'";
  case 40:
    return "(";
  case 41:
    return ")";
  case 42:
    return "*";
  case 43:
    return "+";
  case 44:
    return ",";
  case 45:
    return "-";
  case 46:
    return ".";
  case 47:
    return "/";
  case 48:
    return "0";
  case 49:
    return "1";
  case 50:
    return "2";
  case 51:
    return "3";
  case 52:
    return "4";
  case 53:
    return "5";
  case 54:
    return "6";
  case 55:
    return "7";
  case 56:
    return "8";
  case 57:
    return "9";
  case 58:
    return ":";
  case 59:
    return ";";
  case 60:
    return "<";
  case 61:
    return "=";
  case 62:
    return ">";
  case 63:
    return "?";
  case 64:
    return "@";
  case 65:
    return "A";
  case 66:
    return "B";
  case 67:
    return "C";
  case 68:
    return "D";
  case 69:
    return "E";
  case 70:
    return "F";
  case 71:
    return "G";
  case 72:
    return "H";
  case 73:
    return "I";
  case 74:
    return "J";
  case 75:
    return "K";
  case 76:
    return "L";
  case 77:
    return "M";
  case 78:
    return "N";
  case 79:
    return "O";
  case 80:
    return "P";
  case 81:
    return "Q";
  case 82:
    return "R";
  case 83:
    return "S";
  case 84:
    return "T";
  case 85:
    return "U";
  case 86:
    return "V";
  case 87:
    return "W";
  case 88:
    return "X";
  case 89:
    return "Y";
  case 90:
    return "Z";
  case 91:
    return "[";
  case 92:
    return "\\\\";
  case 93:
    return "]";
  case 94:
    return "^";
  case 95:
    return "_";
  case 96:
    return "`";
  case 97:
    return "a";
  case 98:
    return "b";
  case 99:
    return "c";
  case 100:
    return "d";
  case 101:
    return "e";
  case 102:
    return "f";
  case 103:
    return "g";
  case 104:
    return "h";
  case 105:
    return "i";
  case 106:
    return "j";
  case 107:
    return "k";
  case 108:
    return "l";
  case 109:
    return "m";
  case 110:
    return "n";
  case 111:
    return "o";
  case 112:
    return "p";
  case 113:
    return "q";
  case 114:
    return "r";
  case 115:
    return "s";
  case 116:
    return "t";
  case 117:
    return "u";
  case 118:
    return "v";
  case 119:
    return "w";
  case 120:
    return "x";
  case 121:
    return "y";
  case 122:
    return "z";
  case 123:
    return "{";
  case 124:
    return "|";
  case 125:
    return "}";
  case 126:
    return "~";
  case 127:
    return "\\x7F";
  case 128:
    return "\\x80";
  case 129:
    return "\\x81";
  case 130:
    return "\\x82";
  case 131:
    return "\\x83";
  case 132:
    return "\\x84";
  case 133:
    return "\\x85";
  case 134:
    return "\\x86";
  case 135:
    return "\\x87";
  case 136:
    return "\\x88";
  case 137:
    return "\\x89";
  case 138:
    return "\\x8A";
  case 139:
    return "\\x8B";
  case 140:
    return "\\x8C";
  case 141:
    return "\\x8D";
  case 142:
    return "\\x8E";
  case 143:
    return "\\x8F";
  case 144:
    return "\\x90";
  case 145:
    return "\\x91";
  case 146:
    return "\\x92";
  case 147:
    return "\\x93";
  case 148:
    return "\\x94";
  case 149:
    return "\\x95";
  case 150:
    return "\\x96";
  case 151:
    return "\\x97";
  case 152:
    return "\\x98";
  case 153:
    return "\\x99";
  case 154:
    return "\\x9A";
  case 155:
    return "\\x9B";
  case 156:
    return "\\x9C";
  case 157:
    return "\\x9D";
  case 158:
    return "\\x9E";
  case 159:
    return "\\x9F";
  case 160:
    return "\\xA0";
  case 161:
    return "\\xA1";
  case 162:
    return "\\xA2";
  case 163:
    return "\\xA3";
  case 164:
    return "\\xA4";
  case 165:
    return "\\xA5";
  case 166:
    return "\\xA6";
  case 167:
    return "\\xA7";
  case 168:
    return "\\xA8";
  case 169:
    return "\\xA9";
  case 170:
    return "\\xAA";
  case 171:
    return "\\xAB";
  case 172:
    return "\\xAC";
  case 173:
    return "\\xAD";
  case 174:
    return "\\xAE";
  case 175:
    return "\\xAF";
  case 176:
    return "\\xB0";
  case 177:
    return "\\xB1";
  case 178:
    return "\\xB2";
  case 179:
    return "\\xB3";
  case 180:
    return "\\xB4";
  case 181:
    return "\\xB5";
  case 182:
    return "\\xB6";
  case 183:
    return "\\xB7";
  case 184:
    return "\\xB8";
  case 185:
    return "\\xB9";
  case 186:
    return "\\xBA";
  case 187:
    return "\\xBB";
  case 188:
    return "\\xBC";
  case 189:
    return "\\xBD";
  case 190:
    return "\\xBE";
  case 191:
    return "\\xBF";
  case 192:
    return "\\xC0";
  case 193:
    return "\\xC1";
  case 194:
    return "\\xC2";
  case 195:
    return "\\xC3";
  case 196:
    return "\\xC4";
  case 197:
    return "\\xC5";
  case 198:
    return "\\xC6";
  case 199:
    return "\\xC7";
  case 200:
    return "\\xC8";
  case 201:
    return "\\xC9";
  case 202:
    return "\\xCA";
  case 203:
    return "\\xCB";
  case 204:
    return "\\xCC";
  case 205:
    return "\\xCD";
  case 206:
    return "\\xCE";
  case 207:
    return "\\xCF";
  case 208:
    return "\\xD0";
  case 209:
    return "\\xD1";
  case 210:
    return "\\xD2";
  case 211:
    return "\\xD3";
  case 212:
    return "\\xD4";
  case 213:
    return "\\xD5";
  case 214:
    return "\\xD6";
  case 215:
    return "\\xD7";
  case 216:
    return "\\xD8";
  case 217:
    return "\\xD9";
  case 218:
    return "\\xDA";
  case 219:
    return "\\xDB";
  case 220:
    return "\\xDC";
  case 221:
    return "\\xDD";
  case 222:
    return "\\xDE";
  case 223:
    return "\\xDF";
  case 224:
    return "\\xE0";
  case 225:
    return "\\xE1";
  case 226:
    return "\\xE2";
  case 227:
    return "\\xE3";
  case 228:
    return "\\xE4";
  case 229:
    return "\\xE5";
  case 230:
    return "\\xE6";
  case 231:
    return "\\xE7";
  case 232:
    return "\\xE8";
  case 233:
    return "\\xE9";
  case 234:
    return "\\xEA";
  case 235:
    return "\\xEB";
  case 236:
    return "\\xEC";
  case 237:
    return "\\xED";
  case 238:
    return "\\xEE";
  case 239:
    return "\\xEF";
  case 240:
    return "\\xF0";
  case 241:
    return "\\xF1";
  case 242:
    return "\\xF2";
  case 243:
    return "\\xF3";
  case 244:
    return "\\xF4";
  case 245:
    return "\\xF5";
  case 246:
    return "\\xF6";
  case 247:
    return "\\xF7";
  case 248:
    return "\\xF8";
  case 249:
    return "\\xF9";
  case 250:
    return "\\xFA";
  case 251:
    return "\\xFB";
  case 252:
    return "\\xFC";
  case 253:
    return "\\xFD";
  case 254:
    return "\\xFE";
  case 255:
    return "\\xFF";
  default:
    assert(0); /* never gets here */
    return "dead code";
  }
  assert(0); /* never gets here */
}

#endif /* XML_DTD */

static unsigned long
getDebugLevel(const char *variableName, unsigned long defaultDebugLevel) {
  const char *const valueOrNull = getenv(variableName);
  if (valueOrNull == NULL) {
    return defaultDebugLevel;
  }
  const char *const value = valueOrNull;

  errno = 0;
  char *afterValue = (char *)value;
  unsigned long debugLevel = strtoul(value, &afterValue, 10);
  if ((errno != 0) || (afterValue[0] != '\0')) {
    errno = 0;
    return defaultDebugLevel;
  }

  return debugLevel;
}