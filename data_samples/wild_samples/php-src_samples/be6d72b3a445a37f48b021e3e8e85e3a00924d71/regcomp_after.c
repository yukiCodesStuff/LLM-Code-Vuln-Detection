  for (i = to->len, p = s; p < end && i < OPT_EXACT_MAXLEN; ) {
    len = enclen(enc, p);
    if (i + len > OPT_EXACT_MAXLEN) break;
    for (j = 0; j < len && p < end; j++)
      to->s[i++] = *p++;
  }