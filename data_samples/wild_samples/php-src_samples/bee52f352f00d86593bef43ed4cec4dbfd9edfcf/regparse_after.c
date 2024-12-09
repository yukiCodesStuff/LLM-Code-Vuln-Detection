      if (IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_OCTAL3)) {
	PUNFETCH;
	prev = p;
	num = scan_unsigned_octal_number(&p, end, 3, enc);
	if (num < 0 || num >= 256) return ONIGERR_TOO_BIG_NUMBER;
	if (p == prev) {  /* can't read nothing. */
	  num = 0; /* but, it's not error */
	}
	tok->type = TK_RAW_BYTE;
	tok->base = 8;
	tok->u.c  = num;
      }
      break;

    default:
      PUNFETCH;
      num = fetch_escaped_value(&p, end, env);
      if (num < 0) return num;
      if (tok->u.c != num) {
	tok->u.code = (OnigCodePoint )num;
	tok->type   = TK_CODE_POINT;
      }
      break;
    }
  }
  else if (c == '[') {
    if (IS_SYNTAX_OP(syn, ONIG_SYN_OP_POSIX_BRACKET) && (PPEEK_IS(':'))) {
      OnigCodePoint send[] = { (OnigCodePoint )':', (OnigCodePoint )']' };
      tok->backp = p; /* point at '[' is readed */
      PINC;
      if (str_exist_check_with_esc(send, 2, p, end,
                                   (OnigCodePoint )']', enc, syn)) {
	tok->type = TK_POSIX_BRACKET_OPEN;
      }
      else {
	PUNFETCH;
	goto cc_in_cc;
      }
    }
    else {
    cc_in_cc:
      if (IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_CCLASS_SET_OP)) {
	tok->type = TK_CC_CC_OPEN;
      }
      else {
	CC_ESC_WARN(env, (UChar* )"[");
      }
    }
  }
  else if (c == '&') {
    if (IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_CCLASS_SET_OP) &&
	!PEND && (PPEEK_IS('&'))) {
      PINC;
      tok->type = TK_CC_AND;
    }
  }

 end:
  *src = p;
  return tok->type;
}

static int
fetch_token(OnigToken* tok, UChar** src, UChar* end, ScanEnv* env)
{
  int r, num;
  OnigCodePoint c;
  OnigEncoding enc = env->enc;
  OnigSyntaxType* syn = env->syntax;
  UChar* prev;
  UChar* p = *src;
  PFETCH_READY;

 start:
  if (PEND) {
    tok->type = TK_EOT;
    return tok->type;
  }

  tok->type  = TK_STRING;
  tok->base  = 0;
  tok->backp = p;

  PFETCH(c);
  if (IS_MC_ESC_CODE(c, syn)) {
    if (PEND) return ONIGERR_END_PATTERN_AT_ESCAPE;

    tok->backp = p;
    PFETCH(c);

    tok->u.c = c;
    tok->escaped = 1;
    switch (c) {
    case '*':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_ASTERISK_ZERO_INF)) break;
      tok->type = TK_OP_REPEAT;
      tok->u.repeat.lower = 0;
      tok->u.repeat.upper = REPEAT_INFINITE;
      goto greedy_check;
      break;

    case '+':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_PLUS_ONE_INF)) break;
      tok->type = TK_OP_REPEAT;
      tok->u.repeat.lower = 1;
      tok->u.repeat.upper = REPEAT_INFINITE;
      goto greedy_check;
      break;

    case '?':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_QMARK_ZERO_ONE)) break;
      tok->type = TK_OP_REPEAT;
      tok->u.repeat.lower = 0;
      tok->u.repeat.upper = 1;
    greedy_check:
      if (!PEND && PPEEK_IS('?') &&
	  IS_SYNTAX_OP(syn, ONIG_SYN_OP_QMARK_NON_GREEDY)) {
	PFETCH(c);
	tok->u.repeat.greedy     = 0;
	tok->u.repeat.possessive = 0;
      }
      else {
      possessive_check:
	if (!PEND && PPEEK_IS('+') &&
	    ((IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_PLUS_POSSESSIVE_REPEAT) &&
	      tok->type != TK_INTERVAL)  ||
	     (IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_PLUS_POSSESSIVE_INTERVAL) &&
	      tok->type == TK_INTERVAL))) {
	  PFETCH(c);
	  tok->u.repeat.greedy     = 1;
	  tok->u.repeat.possessive = 1;
	}
	else {
	  tok->u.repeat.greedy     = 1;
	  tok->u.repeat.possessive = 0;
	}
      }
      break;

    case '{':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_BRACE_INTERVAL)) break;
      r = fetch_range_quantifier(&p, end, tok, env);
      if (r < 0) return r;  /* error */
      if (r == 0) goto greedy_check;
      else if (r == 2) { /* {n} */
	if (IS_SYNTAX_BV(syn, ONIG_SYN_FIXED_INTERVAL_IS_GREEDY_ONLY))
	  goto possessive_check;

	goto greedy_check;
      }
      /* r == 1 : normal char */
      break;

    case '|':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_VBAR_ALT)) break;
      tok->type = TK_ALT;
      break;

    case '(':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_LPAREN_SUBEXP)) break;
      tok->type = TK_SUBEXP_OPEN;
      break;

    case ')':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_LPAREN_SUBEXP)) break;
      tok->type = TK_SUBEXP_CLOSE;
      break;

    case 'w':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_W_WORD)) break;
      tok->type = TK_CHAR_TYPE;
      tok->u.prop.ctype = ONIGENC_CTYPE_WORD;
      tok->u.prop.not   = 0;
      break;

    case 'W':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_W_WORD)) break;
      tok->type = TK_CHAR_TYPE;
      tok->u.prop.ctype = ONIGENC_CTYPE_WORD;
      tok->u.prop.not   = 1;
      break;

    case 'b':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_B_WORD_BOUND)) break;
      tok->type = TK_ANCHOR;
      tok->u.anchor = ANCHOR_WORD_BOUND;
      break;

    case 'B':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_B_WORD_BOUND)) break;
      tok->type = TK_ANCHOR;
      tok->u.anchor = ANCHOR_NOT_WORD_BOUND;
      break;

#ifdef USE_WORD_BEGIN_END
    case '<':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_LTGT_WORD_BEGIN_END)) break;
      tok->type = TK_ANCHOR;
      tok->u.anchor = ANCHOR_WORD_BEGIN;
      break;

    case '>':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_LTGT_WORD_BEGIN_END)) break;
      tok->type = TK_ANCHOR;
      tok->u.anchor = ANCHOR_WORD_END;
      break;
#endif

    case 's':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_S_WHITE_SPACE)) break;
      tok->type = TK_CHAR_TYPE;
      tok->u.prop.ctype = ONIGENC_CTYPE_SPACE;
      tok->u.prop.not   = 0;
      break;

    case 'S':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_S_WHITE_SPACE)) break;
      tok->type = TK_CHAR_TYPE;
      tok->u.prop.ctype = ONIGENC_CTYPE_SPACE;
      tok->u.prop.not   = 1;
      break;

    case 'd':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_D_DIGIT)) break;
      tok->type = TK_CHAR_TYPE;
      tok->u.prop.ctype = ONIGENC_CTYPE_DIGIT;
      tok->u.prop.not   = 0;
      break;

    case 'D':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_D_DIGIT)) break;
      tok->type = TK_CHAR_TYPE;
      tok->u.prop.ctype = ONIGENC_CTYPE_DIGIT;
      tok->u.prop.not   = 1;
      break;

    case 'h':
      if (! IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_ESC_H_XDIGIT)) break;
      tok->type = TK_CHAR_TYPE;
      tok->u.prop.ctype = ONIGENC_CTYPE_XDIGIT;
      tok->u.prop.not   = 0;
      break;

    case 'H':
      if (! IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_ESC_H_XDIGIT)) break;
      tok->type = TK_CHAR_TYPE;
      tok->u.prop.ctype = ONIGENC_CTYPE_XDIGIT;
      tok->u.prop.not   = 1;
      break;

    case 'A':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_AZ_BUF_ANCHOR)) break;
    begin_buf:
      tok->type = TK_ANCHOR;
      tok->u.subtype = ANCHOR_BEGIN_BUF;
      break;

    case 'Z':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_AZ_BUF_ANCHOR)) break;
      tok->type = TK_ANCHOR;
      tok->u.subtype = ANCHOR_SEMI_END_BUF;
      break;

    case 'z':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_AZ_BUF_ANCHOR)) break;
    end_buf:
      tok->type = TK_ANCHOR;
      tok->u.subtype = ANCHOR_END_BUF;
      break;

    case 'G':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_CAPITAL_G_BEGIN_ANCHOR)) break;
      tok->type = TK_ANCHOR;
      tok->u.subtype = ANCHOR_BEGIN_POSITION;
      break;

    case '`':
      if (! IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_ESC_GNU_BUF_ANCHOR)) break;
      goto begin_buf;
      break;

    case '\'':
      if (! IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_ESC_GNU_BUF_ANCHOR)) break;
      goto end_buf;
      break;

    case 'x':
      if (PEND) break;

      prev = p;
      if (PPEEK_IS('{') && IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_X_BRACE_HEX8)) {
	PINC;
	num = scan_unsigned_hexadecimal_number(&p, end, 8, enc);
	if (num < 0) return ONIGERR_TOO_BIG_WIDE_CHAR_VALUE;
	if (!PEND) {
          if (ONIGENC_IS_CODE_XDIGIT(enc, PPEEK))
            return ONIGERR_TOO_LONG_WIDE_CHAR_VALUE;
        }

	if ((p > prev + enclen(enc, prev)) && !PEND && PPEEK_IS('}')) {
	  PINC;
	  tok->type   = TK_CODE_POINT;
	  tok->u.code = (OnigCodePoint )num;
	}
	else {
	  /* can't read nothing or invalid format */
	  p = prev;
	}
      }
      else if (IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_X_HEX2)) {
	num = scan_unsigned_hexadecimal_number(&p, end, 2, enc);
	if (num < 0) return ONIGERR_TOO_BIG_NUMBER;
	if (p == prev) {  /* can't read nothing. */
	  num = 0; /* but, it's not error */
	}
	tok->type = TK_RAW_BYTE;
	tok->base = 16;
	tok->u.c  = num;
      }
      break;

    case 'u':
      if (PEND) break;

      prev = p;
      if (IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_ESC_U_HEX4)) {
	num = scan_unsigned_hexadecimal_number(&p, end, 4, enc);
	if (num < 0) return ONIGERR_TOO_BIG_NUMBER;
	if (p == prev) {  /* can't read nothing. */
	  num = 0; /* but, it's not error */
	}
	tok->type   = TK_CODE_POINT;
	tok->base   = 16;
	tok->u.code = (OnigCodePoint )num;
      }
      break;

    case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      PUNFETCH;
      prev = p;
      num = onig_scan_unsigned_number(&p, end, enc);
      if (num < 0 || num > ONIG_MAX_BACKREF_NUM) {
        goto skip_backref;
      }

      if (IS_SYNTAX_OP(syn, ONIG_SYN_OP_DECIMAL_BACKREF) && 
	  (num <= env->num_mem || num <= 9)) { /* This spec. from GNU regex */
	if (IS_SYNTAX_BV(syn, ONIG_SYN_STRICT_CHECK_BACKREF)) {
	  if (num > env->num_mem || IS_NULL(SCANENV_MEM_NODES(env)[num]))
	    return ONIGERR_INVALID_BACKREF;
	}

	tok->type = TK_BACKREF;
	tok->u.backref.num     = 1;
	tok->u.backref.ref1    = num;
	tok->u.backref.by_name = 0;
#ifdef USE_BACKREF_WITH_LEVEL
	tok->u.backref.exist_level = 0;
#endif
	break;
      }

    skip_backref:
      if (c == '8' || c == '9') {
	/* normal char */
	p = prev; PINC;
	break;
      }

      p = prev;
      /* fall through */
    case '0':
      if (IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_OCTAL3)) {
	prev = p;
	num = scan_unsigned_octal_number(&p, end, (c == '0' ? 2:3), enc);
	if (num < 0 || num >= 256) return ONIGERR_TOO_BIG_NUMBER;
	if (p == prev) {  /* can't read nothing. */
	  num = 0; /* but, it's not error */
	}
	tok->type = TK_RAW_BYTE;
	tok->base = 8;
	tok->u.c  = num;
      }
      else if (c != '0') {
	PINC;
      }
      break;

