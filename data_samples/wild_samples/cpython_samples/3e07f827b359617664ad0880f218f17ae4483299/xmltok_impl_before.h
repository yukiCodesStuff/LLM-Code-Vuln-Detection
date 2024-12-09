enum {
  BT_NONXML,   /* e.g. noncharacter-FFFF */
  BT_MALFORM,  /* illegal, with regard to encoding */
  BT_LT,       /* less than = "<" */
  BT_AMP,      /* ampersand = "&" */
  BT_RSQB,     /* right square bracket = "[" */
  BT_LEAD2,    /* lead byte of a 2-byte UTF-8 character */
  BT_LEAD3,    /* lead byte of a 3-byte UTF-8 character */
  BT_LEAD4,    /* lead byte of a 4-byte UTF-8 character */
  BT_TRAIL,    /* trailing unit, e.g. second 16-bit unit of a 4-byte char. */
  BT_CR,       /* carriage return = "\r" */
  BT_LF,       /* line feed = "\n" */
  BT_GT,       /* greater than = ">" */
  BT_QUOT,     /* quotation character = "\"" */
  BT_APOS,     /* aposthrophe = "'" */
  BT_EQUALS,   /* equal sign = "=" */
  BT_QUEST,    /* question mark = "?" */
  BT_EXCL,     /* exclamation mark = "!" */
  BT_SOL,      /* solidus, slash = "/" */
  BT_SEMI,     /* semicolon = ";" */
  BT_NUM,      /* number sign = "#" */
  BT_LSQB,     /* left square bracket = "[" */
  BT_S,        /* white space, e.g. "\t", " "[, "\r"] */
  BT_NMSTRT,   /* non-hex name start letter = "G".."Z" + "g".."z" + "_" */
  BT_COLON,    /* colon = ":" */
  BT_HEX,      /* hex letter = "A".."F" + "a".."f" */
  BT_DIGIT,    /* digit = "0".."9" */
  BT_NAME,     /* dot and middle dot = "." + chr(0xb7) */
  BT_MINUS,    /* minus = "-" */
  BT_OTHER,    /* known not to be a name or name start character */
  BT_NONASCII, /* might be a name or name start character */
  BT_PERCNT,   /* percent sign = "%" */
  BT_LPAR,     /* left parenthesis = "(" */
  BT_RPAR,     /* right parenthesis = "(" */
  BT_AST,      /* asterisk = "*" */
  BT_PLUS,     /* plus sign = "+" */
  BT_COMMA,    /* comma = "," */
  BT_VERBAR    /* vertical bar = "|" */
};

#include <stddef.h>