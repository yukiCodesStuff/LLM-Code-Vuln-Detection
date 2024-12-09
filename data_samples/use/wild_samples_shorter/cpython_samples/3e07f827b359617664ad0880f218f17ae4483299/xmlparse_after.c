/* 5ab094ffadd6edfc94c3eee53af44a86951f9f1f0933ada3114bbce2bfb02c99 (2.5.0+)
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
   Copyright (c) 2021      Dong-hee Na <donghee.na@python.org>
   Copyright (c) 2022      Samanta Navarro <ferivoz@riseup.net>
   Copyright (c) 2022      Jeffrey Walton <noloader@gmail.com>
   Copyright (c) 2022      Jann Horn <jannh@google.com>
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
  parserInit(parser, encodingName);

  if (encodingName && ! parser->m_protocolEncodingName) {
    if (dtd) {
      // We need to stop the upcoming call to XML_ParserFree from happily
      // destroying parser->m_dtd because the DTD is shared with the parent
      // parser and the only guard that keeps XML_ParserFree from destroying
      // parser->m_dtd is parser->m_isParamEntity but it will be set to
      // XML_TRUE only later in XML_ExternalEntityParserCreate (or not at all).
      parser->m_dtd = NULL;
    }
    XML_ParserFree(parser);
    return NULL;
  }

        int len;
        const char *rawName;
        TAG *tag = parser->m_tagStack;
        rawName = s + enc->minBytesPerChar * 2;
        len = XmlNameLength(enc, rawName);
        if (len != tag->rawNameLength
            || memcmp(tag->rawName, rawName, len) != 0) {
          *eventPP = rawName;
          return XML_ERROR_TAG_MISMATCH;
        }
        parser->m_tagStack = tag->parent;
        tag->parent = parser->m_freeTagList;
        parser->m_freeTagList = tag;
        --parser->m_tagLevel;
        if (parser->m_endElementHandler) {
          const XML_Char *localPart;
          const XML_Char *prefix;
              parser->m_handlerArg, parser->m_declElementType->name,
              parser->m_declAttributeId->name, parser->m_declAttributeType, 0,
              role == XML_ROLE_REQUIRED_ATTRIBUTE_VALUE);
          handleDefault = XML_FALSE;
        }
      }
      poolClear(&parser->m_tempPool);
      break;
    case XML_ROLE_DEFAULT_ATTRIBUTE_VALUE:
    case XML_ROLE_FIXED_ATTRIBUTE_VALUE:
      if (dtd->keepProcessing) {
             *
             * If 'standalone' is false, the DTD must have no
             * parameter entities or we wouldn't have passed the outer
             * 'if' statement.  That means the only entity in the hash
             * table is the external subset name "#" which cannot be
             * given as a parameter entity name in XML syntax, so the
             * lookup must have returned NULL and we don't even reach
             * the test for an internal entity.

  if (result != XML_ERROR_NONE)
    return result;

  if (textEnd != next && parser->m_parsingStatus.parsing == XML_SUSPENDED) {
    entity->processed = (int)(next - (const char *)entity->textPtr);
    return result;
  }

#ifdef XML_DTD
  entityTrackingOnClose(parser, entity, __LINE__);
#endif
  entity->open = XML_FALSE;
  parser->m_openInternalEntities = openEntity->next;
  /* put openEntity back in list of free instances */
  openEntity->next = parser->m_freeInternalEntities;
  parser->m_freeInternalEntities = openEntity;

  // If there are more open entities we want to stop right here and have the
  // upcoming call to XML_ResumeParser continue with entity content, or it would
  // be ignored altogether.
  if (parser->m_openInternalEntities != NULL
      && parser->m_parsingStatus.parsing == XML_SUSPENDED) {
    return XML_ERROR_NONE;
  }

#ifdef XML_DTD
  if (entity->is_param) {