#ifdef USE_NAMED_GROUP
    case 'k':
      if (IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_ESC_K_NAMED_BACKREF)) {
	PFETCH(c);
	if (c == '<' || c == '\'') {
	  UChar* name_end;
	  int* backs;
	  int back_num;

	  prev = p;

#ifdef USE_BACKREF_WITH_LEVEL
	  name_end = NULL_UCHARP; /* no need. escape gcc warning. */
	  r = fetch_name_with_level((OnigCodePoint )c, &p, end, &name_end,
				    env, &back_num, &tok->u.backref.level);
	  if (r == 1) tok->u.backref.exist_level = 1;
	  else        tok->u.backref.exist_level = 0;
#else
	  r = fetch_name(&p, end, &name_end, env, &back_num, 1);
#endif
	  if (r < 0) return r;

	  if (back_num != 0) {
	    if (back_num < 0) {
	      back_num = BACKREF_REL_TO_ABS(back_num, env);
	      if (back_num <= 0)
		return ONIGERR_INVALID_BACKREF;
	    }

	    if (IS_SYNTAX_BV(syn, ONIG_SYN_STRICT_CHECK_BACKREF)) {
	      if (back_num > env->num_mem ||
		  IS_NULL(SCANENV_MEM_NODES(env)[back_num]))
		return ONIGERR_INVALID_BACKREF;
	    }
	    tok->type = TK_BACKREF;
	    tok->u.backref.by_name = 0;
	    tok->u.backref.num  = 1;
	    tok->u.backref.ref1 = back_num;
	  }
	  else {
	    num = onig_name_to_group_numbers(env->reg, prev, name_end, &backs);
	    if (num <= 0) {
	      onig_scan_env_set_error_string(env,
			     ONIGERR_UNDEFINED_NAME_REFERENCE, prev, name_end);
	      return ONIGERR_UNDEFINED_NAME_REFERENCE;
	    }
	    if (IS_SYNTAX_BV(syn, ONIG_SYN_STRICT_CHECK_BACKREF)) {
	      int i;
	      for (i = 0; i < num; i++) {
		if (backs[i] > env->num_mem ||
		    IS_NULL(SCANENV_MEM_NODES(env)[backs[i]]))
		  return ONIGERR_INVALID_BACKREF;
	      }
	    }

	    tok->type = TK_BACKREF;
	    tok->u.backref.by_name = 1;
	    if (num == 1) {
	      tok->u.backref.num  = 1;
	      tok->u.backref.ref1 = backs[0];
	    }
	    else {
	      tok->u.backref.num  = num;
	      tok->u.backref.refs = backs;
	    }
	  }
	}
	else
	  PUNFETCH;
      }
      break;
#endif

#ifdef USE_SUBEXP_CALL
    case 'g':
      if (IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_ESC_G_SUBEXP_CALL)) {
	PFETCH(c);
	if (c == '<' || c == '\'') {
	  int gnum;
	  UChar* name_end;

	  prev = p;
	  r = fetch_name((OnigCodePoint )c, &p, end, &name_end, env, &gnum, 1);
	  if (r < 0) return r;

	  tok->type = TK_CALL;
	  tok->u.call.name     = prev;
	  tok->u.call.name_end = name_end;
	  tok->u.call.gnum     = gnum;
	}
	else
	  PUNFETCH;
      }
      break;
#endif

    case 'Q':
      if (IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_ESC_CAPITAL_Q_QUOTE)) {
	tok->type = TK_QUOTE_OPEN;
      }
      break;

    case 'p':
    case 'P':
      if (PPEEK_IS('{') &&
	  IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_ESC_P_BRACE_CHAR_PROPERTY)) {
	PINC;
	tok->type = TK_CHAR_PROPERTY;
	tok->u.prop.not = (c == 'P' ? 1 : 0);

	if (IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_ESC_P_BRACE_CIRCUMFLEX_NOT)) {
	  PFETCH(c);
	  if (c == '^') {
	    tok->u.prop.not = (tok->u.prop.not == 0 ? 1 : 0);
	  }
	  else
	    PUNFETCH;
	}
      }
      break;

    default:
      PUNFETCH;
      num = fetch_escaped_value(&p, end, env);
      if (num < 0) return num;
      /* set_raw: */
      if (tok->u.c != num) {
	tok->type = TK_CODE_POINT;
	tok->u.code = (OnigCodePoint )num;
      }
      else { /* string */
	p = tok->backp + enclen(enc, tok->backp);
      }
      break;
    }
  }
  else {
    tok->u.c = c;
    tok->escaped = 0;

#ifdef USE_VARIABLE_META_CHARS
    if ((c != ONIG_INEFFECTIVE_META_CHAR) &&
	IS_SYNTAX_OP(syn, ONIG_SYN_OP_VARIABLE_META_CHARACTERS)) {
      if (c == MC_ANYCHAR(syn))
	goto any_char;
      else if (c == MC_ANYTIME(syn))
	goto anytime;
      else if (c == MC_ZERO_OR_ONE_TIME(syn))
	goto zero_or_one_time;
      else if (c == MC_ONE_OR_MORE_TIME(syn))
	goto one_or_more_time;
      else if (c == MC_ANYCHAR_ANYTIME(syn)) {
	tok->type = TK_ANYCHAR_ANYTIME;
	goto out;
      }
    }
#endif

    switch (c) {
    case '.':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_DOT_ANYCHAR)) break;
#ifdef USE_VARIABLE_META_CHARS
    any_char:
#endif
      tok->type = TK_ANYCHAR;
      break;

    case '*':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_ASTERISK_ZERO_INF)) break;
#ifdef USE_VARIABLE_META_CHARS
    anytime:
#endif
      tok->type = TK_OP_REPEAT;
      tok->u.repeat.lower = 0;
      tok->u.repeat.upper = REPEAT_INFINITE;
      goto greedy_check;
      break;

    case '+':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_PLUS_ONE_INF)) break;
#ifdef USE_VARIABLE_META_CHARS
    one_or_more_time:
#endif
      tok->type = TK_OP_REPEAT;
      tok->u.repeat.lower = 1;
      tok->u.repeat.upper = REPEAT_INFINITE;
      goto greedy_check;
      break;

    case '?':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_QMARK_ZERO_ONE)) break;
#ifdef USE_VARIABLE_META_CHARS
    zero_or_one_time:
#endif
      tok->type = TK_OP_REPEAT;
      tok->u.repeat.lower = 0;
      tok->u.repeat.upper = 1;
      goto greedy_check;
      break;

    case '{':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_BRACE_INTERVAL)) break;
      r = fetch_range_quantifier(&p, end, tok, env);
      if (r < 0) return r;  /* error */
      if (r == 0) goto greedy_check;
      else if (r == 2) { /* {n} */
	if (IS_SYNTAX_BV(syn, ONIG_SYN_FIXED_INTERVAL_IS_GREEDY_ONLY))
	  goto possessive_check;

	goto greedy_check;
      }
      /* r == 1 : normal char */
      break;

    case '|':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_VBAR_ALT)) break;
      tok->type = TK_ALT;
      break;

    case '(':
      if (PPEEK_IS('?') &&
          IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_QMARK_GROUP_EFFECT)) {
        PINC;
        if (PPEEK_IS('#')) {
          PFETCH(c);
          while (1) {
            if (PEND) return ONIGERR_END_PATTERN_IN_GROUP;
            PFETCH(c);
            if (c == MC_ESC(syn)) {
              if (!PEND) PFETCH(c);
            }
            else {
              if (c == ')') break;
            }
          }
          goto start;
        }
        PUNFETCH;
      }

      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_LPAREN_SUBEXP)) break;
      tok->type = TK_SUBEXP_OPEN;
      break;

    case ')':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_LPAREN_SUBEXP)) break;
      tok->type = TK_SUBEXP_CLOSE;
      break;

    case '^':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_LINE_ANCHOR)) break;
      tok->type = TK_ANCHOR;
      tok->u.subtype = (IS_SINGLELINE(env->option)
			? ANCHOR_BEGIN_BUF : ANCHOR_BEGIN_LINE);
      break;

    case '$':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_LINE_ANCHOR)) break;
      tok->type = TK_ANCHOR;
      tok->u.subtype = (IS_SINGLELINE(env->option)
			? ANCHOR_SEMI_END_BUF : ANCHOR_END_LINE);
      break;

    case '[':
      if (! IS_SYNTAX_OP(syn, ONIG_SYN_OP_BRACKET_CC)) break;
      tok->type = TK_CC_OPEN;
      break;

    case ']':
      if (*src > env->pattern)   /* /].../ is allowed. */
	CLOSE_BRACKET_WITHOUT_ESC_WARN(env, (UChar* )"]");
      break;

    case '#':
      if (IS_EXTEND(env->option)) {
	while (!PEND) {
	  PFETCH(c);
	  if (ONIGENC_IS_CODE_NEWLINE(enc, c))
	    break;
	}
	goto start;
	break;
      }
      break;

    case ' ': case '\t': case '\n': case '\r': case '\f':
      if (IS_EXTEND(env->option))
	goto start;
      break;

    default:
      /* string */
      break;
    }
  }

