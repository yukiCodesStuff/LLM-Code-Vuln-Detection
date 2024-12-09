	if (IS_FIND_LONGEST(option) && DATA_ENSURE_CHECK1) {
	  goto fail; /* for retry */
	}
      }

      /* default behavior: return first-matching result. */
      goto finish;
      break;

    case OP_EXACT1:  MOP_IN(OP_EXACT1);
#if 0
      DATA_ENSURE(1);
      if (*p != *s) goto fail;
      p++; s++;
#endif
      if (*p != *s++) goto fail;
      DATA_ENSURE(0);
      p++;
      MOP_OUT;
      break;

    case OP_EXACT1_IC:  MOP_IN(OP_EXACT1_IC);
      {
	int len;
	UChar *q, lowbuf[ONIGENC_MBC_CASE_FOLD_MAXLEN];

	DATA_ENSURE(1);
	len = ONIGENC_MBC_CASE_FOLD(encode,
		    /* DISABLE_CASE_FOLD_MULTI_CHAR(case_fold_flag), */
		    case_fold_flag,
		    &s, end, lowbuf);
	DATA_ENSURE(0);
	q = lowbuf;
	while (len-- > 0) {
	  if (*p != *q) {
            goto fail;
          }
	  p++; q++;
	}
      }
    else {
      UChar *q = p + reg->dmin;
      while (p < q) p += enclen(reg->enc, p);
    }
      if (reg->dmax != ONIG_INFINITE_DISTANCE) {
	*low = p - reg->dmax;
	if (*low > s) {
	  *low = onigenc_get_right_adjust_char_head_with_prev(reg->enc, s,
							      *low, (const UChar** )low_prev);
	  if (low_prev && IS_NULL(*low_prev))
	    *low_prev = onigenc_get_prev_char_head(reg->enc,
						   (pprev ? pprev : s), *low);
	}
	else {
	  if (low_prev)
	    *low_prev = onigenc_get_prev_char_head(reg->enc,
					       (pprev ? pprev : str), *low);
	}
      }
	else {
	  if (low_prev)
	    *low_prev = onigenc_get_prev_char_head(reg->enc,
					       (pprev ? pprev : str), *low);
	}
      }
    }
    /* no needs to adjust *high, *high is used as range check only */
    *high = p - reg->dmin;

#ifdef ONIG_DEBUG_SEARCH
    fprintf(stderr,
    "forward_search_range success: low: %d, high: %d, dmin: %d, dmax: %d\n",
	    (int )(*low - str), (int )(*high - str), reg->dmin, reg->dmax);
#endif
    return 1; /* success */
  }

  return 0; /* fail */
}

static int set_bm_backward_skip P_((UChar* s, UChar* end, OnigEncoding enc,
				    int** skip));

#define BM_BACKWARD_SEARCH_LENGTH_THRESHOLD   100

static int
backward_search_range(regex_t* reg, const UChar* str, const UChar* end,
		      UChar* s, const UChar* range, UChar* adjrange,
		      UChar** low, UChar** high)
{
  int r;
  UChar *p;

  range += reg->dmin;
  p = s;

 retry:
  switch (reg->optimize) {
  case ONIG_OPTIMIZE_EXACT:
  exact_method:
    p = slow_search_backward(reg->enc, reg->exact, reg->exact_end,
			     range, adjrange, end, p);
    break;

  case ONIG_OPTIMIZE_EXACT_IC:
    p = slow_search_backward_ic(reg->enc, reg->case_fold_flag,
                                reg->exact, reg->exact_end,
                                range, adjrange, end, p);
    break;

  case ONIG_OPTIMIZE_EXACT_BM:
  case ONIG_OPTIMIZE_EXACT_BM_NOT_REV:
    if (IS_NULL(reg->int_map_backward)) {
      if (s - range < BM_BACKWARD_SEARCH_LENGTH_THRESHOLD)
	goto exact_method;

      r = set_bm_backward_skip(reg->exact, reg->exact_end, reg->enc,
			       &(reg->int_map_backward));
      if (r) return r;
    }
    p = bm_search_backward(reg, reg->exact, reg->exact_end, range, adjrange,
			   end, p);
    break;

  case ONIG_OPTIMIZE_MAP:
    p = map_search_backward(reg->enc, reg->map, range, adjrange, p);
    break;
  }

  if (p) {
    if (reg->sub_anchor) {
      UChar* prev;

      switch (reg->sub_anchor) {
      case ANCHOR_BEGIN_LINE:
	if (!ON_STR_BEGIN(p)) {
	  prev = onigenc_get_prev_char_head(reg->enc, str, p);
	  if (!ONIGENC_IS_MBC_NEWLINE(reg->enc, prev, end)) {
	    p = prev;
	    goto retry;
	  }