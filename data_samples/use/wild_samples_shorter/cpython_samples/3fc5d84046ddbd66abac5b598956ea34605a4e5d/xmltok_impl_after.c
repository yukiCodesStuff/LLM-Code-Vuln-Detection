/* This file is included (from xmltok.c, 1-3 times depending on XML_MIN_SIZE)!
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000      Clark Cooper <coopercc@users.sourceforge.net>
   Copyright (c) 2002      Fred L. Drake, Jr. <fdrake@users.sourceforge.net>
   Copyright (c) 2002-2016 Karl Waclawek <karl@waclawek.net>
   Copyright (c) 2016-2021 Sebastian Pipping <sebastian@pipping.org>
   Copyright (c) 2017      Rhodri James <rhodri@wildebeest.org.uk>
   Copyright (c) 2018      Benjamin Peterson <benjamin@python.org>
   Copyright (c) 2018      Anton Maklakov <antmak.pub@gmail.com>
   Copyright (c) 2019      David Loffredo <loffredo@steptools.com>
   Copyright (c) 2020      Boris Kolpackov <boris@codesynthesis.com>
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the

#ifdef XML_TOK_IMPL_C

#  ifndef IS_INVALID_CHAR // i.e. for UTF-16 and XML_MIN_SIZE not defined
#    define IS_INVALID_CHAR(enc, ptr, n) (0)
#  endif

#  define INVALID_LEAD_CASE(n, ptr, nextTokPtr)                                \
#  define LEAD_CASE(n)                                                         \
  case BT_LEAD##n:                                                             \
    ptr += n;                                                                  \
    pos->columnNumber++;                                                       \
    break;
      LEAD_CASE(2)
      LEAD_CASE(3)
      LEAD_CASE(4)
#  undef LEAD_CASE
    case BT_LF:
      pos->columnNumber = 0;
      pos->lineNumber++;
      ptr += MINBPC(enc);
      break;
    case BT_CR:
      ptr += MINBPC(enc);
      if (HAS_CHAR(enc, ptr, end) && BYTE_TYPE(enc, ptr) == BT_LF)
        ptr += MINBPC(enc);
      pos->columnNumber = 0;
      break;
    default:
      ptr += MINBPC(enc);
      pos->columnNumber++;
      break;
    }
  }
}

#  undef DO_LEAD_CASE