#ifdef USE_VARIABLE_META_CHARS
 out:
#endif
  *src = p;
  return tok->type;
}

static int
add_ctype_to_cc_by_range(CClassNode* cc, int ctype ARG_UNUSED, int not,
			 OnigEncoding enc ARG_UNUSED,
                         OnigCodePoint sb_out, const OnigCodePoint mbr[])
{
  int i, r;
  OnigCodePoint j;

  int n = ONIGENC_CODE_RANGE_NUM(mbr);

  if (not == 0) {
    for (i = 0; i < n; i++) {
      for (j  = ONIGENC_CODE_RANGE_FROM(mbr, i);
           j <= ONIGENC_CODE_RANGE_TO(mbr, i); j++) {
	if (j >= sb_out) {
	  if (j == ONIGENC_CODE_RANGE_TO(mbr, i)) i++;
	  else if (j > ONIGENC_CODE_RANGE_FROM(mbr, i)) {
	    r = add_code_range_to_buf(&(cc->mbuf), j,
				      ONIGENC_CODE_RANGE_TO(mbr, i));
	    if (r != 0) return r;
	    i++;
	  }

	  goto sb_end;
	}
        BITSET_SET_BIT(cc->bs, j);
      }
    }

  sb_end:
    for ( ; i < n; i++) {
      r = add_code_range_to_buf(&(cc->mbuf),
                                ONIGENC_CODE_RANGE_FROM(mbr, i),
                                ONIGENC_CODE_RANGE_TO(mbr, i));
      if (r != 0) return r;
    }
  }
  else {
    OnigCodePoint prev = 0;

    for (i = 0; i < n; i++) {
      for (j = prev;
	   j < ONIGENC_CODE_RANGE_FROM(mbr, i); j++) {
	if (j >= sb_out) {
	  goto sb_end2;
	}
	BITSET_SET_BIT(cc->bs, j);
      }
      prev = ONIGENC_CODE_RANGE_TO(mbr, i) + 1;
    }
    for (j = prev; j < sb_out; j++) {
      BITSET_SET_BIT(cc->bs, j);
    }

  sb_end2:
    prev = sb_out;

    for (i = 0; i < n; i++) {
      if (prev < ONIGENC_CODE_RANGE_FROM(mbr, i)) {
	r = add_code_range_to_buf(&(cc->mbuf), prev,
                                  ONIGENC_CODE_RANGE_FROM(mbr, i) - 1);
	if (r != 0) return r;
      }
      prev = ONIGENC_CODE_RANGE_TO(mbr, i) + 1;
    }
    if (prev < 0x7fffffff) {
      r = add_code_range_to_buf(&(cc->mbuf), prev, 0x7fffffff);
      if (r != 0) return r;
    }
  }

  return 0;
}

static int
add_ctype_to_cc(CClassNode* cc, int ctype, int not, ScanEnv* env)
{
  int c, r;
  const OnigCodePoint *ranges;
  OnigCodePoint sb_out;
  OnigEncoding enc = env->enc;

  r = ONIGENC_GET_CTYPE_CODE_RANGE(enc, ctype, &sb_out, &ranges);
  if (r == 0) {
    return add_ctype_to_cc_by_range(cc, ctype, not, env->enc, sb_out, ranges);
  }
  else if (r != ONIG_NO_SUPPORT_CONFIG) {
    return r;
  }

  r = 0;
  switch (ctype) {
  case ONIGENC_CTYPE_ALPHA:
  case ONIGENC_CTYPE_BLANK:
  case ONIGENC_CTYPE_CNTRL:
  case ONIGENC_CTYPE_DIGIT:
  case ONIGENC_CTYPE_LOWER:
  case ONIGENC_CTYPE_PUNCT:
  case ONIGENC_CTYPE_SPACE:
  case ONIGENC_CTYPE_UPPER:
  case ONIGENC_CTYPE_XDIGIT:
  case ONIGENC_CTYPE_ASCII:
  case ONIGENC_CTYPE_ALNUM:
    if (not != 0) {
      for (c = 0; c < SINGLE_BYTE_SIZE; c++) {
	if (! ONIGENC_IS_CODE_CTYPE(enc, (OnigCodePoint )c, ctype))
	  BITSET_SET_BIT(cc->bs, c);
      }
      ADD_ALL_MULTI_BYTE_RANGE(enc, cc->mbuf);
    }
    else {
      for (c = 0; c < SINGLE_BYTE_SIZE; c++) {
	if (ONIGENC_IS_CODE_CTYPE(enc, (OnigCodePoint )c, ctype))
	  BITSET_SET_BIT(cc->bs, c);
      }
    }
    break;

  case ONIGENC_CTYPE_GRAPH:
  case ONIGENC_CTYPE_PRINT:
    if (not != 0) {
      for (c = 0; c < SINGLE_BYTE_SIZE; c++) {
	if (! ONIGENC_IS_CODE_CTYPE(enc, (OnigCodePoint )c, ctype))
	  BITSET_SET_BIT(cc->bs, c);
      }
    }
    else {
      for (c = 0; c < SINGLE_BYTE_SIZE; c++) {
	if (ONIGENC_IS_CODE_CTYPE(enc, (OnigCodePoint )c, ctype))
	  BITSET_SET_BIT(cc->bs, c);
      }
      ADD_ALL_MULTI_BYTE_RANGE(enc, cc->mbuf);
    }
    break;

  case ONIGENC_CTYPE_WORD:
    if (not == 0) {
      for (c = 0; c < SINGLE_BYTE_SIZE; c++) {
	if (IS_CODE_SB_WORD(enc, c)) BITSET_SET_BIT(cc->bs, c);
      }
      ADD_ALL_MULTI_BYTE_RANGE(enc, cc->mbuf);
    }
    else {
      for (c = 0; c < SINGLE_BYTE_SIZE; c++) {
        if ((ONIGENC_CODE_TO_MBCLEN(enc, c) > 0) /* check invalid code point */
	    && ! ONIGENC_IS_CODE_WORD(enc, c))
	  BITSET_SET_BIT(cc->bs, c);
      }
    }
    break;

  default:
    return ONIGERR_PARSER_BUG;
    break;
  }

  return r;
}

static int
parse_posix_bracket(CClassNode* cc, UChar** src, UChar* end, ScanEnv* env)
{
#define POSIX_BRACKET_CHECK_LIMIT_LENGTH  20
#define POSIX_BRACKET_NAME_MIN_LEN         4

  static PosixBracketEntryType PBS[] = {
    { (UChar* )"alnum",  ONIGENC_CTYPE_ALNUM,  5 },
    { (UChar* )"alpha",  ONIGENC_CTYPE_ALPHA,  5 },
    { (UChar* )"blank",  ONIGENC_CTYPE_BLANK,  5 },
    { (UChar* )"cntrl",  ONIGENC_CTYPE_CNTRL,  5 },
    { (UChar* )"digit",  ONIGENC_CTYPE_DIGIT,  5 },
    { (UChar* )"graph",  ONIGENC_CTYPE_GRAPH,  5 },
    { (UChar* )"lower",  ONIGENC_CTYPE_LOWER,  5 },
    { (UChar* )"print",  ONIGENC_CTYPE_PRINT,  5 },
    { (UChar* )"punct",  ONIGENC_CTYPE_PUNCT,  5 },
    { (UChar* )"space",  ONIGENC_CTYPE_SPACE,  5 },
    { (UChar* )"upper",  ONIGENC_CTYPE_UPPER,  5 },
    { (UChar* )"xdigit", ONIGENC_CTYPE_XDIGIT, 6 },
    { (UChar* )"ascii",  ONIGENC_CTYPE_ASCII,  5 },
    { (UChar* )"word",   ONIGENC_CTYPE_WORD,   4 },
    { (UChar* )NULL,     -1, 0 }
  };

  PosixBracketEntryType *pb;
  int not, i, r;
  OnigCodePoint c;
  OnigEncoding enc = env->enc;
  UChar *p = *src;

  if (PPEEK_IS('^')) {
    PINC_S;
    not = 1;
  }
  else
    not = 0;

  if (onigenc_strlen(enc, p, end) < POSIX_BRACKET_NAME_MIN_LEN + 3)
    goto not_posix_bracket;

  for (pb = PBS; IS_NOT_NULL(pb->name); pb++) {
    if (onigenc_with_ascii_strncmp(enc, p, end, pb->name, pb->len) == 0) {
      p = (UChar* )onigenc_step(enc, p, end, pb->len);
      if (onigenc_with_ascii_strncmp(enc, p, end, (UChar* )":]", 2) != 0)
        return ONIGERR_INVALID_POSIX_BRACKET_TYPE;

      r = add_ctype_to_cc(cc, pb->ctype, not, env);
      if (r != 0) return r;

      PINC_S; PINC_S;
      *src = p;
      return 0;
    }
  }

 not_posix_bracket:
  c = 0;
  i = 0;
  while (!PEND && ((c = PPEEK) != ':') && c != ']') {
    PINC_S;
    if (++i > POSIX_BRACKET_CHECK_LIMIT_LENGTH) break;
  }
  if (c == ':' && ! PEND) {
    PINC_S;
    if (! PEND) {
      PFETCH_S(c);
      if (c == ']')
        return ONIGERR_INVALID_POSIX_BRACKET_TYPE;
    }
  }

  return 1;  /* 1: is not POSIX bracket, but no error. */
}

static int
fetch_char_property_to_ctype(UChar** src, UChar* end, ScanEnv* env)
{
  int r;
  OnigCodePoint c;
  OnigEncoding enc = env->enc;
  UChar *prev, *start, *p = *src;

  r = 0;
  start = prev = p;

  while (!PEND) {
    prev = p;
    PFETCH_S(c);
    if (c == '}') {
      r = ONIGENC_PROPERTY_NAME_TO_CTYPE(enc, start, prev);
      if (r < 0) break;

      *src = p;
      return r;
    }
    else if (c == '(' || c == ')' || c == '{' || c == '|') {
      r = ONIGERR_INVALID_CHAR_PROPERTY_NAME;
      break;
    }
  }

  onig_scan_env_set_error_string(env, r, *src, prev);
  return r;
}

