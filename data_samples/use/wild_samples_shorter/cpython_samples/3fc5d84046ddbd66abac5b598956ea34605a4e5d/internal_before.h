                        \___/_/\_\ .__/ \__,_|\__|
                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000-2017 Expat development team
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
#  endif
#endif

#ifndef UNUSED_P
#  define UNUSED_P(p) (void)p
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XML_ENABLE_VISIBILITY
#  if XML_ENABLE_VISIBILITY
__attribute__((visibility("default")))
#  endif
#endif
void
_INTERNAL_trim_to_complete_utf8_characters(const char *from,
                                           const char **fromLimRef);

#ifdef __cplusplus
}
#endif