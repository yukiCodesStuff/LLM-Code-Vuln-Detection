/* 90815a2b2c80c03b2b889fe1d427bb2b9e3282aa065e42784e001db4f23de324 (2.4.9+)
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
   Copyright (c) 2021      Dong-hee Na <donghee.na@python.org>
   Copyright (c) 2022      Samanta Navarro <ferivoz@riseup.net>
   Copyright (c) 2022      Jeffrey Walton <noloader@gmail.com>
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
  parserInit(parser, encodingName);

  if (encodingName && ! parser->m_protocolEncodingName) {
    XML_ParserFree(parser);
    return NULL;
  }

        int len;
        const char *rawName;
        TAG *tag = parser->m_tagStack;
        parser->m_tagStack = tag->parent;
        tag->parent = parser->m_freeTagList;
        parser->m_freeTagList = tag;
        rawName = s + enc->minBytesPerChar * 2;
        len = XmlNameLength(enc, rawName);
        if (len != tag->rawNameLength
            || memcmp(tag->rawName, rawName, len) != 0) {
          *eventPP = rawName;
          return XML_ERROR_TAG_MISMATCH;
        }
        --parser->m_tagLevel;
        if (parser->m_endElementHandler) {
          const XML_Char *localPart;
          const XML_Char *prefix;
              parser->m_handlerArg, parser->m_declElementType->name,
              parser->m_declAttributeId->name, parser->m_declAttributeType, 0,
              role == XML_ROLE_REQUIRED_ATTRIBUTE_VALUE);
          poolClear(&parser->m_tempPool);
          handleDefault = XML_FALSE;
        }
      }
      break;
    case XML_ROLE_DEFAULT_ATTRIBUTE_VALUE:
    case XML_ROLE_FIXED_ATTRIBUTE_VALUE:
      if (dtd->keepProcessing) {
             *
             * If 'standalone' is false, the DTD must have no
             * parameter entities or we wouldn't have passed the outer
             * 'if' statement.  That measn the only entity in the hash
             * table is the external subset name "#" which cannot be
             * given as a parameter entity name in XML syntax, so the
             * lookup must have returned NULL and we don't even reach
             * the test for an internal entity.

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
    parser->m_freeInternalEntities = openEntity;
  }

#ifdef XML_DTD
  if (entity->is_param) {