static int
parse_char_property(Node** np, OnigToken* tok, UChar** src, UChar* end,
		    ScanEnv* env)
{
  int r, ctype;
  CClassNode* cc;

  ctype = fetch_char_property_to_ctype(src, end, env);
  if (ctype < 0) return ctype;

  *np = node_new_cclass();
  CHECK_NULL_RETURN_MEMERR(*np);
  cc = NCCLASS(*np);
  r = add_ctype_to_cc(cc, ctype, 0, env);
  if (r != 0) return r;
  if (tok->u.prop.not != 0) NCCLASS_SET_NOT(cc);

  return 0;
}


enum CCSTATE {
  CCS_VALUE,
  CCS_RANGE,
  CCS_COMPLETE,
  CCS_START
};

enum CCVALTYPE {
  CCV_SB,
  CCV_CODE_POINT,
  CCV_CLASS
};

static int
next_state_class(CClassNode* cc, OnigCodePoint* vs, enum CCVALTYPE* type,
		 enum CCSTATE* state, ScanEnv* env)
{
  int r;

  if (*state == CCS_RANGE)
    return ONIGERR_CHAR_CLASS_VALUE_AT_END_OF_RANGE;

  if (*state == CCS_VALUE && *type != CCV_CLASS) {
    if (*type == CCV_SB)
      BITSET_SET_BIT(cc->bs, (int )(*vs));
    else if (*type == CCV_CODE_POINT) {
      r = add_code_range(&(cc->mbuf), env, *vs, *vs);
      if (r < 0) return r;
    }
  }

  if (*state != CCS_START)
    *state = CCS_VALUE;

  *type  = CCV_CLASS;
  return 0;
}

static int
next_state_val(CClassNode* cc, OnigCodePoint *vs, OnigCodePoint v,
	       int* vs_israw, int v_israw,
	       enum CCVALTYPE intype, enum CCVALTYPE* type,
	       enum CCSTATE* state, ScanEnv* env)
{
  int r;

  switch (*state) {
  case CCS_VALUE:
    if (*type == CCV_SB)
    {
    if (*vs > 0xff)
      return ONIGERR_INVALID_CODE_POINT_VALUE;
      BITSET_SET_BIT(cc->bs, (int )(*vs));
    }
    else if (*type == CCV_CODE_POINT) {
      r = add_code_range(&(cc->mbuf), env, *vs, *vs);
      if (r < 0) return r;
    }
    break;

  case CCS_RANGE:
    if (intype == *type) {
      if (intype == CCV_SB) {
        if (*vs > 0xff || v > 0xff)
          return ONIGERR_INVALID_CODE_POINT_VALUE;

	if (*vs > v) {
	  if (IS_SYNTAX_BV(env->syntax, ONIG_SYN_ALLOW_EMPTY_RANGE_IN_CC))
	    goto ccs_range_end;
	  else
	    return ONIGERR_EMPTY_RANGE_IN_CHAR_CLASS;
	}
	bitset_set_range(cc->bs, (int )*vs, (int )v);
      }
      else {
	r = add_code_range(&(cc->mbuf), env, *vs, v);
	if (r < 0) return r;
      }
    }
    else {
#if 0
      if (intype == CCV_CODE_POINT && *type == CCV_SB) {
#endif
	if (*vs > v) {
	  if (IS_SYNTAX_BV(env->syntax, ONIG_SYN_ALLOW_EMPTY_RANGE_IN_CC))
	    goto ccs_range_end;
	  else
	    return ONIGERR_EMPTY_RANGE_IN_CHAR_CLASS;
	}
	bitset_set_range(cc->bs, (int )*vs, (int )(v < 0xff ? v : 0xff));
	r = add_code_range(&(cc->mbuf), env, (OnigCodePoint )*vs, v);
	if (r < 0) return r;
#if 0
      }
      else
	return ONIGERR_MISMATCH_CODE_LENGTH_IN_CLASS_RANGE;
#endif
    }
  ccs_range_end:
    *state = CCS_COMPLETE;
    break;

  case CCS_COMPLETE:
  case CCS_START:
    *state = CCS_VALUE;
    break;

  default:
    break;
  }

  *vs_israw = v_israw;
  *vs       = v;
  *type     = intype;
  return 0;
}

static int
code_exist_check(OnigCodePoint c, UChar* from, UChar* end, int ignore_escaped,
		 ScanEnv* env)
{
  int in_esc;
  OnigCodePoint code;
  OnigEncoding enc = env->enc;
  UChar* p = from;

  in_esc = 0;
  while (! PEND) {
    if (ignore_escaped && in_esc) {
      in_esc = 0;
    }
    else {
      PFETCH_S(code);
      if (code == c) return 1;
      if (code == MC_ESC(env->syntax)) in_esc = 1;
    }
  }
  return 0;
}

static int
parse_char_class(Node** np, OnigToken* tok, UChar** src, UChar* end,
		 ScanEnv* env)
{
  int r, neg, len, fetched, and_start;
  OnigCodePoint v, vs;
  UChar *p;
  Node* node;
  CClassNode *cc, *prev_cc;
  CClassNode work_cc;

  enum CCSTATE state;
  enum CCVALTYPE val_type, in_type;
  int val_israw, in_israw;

  prev_cc = (CClassNode* )NULL;
  *np = NULL_NODE;
  r = fetch_token_in_cc(tok, src, end, env);
  if (r == TK_CHAR && tok->u.c == '^' && tok->escaped == 0) {
    neg = 1;
    r = fetch_token_in_cc(tok, src, end, env);
  }
  else {
    neg = 0;
  }

  if (r < 0) return r;
  if (r == TK_CC_CLOSE) {
    if (! code_exist_check((OnigCodePoint )']',
                           *src, env->pattern_end, 1, env))
      return ONIGERR_EMPTY_CHAR_CLASS;

    CC_ESC_WARN(env, (UChar* )"]");
    r = tok->type = TK_CHAR;  /* allow []...] */
  }

  *np = node = node_new_cclass();
  CHECK_NULL_RETURN_MEMERR(node);
  cc = NCCLASS(node);

  and_start = 0;
  state = CCS_START;
  p = *src;
  while (r != TK_CC_CLOSE) {
    fetched = 0;
    switch (r) {
    case TK_CHAR:
      len = ONIGENC_CODE_TO_MBCLEN(env->enc, tok->u.c);
      if (len > 1) {
	in_type = CCV_CODE_POINT;
      }
      else if (len < 0) {
	r = len;
	goto err;
      }
      else {
      sb_char:
	in_type = CCV_SB;
      }
      v = (OnigCodePoint )tok->u.c;
      in_israw = 0;
      goto val_entry2;
      break;

    case TK_RAW_BYTE:
      /* tok->base != 0 : octal or hexadec. */
      if (! ONIGENC_IS_SINGLEBYTE(env->enc) && tok->base != 0) {
	UChar buf[ONIGENC_CODE_TO_MBC_MAXLEN];
	UChar* bufe = buf + ONIGENC_CODE_TO_MBC_MAXLEN;
	UChar* psave = p;
	int i, base = tok->base;

	buf[0] = tok->u.c;
	for (i = 1; i < ONIGENC_MBC_MAXLEN(env->enc); i++) {
	  r = fetch_token_in_cc(tok, &p, end, env);
	  if (r < 0) goto err;
	  if (r != TK_RAW_BYTE || tok->base != base) {
	    fetched = 1;
	    break;
	  }
	  buf[i] = tok->u.c;
	}

	if (i < ONIGENC_MBC_MINLEN(env->enc)) {
	  r = ONIGERR_TOO_SHORT_MULTI_BYTE_STRING;
	  goto err;
	}

	len = enclen(env->enc, buf);
	if (i < len) {
	  r = ONIGERR_TOO_SHORT_MULTI_BYTE_STRING;
	  goto err;
	}
	else if (i > len) { /* fetch back */
	  p = psave;
	  for (i = 1; i < len; i++) {
	    r = fetch_token_in_cc(tok, &p, end, env);
	  }
	  fetched = 0;
	}

	if (i == 1) {
	  v = (OnigCodePoint )buf[0];
	  goto raw_single;
	}
	else {
	  v = ONIGENC_MBC_TO_CODE(env->enc, buf, bufe);
	  in_type = CCV_CODE_POINT;
	}
      }
      else {
	v = (OnigCodePoint )tok->u.c;
      raw_single:
	in_type = CCV_SB;
      }
      in_israw = 1;
      goto val_entry2;
      break;

    case TK_CODE_POINT:
      v = tok->u.code;
      in_israw = 1;
    val_entry:
      len = ONIGENC_CODE_TO_MBCLEN(env->enc, v);
      if (len < 0) {
	r = len;
	goto err;
      }
      in_type = (len == 1 ? CCV_SB : CCV_CODE_POINT);
    val_entry2:
      r = next_state_val(cc, &vs, v, &val_israw, in_israw, in_type, &val_type,
			 &state, env);
      if (r != 0) goto err;
      break;

    case TK_POSIX_BRACKET_OPEN:
      r = parse_posix_bracket(cc, &p, end, env);
      if (r < 0) goto err;
      if (r == 1) {  /* is not POSIX bracket */
	CC_ESC_WARN(env, (UChar* )"[");
	p = tok->backp;
	v = (OnigCodePoint )tok->u.c;
	in_israw = 0;
	goto val_entry;
      }
      goto next_class;
      break;

    case TK_CHAR_TYPE:
      r = add_ctype_to_cc(cc, tok->u.prop.ctype, tok->u.prop.not, env);
      if (r != 0) return r;

    next_class:
      r = next_state_class(cc, &vs, &val_type, &state, env);
      if (r != 0) goto err;
      break;

    case TK_CHAR_PROPERTY:
      {
	int ctype;

	ctype = fetch_char_property_to_ctype(&p, end, env);
	if (ctype < 0) return ctype;
	r = add_ctype_to_cc(cc, ctype, tok->u.prop.not, env);
	if (r != 0) return r;
	goto next_class;
      }
      break;

    case TK_CC_RANGE:
      if (state == CCS_VALUE) {
	r = fetch_token_in_cc(tok, &p, end, env);
	if (r < 0) goto err;
	fetched = 1;
	if (r == TK_CC_CLOSE) { /* allow [x-] */
	range_end_val:
	  v = (OnigCodePoint )'-';
	  in_israw = 0;
	  goto val_entry;
	}
	else if (r == TK_CC_AND) {
	  CC_ESC_WARN(env, (UChar* )"-");
	  goto range_end_val;
	}
	state = CCS_RANGE;
      }
      else if (state == CCS_START) {
	/* [-xa] is allowed */
	v = (OnigCodePoint )tok->u.c;
	in_israw = 0;

	r = fetch_token_in_cc(tok, &p, end, env);
	if (r < 0) goto err;
	fetched = 1;
	/* [--x] or [a&&-x] is warned. */
	if (r == TK_CC_RANGE || and_start != 0)
	  CC_ESC_WARN(env, (UChar* )"-");

	goto val_entry;
      }
      else if (state == CCS_RANGE) {
	CC_ESC_WARN(env, (UChar* )"-");
	goto sb_char;  /* [!--x] is allowed */
      }
      else { /* CCS_COMPLETE */
	r = fetch_token_in_cc(tok, &p, end, env);
	if (r < 0) goto err;
	fetched = 1;
	if (r == TK_CC_CLOSE) goto range_end_val; /* allow [a-b-] */
	else if (r == TK_CC_AND) {
	  CC_ESC_WARN(env, (UChar* )"-");
	  goto range_end_val;
	}
	
	if (IS_SYNTAX_BV(env->syntax, ONIG_SYN_ALLOW_DOUBLE_RANGE_OP_IN_CC)) {
	  CC_ESC_WARN(env, (UChar* )"-");
	  goto sb_char;   /* [0-9-a] is allowed as [0-9\-a] */
	}
	r = ONIGERR_UNMATCHED_RANGE_SPECIFIER_IN_CHAR_CLASS;
	goto err;
      }
      break;

    case TK_CC_CC_OPEN: /* [ */
      {
	Node *anode;
	CClassNode* acc;

	r = parse_char_class(&anode, tok, &p, end, env);
	if (r != 0) goto cc_open_err;
	acc = NCCLASS(anode);
	r = or_cclass(cc, acc, env->enc);

	onig_node_free(anode);
      cc_open_err:
	if (r != 0) goto err;
      }
      break;

    case TK_CC_AND: /* && */
      {
	if (state == CCS_VALUE) {
	  r = next_state_val(cc, &vs, 0, &val_israw, 0, val_type,
			     &val_type, &state, env);
	  if (r != 0) goto err;
	}
	/* initialize local variables */
	and_start = 1;
	state = CCS_START;

	if (IS_NOT_NULL(prev_cc)) {
	  r = and_cclass(prev_cc, cc, env->enc);
	  if (r != 0) goto err;
	  bbuf_free(cc->mbuf);
	}
	else {
	  prev_cc = cc;
	  cc = &work_cc;
	}
	initialize_cclass(cc);
      }
      break;

    case TK_EOT:
      r = ONIGERR_PREMATURE_END_OF_CHAR_CLASS;
      goto err;
      break;
    default:
      r = ONIGERR_PARSER_BUG;
      goto err;
      break;
    }

    if (fetched)
      r = tok->type;
    else {
      r = fetch_token_in_cc(tok, &p, end, env);
      if (r < 0) goto err;
    }
  }

  if (state == CCS_VALUE) {
    r = next_state_val(cc, &vs, 0, &val_israw, 0, val_type,
		       &val_type, &state, env);
    if (r != 0) goto err;
  }

  if (IS_NOT_NULL(prev_cc)) {
    r = and_cclass(prev_cc, cc, env->enc);
    if (r != 0) goto err;
    bbuf_free(cc->mbuf);
    cc = prev_cc;
  }

  if (neg != 0)
    NCCLASS_SET_NOT(cc);
  else
    NCCLASS_CLEAR_NOT(cc);
  if (IS_NCCLASS_NOT(cc) &&
      IS_SYNTAX_BV(env->syntax, ONIG_SYN_NOT_NEWLINE_IN_NEGATIVE_CC)) {
    int is_empty;

    is_empty = (IS_NULL(cc->mbuf) ? 1 : 0);
    if (is_empty != 0)
      BITSET_IS_EMPTY(cc->bs, is_empty);

    if (is_empty == 0) {
#define NEWLINE_CODE    0x0a

      if (ONIGENC_IS_CODE_NEWLINE(env->enc, NEWLINE_CODE)) {
        if (ONIGENC_CODE_TO_MBCLEN(env->enc, NEWLINE_CODE) == 1)
          BITSET_SET_BIT(cc->bs, NEWLINE_CODE);
        else
          add_code_range(&(cc->mbuf), env, NEWLINE_CODE, NEWLINE_CODE);
      }
    }
  }
  *src = p;
  return 0;

 err:
  if (cc != NCCLASS(*np))
    bbuf_free(cc->mbuf);
  onig_node_free(*np);
  return r;
}

