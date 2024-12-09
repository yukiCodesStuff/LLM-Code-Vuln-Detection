	PUNFETCH;
	prev = p;
	num = scan_unsigned_octal_number(&p, end, 3, enc);
	if (num < 0 || num >= 256) return ONIGERR_TOO_BIG_NUMBER;
	if (p == prev) {  /* can't read nothing. */
	  num = 0; /* but, it's not error */
	}
	tok->type = TK_RAW_BYTE;
      if (IS_SYNTAX_OP(syn, ONIG_SYN_OP_ESC_OCTAL3)) {
	prev = p;
	num = scan_unsigned_octal_number(&p, end, (c == '0' ? 2:3), enc);
	if (num < 0 || num >= 256) return ONIGERR_TOO_BIG_NUMBER;
	if (p == prev) {  /* can't read nothing. */
	  num = 0; /* but, it's not error */
	}
	tok->type = TK_RAW_BYTE;
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