                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000-2017 Expat development team
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the

#ifdef _WIN32
#  include "winconfig.h"
#else
#  ifdef HAVE_EXPAT_CONFIG_H
#    include <expat_config.h>
#  endif
#endif /* ndef _WIN32 */

#include "expat_external.h"
#include "internal.h"
#include "xmlrole.h"
#ifdef XML_DTD
  if (! state->documentEntity && tok == XML_TOK_PARAM_ENTITY_REF)
    return XML_ROLE_INNER_PARAM_ENTITY_REF;
#endif
  state->handler = error;
  return XML_ROLE_ERROR;
}