static int parse_subexp(Node** top, OnigToken* tok, int term,
			UChar** src, UChar* end, ScanEnv* env);

static int
parse_enclose(Node** np, OnigToken* tok, int term, UChar** src, UChar* end,
	      ScanEnv* env)
{
  int r, num;
  Node *target;
  OnigOptionType option;
  OnigCodePoint c;
  OnigEncoding enc = env->enc;

#ifdef USE_NAMED_GROUP
  int list_capture;
#endif

  UChar* p = *src;
  PFETCH_READY;

  *np = NULL;
  if (PEND) return ONIGERR_END_PATTERN_WITH_UNMATCHED_PARENTHESIS;

  option = env->option;
  if (PPEEK_IS('?') &&
      IS_SYNTAX_OP2(env->syntax, ONIG_SYN_OP2_QMARK_GROUP_EFFECT)) {
    PINC;
    if (PEND) return ONIGERR_END_PATTERN_IN_GROUP;

    PFETCH(c);
    switch (c) {
    case ':':   /* (?:...) grouping only */
    group:
      r = fetch_token(tok, &p, end, env);
      if (r < 0) return r;
      r = parse_subexp(np, tok, term, &p, end, env);
      if (r < 0) return r;
      *src = p;
      return 1; /* group */
      break;

    case '=':
      *np = onig_node_new_anchor(ANCHOR_PREC_READ);
      break;
    case '!':  /*         preceding read */
      *np = onig_node_new_anchor(ANCHOR_PREC_READ_NOT);
      break;
    case '>':            /* (?>...) stop backtrack */
      *np = node_new_enclose(ENCLOSE_STOP_BACKTRACK);
      break;

#ifdef USE_NAMED_GROUP
    case '\'':
      if (IS_SYNTAX_OP2(env->syntax, ONIG_SYN_OP2_QMARK_LT_NAMED_GROUP)) {
	goto named_group1;
      }
      else
	return ONIGERR_UNDEFINED_GROUP_OPTION;
      break;
#endif

    case '<':   /* look behind (?<=...), (?<!...) */
      PFETCH(c);
      if (c == '=')
	*np = onig_node_new_anchor(ANCHOR_LOOK_BEHIND);
      else if (c == '!')
	*np = onig_node_new_anchor(ANCHOR_LOOK_BEHIND_NOT);
#ifdef USE_NAMED_GROUP
      else {
	if (IS_SYNTAX_OP2(env->syntax, ONIG_SYN_OP2_QMARK_LT_NAMED_GROUP)) {
	  UChar *name;
	  UChar *name_end;

	  PUNFETCH;
	  c = '<';

	named_group1:
	  list_capture = 0;

	named_group2:
	  name = p;
	  r = fetch_name((OnigCodePoint )c, &p, end, &name_end, env, &num, 0);
	  if (r < 0) return r;

	  num = scan_env_add_mem_entry(env);
	  if (num < 0) return num;
	  if (list_capture != 0 && num >= (int )BIT_STATUS_BITS_NUM)
	    return ONIGERR_GROUP_NUMBER_OVER_FOR_CAPTURE_HISTORY;

	  r = name_add(env->reg, name, name_end, num, env);
	  if (r != 0) return r;
	  *np = node_new_enclose_memory(env->option, 1);
	  CHECK_NULL_RETURN_MEMERR(*np);
	  NENCLOSE(*np)->regnum = num;
	  if (list_capture != 0)
	    BIT_STATUS_ON_AT_SIMPLE(env->capture_history, num);
	  env->num_named++;
	}
	else {
	  return ONIGERR_UNDEFINED_GROUP_OPTION;
	}
      }
#else
      else {
	return ONIGERR_UNDEFINED_GROUP_OPTION;
      }
#endif
      break;

    case '@':
      if (IS_SYNTAX_OP2(env->syntax, ONIG_SYN_OP2_ATMARK_CAPTURE_HISTORY)) {
#ifdef USE_NAMED_GROUP
	if (IS_SYNTAX_OP2(env->syntax, ONIG_SYN_OP2_QMARK_LT_NAMED_GROUP)) {
	  PFETCH(c);
	  if (c == '<' || c == '\'') {
	    list_capture = 1;
	    goto named_group2; /* (?@<name>...) */
	  }
	  PUNFETCH;
	}
#endif
	*np = node_new_enclose_memory(env->option, 0);
	CHECK_NULL_RETURN_MEMERR(*np);
	num = scan_env_add_mem_entry(env);
	if (num < 0) {
	  onig_node_free(*np);
	  return num;
	}
	else if (num >= (int )BIT_STATUS_BITS_NUM) {
	  onig_node_free(*np);
	  return ONIGERR_GROUP_NUMBER_OVER_FOR_CAPTURE_HISTORY;
	}
	NENCLOSE(*np)->regnum = num;
	BIT_STATUS_ON_AT_SIMPLE(env->capture_history, num);
      }
      else {
	return ONIGERR_UNDEFINED_GROUP_OPTION;
      }
      break;

#ifdef USE_POSIXLINE_OPTION
    case 'p':
#endif
    case '-': case 'i': case 'm': case 's': case 'x':
      {
	int neg = 0;

	while (1) {
	  switch (c) {
	  case ':':
	  case ')':
	  break;

	  case '-':  neg = 1; break;
	  case 'x':  ONOFF(option, ONIG_OPTION_EXTEND,     neg); break;
	  case 'i':  ONOFF(option, ONIG_OPTION_IGNORECASE, neg); break;
	  case 's':
	    if (IS_SYNTAX_OP2(env->syntax, ONIG_SYN_OP2_OPTION_PERL)) {
	      ONOFF(option, ONIG_OPTION_MULTILINE,  neg);
	    }
	    else
	      return ONIGERR_UNDEFINED_GROUP_OPTION;
	    break;

	  case 'm':
	    if (IS_SYNTAX_OP2(env->syntax, ONIG_SYN_OP2_OPTION_PERL)) {
	      ONOFF(option, ONIG_OPTION_SINGLELINE, (neg == 0 ? 1 : 0));
	    }
	    else if (IS_SYNTAX_OP2(env->syntax, ONIG_SYN_OP2_OPTION_RUBY)) {
	      ONOFF(option, ONIG_OPTION_MULTILINE,  neg);
	    }
	    else
	      return ONIGERR_UNDEFINED_GROUP_OPTION;
	    break;
#ifdef USE_POSIXLINE_OPTION
	  case 'p':
	    ONOFF(option, ONIG_OPTION_MULTILINE|ONIG_OPTION_SINGLELINE, neg);
	    break;
#endif
	  default:
	    return ONIGERR_UNDEFINED_GROUP_OPTION;
	  }

	  if (c == ')') {
	    *np = node_new_option(option);
	    CHECK_NULL_RETURN_MEMERR(*np);
	    *src = p;
	    return 2; /* option only */
	  }
	  else if (c == ':') {
	    OnigOptionType prev = env->option;

	    env->option     = option;
	    r = fetch_token(tok, &p, end, env);
	    if (r < 0) return r;
	    r = parse_subexp(&target, tok, term, &p, end, env);
	    env->option = prev;
	    if (r < 0) return r;
	    *np = node_new_option(option);
	    CHECK_NULL_RETURN_MEMERR(*np);
	    NENCLOSE(*np)->target = target;
	    *src = p;
	    return 0;
	  }

	  if (PEND) return ONIGERR_END_PATTERN_IN_GROUP;
	  PFETCH(c);
	}
      }
      break;

    default:
      return ONIGERR_UNDEFINED_GROUP_OPTION;
    }
  }
  else {
    if (ONIG_IS_OPTION_ON(env->option, ONIG_OPTION_DONT_CAPTURE_GROUP))
      goto group;

    *np = node_new_enclose_memory(env->option, 0);
    CHECK_NULL_RETURN_MEMERR(*np);
    num = scan_env_add_mem_entry(env);
    if (num < 0) return num;
    NENCLOSE(*np)->regnum = num;
  }

  CHECK_NULL_RETURN_MEMERR(*np);
  r = fetch_token(tok, &p, end, env);
  if (r < 0) return r;
  r = parse_subexp(&target, tok, term, &p, end, env);
  if (r < 0) return r;

  if (NTYPE(*np) == NT_ANCHOR)
    NANCHOR(*np)->target = target;
  else {
    NENCLOSE(*np)->target = target;
    if (NENCLOSE(*np)->type == ENCLOSE_MEMORY) {
      /* Don't move this to previous of parse_subexp() */
      r = scan_env_set_mem_node(env, NENCLOSE(*np)->regnum, *np);
      if (r != 0) return r;
    }
  }

  *src = p;
  return 0;
}

static const char* PopularQStr[] = {
  "?", "*", "+", "??", "*?", "+?"
};

static const char* ReduceQStr[] = {
  "", "", "*", "*?", "??", "+ and ??", "+? and ?"
};

static int
set_quantifier(Node* qnode, Node* target, int group, ScanEnv* env)
{
  QtfrNode* qn;

  qn = NQTFR(qnode);
  if (qn->lower == 1 && qn->upper == 1) {
    return 1;
  }

  switch (NTYPE(target)) {
  case NT_STR:
    if (! group) {
      StrNode* sn = NSTR(target);
      if (str_node_can_be_split(sn, env->enc)) {
	Node* n = str_node_split_last_char(sn, env->enc);
	if (IS_NOT_NULL(n)) {
	  qn->target = n;
	  return 2;
	}
      }
    }
    break;

  case NT_QTFR:
    { /* check redundant double repeat. */
      /* verbose warn (?:.?)? etc... but not warn (.?)? etc... */
      QtfrNode* qnt   = NQTFR(target);
      int nestq_num   = popular_quantifier_num(qn);
      int targetq_num = popular_quantifier_num(qnt);

#ifdef USE_WARNING_REDUNDANT_NESTED_REPEAT_OPERATOR
      if (!IS_QUANTIFIER_BY_NUMBER(qn) && !IS_QUANTIFIER_BY_NUMBER(qnt) &&
	  IS_SYNTAX_BV(env->syntax, ONIG_SYN_WARN_REDUNDANT_NESTED_REPEAT)) {
        UChar buf[WARN_BUFSIZE];

        switch(ReduceTypeTable[targetq_num][nestq_num]) {
        case RQ_ASIS:
          break;

        case RQ_DEL:
          if (onig_verb_warn != onig_null_warn) {
            onig_snprintf_with_pattern(buf, WARN_BUFSIZE, env->enc,
                                 env->pattern, env->pattern_end,
                                 (UChar* )"redundant nested repeat operator");
            (*onig_verb_warn)((char* )buf);
          }
          goto warn_exit;
          break;

        default:
          if (onig_verb_warn != onig_null_warn) {
            onig_snprintf_with_pattern(buf, WARN_BUFSIZE, env->enc,
                                       env->pattern, env->pattern_end,
            (UChar* )"nested repeat operator %s and %s was replaced with '%s'",
            PopularQStr[targetq_num], PopularQStr[nestq_num],
            ReduceQStr[ReduceTypeTable[targetq_num][nestq_num]]);
            (*onig_verb_warn)((char* )buf);
          }
          goto warn_exit;
          break;
        }
      }

    warn_exit:
#endif
      if (targetq_num >= 0) {
	if (nestq_num >= 0) {
	  onig_reduce_nested_quantifier(qnode, target);
	  goto q_exit;
	}
	else if (targetq_num == 1 || targetq_num == 2) { /* * or + */
	  /* (?:a*){n,m}, (?:a+){n,m} => (?:a*){n,n}, (?:a+){n,n} */
	  if (! IS_REPEAT_INFINITE(qn->upper) && qn->upper > 1 && qn->greedy) {
	    qn->upper = (qn->lower == 0 ? 1 : qn->lower);
	  }
	}
      }
    }
    break;

  default:
    break;
  }

  qn->target = target;
 q_exit:
  return 0;
}


#ifdef USE_SHARED_CCLASS_TABLE

#define THRESHOLD_RANGE_NUM_FOR_SHARE_CCLASS     8

/* for ctype node hash table */

typedef struct {
  OnigEncoding enc;
  int not;
  int type;
} type_cclass_key;

static int type_cclass_cmp(type_cclass_key* x, type_cclass_key* y)
{
  if (x->type != y->type) return 1;
  if (x->enc  != y->enc)  return 1;
  if (x->not  != y->not)  return 1;
  return 0;
}

static int type_cclass_hash(type_cclass_key* key)
{
  int i, val;
  UChar *p;

  val = 0;

  p = (UChar* )&(key->enc);
  for (i = 0; i < (int )sizeof(key->enc); i++) {
    val = val * 997 + (int )*p++;
  }

  p = (UChar* )(&key->type);
  for (i = 0; i < (int )sizeof(key->type); i++) {
    val = val * 997 + (int )*p++;
  }

  val += key->not;
  return val + (val >> 5);
}

static struct st_hash_type type_type_cclass_hash = {
    type_cclass_cmp,
    type_cclass_hash,
};

static st_table* OnigTypeCClassTable;


static int
i_free_shared_class(type_cclass_key* key, Node* node, void* arg ARG_UNUSED)
{
  if (IS_NOT_NULL(node)) {
    CClassNode* cc = NCCLASS(node);
    if (IS_NOT_NULL(cc->mbuf)) xfree(cc->mbuf);
    xfree(node);
  }

  if (IS_NOT_NULL(key)) xfree(key);
  return ST_DELETE;
}

extern int
onig_free_shared_cclass_table(void)
{
  if (IS_NOT_NULL(OnigTypeCClassTable)) {
    onig_st_foreach(OnigTypeCClassTable, i_free_shared_class, 0);
    onig_st_free_table(OnigTypeCClassTable);
    OnigTypeCClassTable = NULL;
  }

  return 0;
}

#endif /* USE_SHARED_CCLASS_TABLE */


#ifndef CASE_FOLD_IS_APPLIED_INSIDE_NEGATIVE_CCLASS
static int
clear_not_flag_cclass(CClassNode* cc, OnigEncoding enc)
{
  BBuf *tbuf;
  int r;

  if (IS_NCCLASS_NOT(cc)) {
    bitset_invert(cc->bs);

    if (! ONIGENC_IS_SINGLEBYTE(enc)) {
      r = not_code_range_buf(enc, cc->mbuf, &tbuf);
      if (r != 0) return r;

      bbuf_free(cc->mbuf);
      cc->mbuf = tbuf;
    }

    NCCLASS_CLEAR_NOT(cc);
  }

  return 0;
}
#endif /* CASE_FOLD_IS_APPLIED_INSIDE_NEGATIVE_CCLASS */

typedef struct {
  ScanEnv*    env;
  CClassNode* cc;
  Node*       alt_root;
  Node**      ptail;
} IApplyCaseFoldArg;

static int
i_apply_case_fold(OnigCodePoint from, OnigCodePoint to[],
		  int to_len, void* arg)
{
  IApplyCaseFoldArg* iarg;
  ScanEnv* env;
  CClassNode* cc;
  BitSetRef bs;

  iarg = (IApplyCaseFoldArg* )arg;
  env = iarg->env;
  cc  = iarg->cc;
  bs = cc->bs;

  if (to_len == 1) {
    int is_in = onig_is_code_in_cc(env->enc, from, cc);
#ifdef CASE_FOLD_IS_APPLIED_INSIDE_NEGATIVE_CCLASS
    if ((is_in != 0 && !IS_NCCLASS_NOT(cc)) ||
	(is_in == 0 &&  IS_NCCLASS_NOT(cc))) {
      if (ONIGENC_MBC_MINLEN(env->enc) > 1 || *to >= SINGLE_BYTE_SIZE) {
	add_code_range(&(cc->mbuf), env, *to, *to);
      }
      else {
	BITSET_SET_BIT(bs, *to);
      }
    }
#else
    if (is_in != 0) {
      if (ONIGENC_MBC_MINLEN(env->enc) > 1 || *to >= SINGLE_BYTE_SIZE) {
	if (IS_NCCLASS_NOT(cc)) clear_not_flag_cclass(cc, env->enc);
	add_code_range(&(cc->mbuf), env, *to, *to);
      }
      else {
	if (IS_NCCLASS_NOT(cc)) {
	  BITSET_CLEAR_BIT(bs, *to);
	}
	else
	  BITSET_SET_BIT(bs, *to);
      }
    }
#endif /* CASE_FOLD_IS_APPLIED_INSIDE_NEGATIVE_CCLASS */
  }
  else {
    int r, i, len;
    UChar buf[ONIGENC_CODE_TO_MBC_MAXLEN];
    Node *snode = NULL_NODE;

    if (onig_is_code_in_cc(env->enc, from, cc)
#ifdef CASE_FOLD_IS_APPLIED_INSIDE_NEGATIVE_CCLASS
	&& !IS_NCCLASS_NOT(cc)
#endif
	) {
      for (i = 0; i < to_len; i++) {
	len = ONIGENC_CODE_TO_MBC(env->enc, to[i], buf);
	if (i == 0) {
	  snode = onig_node_new_str(buf, buf + len);
	  CHECK_NULL_RETURN_MEMERR(snode);

	  /* char-class expanded multi-char only
	     compare with string folded at match time. */
	  NSTRING_SET_AMBIG(snode);
	}
	else {
	  r = onig_node_str_cat(snode, buf, buf + len);
	  if (r < 0) {
	    onig_node_free(snode);
	    return r;
	  }
	}
      }

      *(iarg->ptail) = onig_node_new_alt(snode, NULL_NODE);
      CHECK_NULL_RETURN_MEMERR(*(iarg->ptail));
      iarg->ptail = &(NCDR((*(iarg->ptail))));
    }
  }

  return 0;
}

static int
parse_exp(Node** np, OnigToken* tok, int term,
	  UChar** src, UChar* end, ScanEnv* env)
{
  int r, len, group = 0;
  Node* qn;
  Node** targetp;

  *np = NULL;
  if (tok->type == (enum TokenSyms )term)
    goto end_of_token;

  switch (tok->type) {
  case TK_ALT:
  case TK_EOT:
  end_of_token:
  *np = node_new_empty();
  return tok->type;
  break;

  case TK_SUBEXP_OPEN:
    r = parse_enclose(np, tok, TK_SUBEXP_CLOSE, src, end, env);
    if (r < 0) return r;
    if (r == 1) group = 1;
    else if (r == 2) { /* option only */
      Node* target;
      OnigOptionType prev = env->option;

      env->option = NENCLOSE(*np)->option;
      r = fetch_token(tok, src, end, env);
      if (r < 0) return r;
      r = parse_subexp(&target, tok, term, src, end, env);
      env->option = prev;
      if (r < 0) return r;
      NENCLOSE(*np)->target = target;	
      return tok->type;
    }
    break;

  case TK_SUBEXP_CLOSE:
    if (! IS_SYNTAX_BV(env->syntax, ONIG_SYN_ALLOW_UNMATCHED_CLOSE_SUBEXP))
      return ONIGERR_UNMATCHED_CLOSE_PARENTHESIS;

    if (tok->escaped) goto tk_raw_byte;
    else goto tk_byte;
    break;

  case TK_STRING:
  tk_byte:
    {
      *np = node_new_str(tok->backp, *src);
      CHECK_NULL_RETURN_MEMERR(*np);

      while (1) {
	r = fetch_token(tok, src, end, env);
	if (r < 0) return r;
	if (r != TK_STRING) break;

	r = onig_node_str_cat(*np, tok->backp, *src);
	if (r < 0) return r;
      }

    string_end:
      targetp = np;
      goto repeat;
    }
    break;

  case TK_RAW_BYTE:
  tk_raw_byte:
    {
      *np = node_new_str_raw_char((UChar )tok->u.c);
      CHECK_NULL_RETURN_MEMERR(*np);
      len = 1;
      while (1) {
	if (len >= ONIGENC_MBC_MINLEN(env->enc)) {
	  if (len == enclen(env->enc, NSTR(*np)->s)) {
	    r = fetch_token(tok, src, end, env);
	    NSTRING_CLEAR_RAW(*np);
	    goto string_end;
	  }
	}

	r = fetch_token(tok, src, end, env);
	if (r < 0) return r;
	if (r != TK_RAW_BYTE) {
	  /* Don't use this, it is wrong for little endian encodings. */
#ifdef USE_PAD_TO_SHORT_BYTE_CHAR
	  int rem;
	  if (len < ONIGENC_MBC_MINLEN(env->enc)) {
	    rem = ONIGENC_MBC_MINLEN(env->enc) - len;
	    (void )node_str_head_pad(NSTR(*np), rem, (UChar )0);
	    if (len + rem == enclen(env->enc, NSTR(*np)->s)) {
	      NSTRING_CLEAR_RAW(*np);
	      goto string_end;
	    }
	  }
#endif
	  return ONIGERR_TOO_SHORT_MULTI_BYTE_STRING;
	}

	r = node_str_cat_char(*np, (UChar )tok->u.c);
	if (r < 0) return r;

	len++;
      }
    }
    break;

  case TK_CODE_POINT:
    {
      UChar buf[ONIGENC_CODE_TO_MBC_MAXLEN];
      int num = ONIGENC_CODE_TO_MBC(env->enc, tok->u.code, buf);
      if (num < 0) return num;
#ifdef NUMBERED_CHAR_IS_NOT_CASE_AMBIG
      *np = node_new_str_raw(buf, buf + num);
#else
      *np = node_new_str(buf, buf + num);
#endif
      CHECK_NULL_RETURN_MEMERR(*np);
    }
    break;

  case TK_QUOTE_OPEN:
    {
      OnigCodePoint end_op[2];
      UChar *qstart, *qend, *nextp;

      end_op[0] = (OnigCodePoint )MC_ESC(env->syntax);
      end_op[1] = (OnigCodePoint )'E';
      qstart = *src;
      qend = find_str_position(end_op, 2, qstart, end, &nextp, env->enc);
      if (IS_NULL(qend)) {
	nextp = qend = end;
      }
      *np = node_new_str(qstart, qend);
      CHECK_NULL_RETURN_MEMERR(*np);
      *src = nextp;
    }
    break;

  case TK_CHAR_TYPE:
    {
      switch (tok->u.prop.ctype) {
      case ONIGENC_CTYPE_WORD:
	*np = node_new_ctype(tok->u.prop.ctype, tok->u.prop.not);
	CHECK_NULL_RETURN_MEMERR(*np);
	break;

      case ONIGENC_CTYPE_SPACE:
      case ONIGENC_CTYPE_DIGIT:
      case ONIGENC_CTYPE_XDIGIT:
	{
	  CClassNode* cc;

#ifdef USE_SHARED_CCLASS_TABLE
          const OnigCodePoint *mbr;
	  OnigCodePoint sb_out;

          r = ONIGENC_GET_CTYPE_CODE_RANGE(env->enc, tok->u.prop.ctype,
					   &sb_out, &mbr);
          if (r == 0 &&
              ONIGENC_CODE_RANGE_NUM(mbr)
              >= THRESHOLD_RANGE_NUM_FOR_SHARE_CCLASS) {
            type_cclass_key  key;
            type_cclass_key* new_key;

            key.enc  = env->enc;
            key.not  = tok->u.prop.not;
            key.type = tok->u.prop.ctype;

            THREAD_ATOMIC_START;

            if (IS_NULL(OnigTypeCClassTable)) {
              OnigTypeCClassTable
                = onig_st_init_table_with_size(&type_type_cclass_hash, 10);
              if (IS_NULL(OnigTypeCClassTable)) {
                THREAD_ATOMIC_END;
                return ONIGERR_MEMORY;
              }
            }
            else {
              if (onig_st_lookup(OnigTypeCClassTable, (st_data_t )&key,
                                 (st_data_t* )np)) {
                THREAD_ATOMIC_END;
                break;
              }
            }

            *np = node_new_cclass_by_codepoint_range(tok->u.prop.not,
						     sb_out, mbr);
            if (IS_NULL(*np)) {
              THREAD_ATOMIC_END;
              return ONIGERR_MEMORY;
            }

            cc = NCCLASS(*np);
            NCCLASS_SET_SHARE(cc);
            new_key = (type_cclass_key* )xmalloc(sizeof(type_cclass_key));
	    xmemcpy(new_key, &key, sizeof(type_cclass_key));
            onig_st_add_direct(OnigTypeCClassTable, (st_data_t )new_key,
                               (st_data_t )*np);
            
            THREAD_ATOMIC_END;
          }
          else {
#endif
            *np = node_new_cclass();
            CHECK_NULL_RETURN_MEMERR(*np);
            cc = NCCLASS(*np);
            add_ctype_to_cc(cc, tok->u.prop.ctype, 0, env);
            if (tok->u.prop.not != 0) NCCLASS_SET_NOT(cc);
#ifdef USE_SHARED_CCLASS_TABLE
          }
#endif
	}
	break;

      default:
	return ONIGERR_PARSER_BUG;
	break;
      }
    }
    break;

  case TK_CHAR_PROPERTY:
    r = parse_char_property(np, tok, src, end, env);
    if (r != 0) return r;
    break;

  case TK_CC_OPEN:
    {
      CClassNode* cc;

      r = parse_char_class(np, tok, src, end, env);
      if (r != 0) return r;

      cc = NCCLASS(*np);
      if (IS_IGNORECASE(env->option)) {
	IApplyCaseFoldArg iarg;

	iarg.env      = env;
	iarg.cc       = cc;
	iarg.alt_root = NULL_NODE;
	iarg.ptail    = &(iarg.alt_root);

	r = ONIGENC_APPLY_ALL_CASE_FOLD(env->enc, env->case_fold_flag,
					i_apply_case_fold, &iarg);
	if (r != 0) {
	  onig_node_free(iarg.alt_root);
	  return r;
	}
	if (IS_NOT_NULL(iarg.alt_root)) {
          Node* work = onig_node_new_alt(*np, iarg.alt_root);
          if (IS_NULL(work)) {
            onig_node_free(iarg.alt_root);
            return ONIGERR_MEMORY;
          }
          *np = work;
	}
      }
    }
    break;

  case TK_ANYCHAR:
    *np = node_new_anychar();
    CHECK_NULL_RETURN_MEMERR(*np);
    break;

  case TK_ANYCHAR_ANYTIME:
    *np = node_new_anychar();
    CHECK_NULL_RETURN_MEMERR(*np);
    qn = node_new_quantifier(0, REPEAT_INFINITE, 0);
    CHECK_NULL_RETURN_MEMERR(qn);
    NQTFR(qn)->target = *np;
    *np = qn;
    break;

  case TK_BACKREF:
    len = tok->u.backref.num;
    *np = node_new_backref(len,
		   (len > 1 ? tok->u.backref.refs : &(tok->u.backref.ref1)),
			   tok->u.backref.by_name,
#ifdef USE_BACKREF_WITH_LEVEL
			   tok->u.backref.exist_level,
			   tok->u.backref.level,
#endif
			   env);
    CHECK_NULL_RETURN_MEMERR(*np);
    break;

#ifdef USE_SUBEXP_CALL
  case TK_CALL:
    {
      int gnum = tok->u.call.gnum;

      if (gnum < 0) {
	gnum = BACKREF_REL_TO_ABS(gnum, env);
	if (gnum <= 0)
	  return ONIGERR_INVALID_BACKREF;
      }
      *np = node_new_call(tok->u.call.name, tok->u.call.name_end, gnum);
      CHECK_NULL_RETURN_MEMERR(*np);
      env->num_call++;
    }
    break;
#endif

  case TK_ANCHOR:
    *np = onig_node_new_anchor(tok->u.anchor);
    break;

  case TK_OP_REPEAT:
  case TK_INTERVAL:
    if (IS_SYNTAX_BV(env->syntax, ONIG_SYN_CONTEXT_INDEP_REPEAT_OPS)) {
      if (IS_SYNTAX_BV(env->syntax, ONIG_SYN_CONTEXT_INVALID_REPEAT_OPS))
	return ONIGERR_TARGET_OF_REPEAT_OPERATOR_NOT_SPECIFIED;
      else
	*np = node_new_empty();
    }
    else {
      goto tk_byte;
    }
    break;

  default:
    return ONIGERR_PARSER_BUG;
    break;
  }

  {
    targetp = np;

  re_entry:
    r = fetch_token(tok, src, end, env);
    if (r < 0) return r;

  repeat:
    if (r == TK_OP_REPEAT || r == TK_INTERVAL) {
      if (is_invalid_quantifier_target(*targetp))
	return ONIGERR_TARGET_OF_REPEAT_OPERATOR_INVALID;

      qn = node_new_quantifier(tok->u.repeat.lower, tok->u.repeat.upper,
			       (r == TK_INTERVAL ? 1 : 0));
      CHECK_NULL_RETURN_MEMERR(qn);
      NQTFR(qn)->greedy = tok->u.repeat.greedy;
      r = set_quantifier(qn, *targetp, group, env);
      if (r < 0) {
	onig_node_free(qn);
	return r;
      }

      if (tok->u.repeat.possessive != 0) {
	Node* en;
	en = node_new_enclose(ENCLOSE_STOP_BACKTRACK);
	if (IS_NULL(en)) {
	  onig_node_free(qn);
	  return ONIGERR_MEMORY;
	}
	NENCLOSE(en)->target = qn;
	qn = en;
      }

      if (r == 0) {
	*targetp = qn;
      }
      else if (r == 1) {
	onig_node_free(qn);
      }
      else if (r == 2) { /* split case: /abc+/ */
	Node *tmp;

	*targetp = node_new_list(*targetp, NULL);
	if (IS_NULL(*targetp)) {
	  onig_node_free(qn);
	  return ONIGERR_MEMORY;
	}
	tmp = NCDR(*targetp) = node_new_list(qn, NULL);
	if (IS_NULL(tmp)) {
	  onig_node_free(qn);
	  return ONIGERR_MEMORY;
	}
	targetp = &(NCAR(tmp));
      }
      goto re_entry;
    }
  }

  return r;
}

static int
parse_branch(Node** top, OnigToken* tok, int term,
	     UChar** src, UChar* end, ScanEnv* env)
{
  int r;
  Node *node, **headp;

  *top = NULL;
  r = parse_exp(&node, tok, term, src, end, env);
  if (r < 0) return r;

  if (r == TK_EOT || r == term || r == TK_ALT) {
    *top = node;
  }
  else {
    *top  = node_new_list(node, NULL);
    headp = &(NCDR(*top));
    while (r != TK_EOT && r != term && r != TK_ALT) {
      r = parse_exp(&node, tok, term, src, end, env);
      if (r < 0) return r;

      if (NTYPE(node) == NT_LIST) {
	*headp = node;
	while (IS_NOT_NULL(NCDR(node))) node = NCDR(node);
	headp = &(NCDR(node));
      }
      else {
	*headp = node_new_list(node, NULL);
	headp = &(NCDR(*headp));
      }
    }
  }

  return r;
}

/* term_tok: TK_EOT or TK_SUBEXP_CLOSE */
static int
parse_subexp(Node** top, OnigToken* tok, int term,
	     UChar** src, UChar* end, ScanEnv* env)
{
  int r;
  Node *node, **headp;

  *top = NULL;
  r = parse_branch(&node, tok, term, src, end, env);
  if (r < 0) {
    onig_node_free(node);
    return r;
  }

  if (r == term) {
    *top = node;
  }
  else if (r == TK_ALT) {
    *top  = onig_node_new_alt(node, NULL);
    headp = &(NCDR(*top));
    while (r == TK_ALT) {
      r = fetch_token(tok, src, end, env);
      if (r < 0) return r;
      r = parse_branch(&node, tok, term, src, end, env);
      if (r < 0) return r;

      *headp = onig_node_new_alt(node, NULL);
      headp = &(NCDR(*headp));
    }

    if (tok->type != (enum TokenSyms )term)
      goto err;
  }
  else {
  err:
    if (term == TK_SUBEXP_CLOSE)
      return ONIGERR_END_PATTERN_WITH_UNMATCHED_PARENTHESIS;
    else
      return ONIGERR_PARSER_BUG;
  }

  return r;
}

static int
parse_regexp(Node** top, UChar** src, UChar* end, ScanEnv* env)
{
  int r;
  OnigToken tok;

  r = fetch_token(&tok, src, end, env);
  if (r < 0) return r;
  r = parse_subexp(top, &tok, TK_EOT, src, end, env);
  if (r < 0) return r;
  return 0;
}

extern int
onig_parse_make_tree(Node** root, const UChar* pattern, const UChar* end,
		     regex_t* reg, ScanEnv* env)
{
  int r;
  UChar* p;

#ifdef USE_NAMED_GROUP
  names_clear(reg);
#endif

  scan_env_clear(env);
  env->option         = reg->options;
  env->case_fold_flag = reg->case_fold_flag;
  env->enc            = reg->enc;
  env->syntax         = reg->syntax;
  env->pattern        = (UChar* )pattern;
  env->pattern_end    = (UChar* )end;
  env->reg            = reg;

  *root = NULL;
  p = (UChar* )pattern;
  r = parse_regexp(root, &p, (UChar* )end, env);
  reg->num_mem = env->num_mem;
  return r;
}

extern void
onig_scan_env_set_error_string(ScanEnv* env, int ecode ARG_UNUSED,
				UChar* arg, UChar* arg_end)
{
  env->error     = arg;
  env->error_end = arg_end;
}
      if (IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_OCTAL3)) {
	prev = p;
	num = scan_unsigned_octal_number(&p, end, (c == '0' ? 2:3), enc);
	if (num < 0 || num >= 256) return ONIGERR_TOO_BIG_NUMBER;
	if (p == prev) {  /* can't read nothing. */
	  num = 0; /* but, it's not error */
	}
	tok->type = TK_RAW_BYTE;
	tok->base = 8;
	tok->u.c  = num;
      }
      else if (c != '0') {
	PINC;
      }
      break;

#ifdef USE_NAMED_GROUP
    case 'k':
      if (IS_SYNTAX_OP2(syn, ONIG_SYN_OP2_ESC_K_NAMED_BACKREF)) {
	PFETCH(c);
	if (c == '<' || c == '\'') {
	  UChar* name_end;
	  int* backs;
	  int back_num;

	  prev = p;

#ifdef USE_BACKREF_WITH_LEVEL
	  name_end = NULL_UCHARP; /* no need. escape gcc warning. */
	  r = fetch_name_with_level((OnigCodePoint )c, &p, end, &name_end,
				    env, &back_num, &tok->u.backref.level);
	  if (r == 1) tok->u.backref.exist_level = 1;
	  else        tok->u.backref.exist_level = 0;
#else
	  r = fetch_name(&p, end, &name_end, env, &back_num, 1);
#endif
	  if (r < 0) return r;

	  if (back_num != 0) {
	    if (back_num < 0) {
	      back_num = BACKREF_REL_TO_ABS(back_num, env);
	      if (back_num <= 0)
		return ONIGERR_INVALID_BACKREF;
	    }

	    if (IS_SYNTAX_BV(syn, ONIG_SYN_STRICT_CHECK_BACKREF)) {
	      if (back_num > env->num_mem ||
		  IS_NULL(SCANENV_MEM_NODES(env)[back_num]))
		return ONIGERR_INVALID_BACKREF;
	    }
	    tok->type = TK_BACKREF;
	    tok->u.backref.by_name = 0;
	    tok->u.backref.num  = 1;
	    tok->u.backref.ref1 = back_num;
	  }
    else if (*type == CCV_CODE_POINT) {
      r = add_code_range(&(cc->mbuf), env, *vs, *vs);
      if (r < 0) return r;
    }
  }

  if (*state != CCS_START)
    *state = CCS_VALUE;

  *type  = CCV_CLASS;
  return 0;
}

static int
next_state_val(CClassNode* cc, OnigCodePoint *vs, OnigCodePoint v,
	       int* vs_israw, int v_israw,
	       enum CCVALTYPE intype, enum CCVALTYPE* type,
	       enum CCSTATE* state, ScanEnv* env)
{
  int r;

  switch (*state) {
  case CCS_VALUE:
    if (*type == CCV_SB)
    {
    if (*vs > 0xff)
      return ONIGERR_INVALID_CODE_POINT_VALUE;
      BITSET_SET_BIT(cc->bs, (int )(*vs));
    }
  switch (*state) {
  case CCS_VALUE:
    if (*type == CCV_SB)
    {
    if (*vs > 0xff)
      return ONIGERR_INVALID_CODE_POINT_VALUE;
      BITSET_SET_BIT(cc->bs, (int )(*vs));
    }