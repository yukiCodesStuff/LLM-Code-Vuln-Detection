	if (IS_FIND_LONGEST(option) && DATA_ENSURE_CHECK1) {
	  goto fail; /* for retry */
	}
      }

      /* default behavior: return first-matching result. */
      goto finish;
      break;

    case OP_EXACT1:  MOP_IN(OP_EXACT1);
      DATA_ENSURE(1);
      if (*p != *s) goto fail;
      p++; s++;
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

      if (q >= end) return 0; /* fail */
      while (p < q) p += enclen(reg->enc, p);
    }
      if (reg->dmax != ONIG_INFINITE_DISTANCE) {
	*low = p - reg->dmax;
	if (p - str < reg->dmax) {
	  *low = (UChar* )str;
	  if (low_prev)
	    *low_prev = onigenc_get_prev_char_head(reg->enc, str, *low);
	}
	else {
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
      }
	else {
	  if (low_prev)
	    *low_prev = onigenc_get_prev_char_head(reg->enc,
					       (pprev ? pprev : str), *low);
	}
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
	}
	break;

      case ANCHOR_END_LINE:
	if (ON_STR_END(p)) {
#ifndef USE_NEWLINE_AT_END_OF_STRING_HAS_EMPTY_LINE
	  prev = onigenc_get_prev_char_head(reg->enc, adjrange, p);
	  if (IS_NULL(prev)) goto fail;
	  if (ONIGENC_IS_MBC_NEWLINE(reg->enc, prev, end)) {
	    p = prev;
	    goto retry;
	  }
#endif
	}
	else if (! ONIGENC_IS_MBC_NEWLINE(reg->enc, p, end)
#ifdef USE_CRNL_AS_LINE_TERMINATOR
              && ! ONIGENC_IS_MBC_CRNL(reg->enc, p, end)
#endif
                ) {
	  p = onigenc_get_prev_char_head(reg->enc, adjrange, p);
	  if (IS_NULL(p)) goto fail;
	  goto retry;
	}
	break;
      }
    }

    /* no needs to adjust *high, *high is used as range check only */
    if (reg->dmax != ONIG_INFINITE_DISTANCE) {
      *low  = p - reg->dmax;
      *high = p - reg->dmin;
      *high = onigenc_get_right_adjust_char_head(reg->enc, adjrange, *high);
    }

#ifdef ONIG_DEBUG_SEARCH
    fprintf(stderr, "backward_search_range: low: %d, high: %d\n",
	    (int )(*low - str), (int )(*high - str));
#endif
    return 1; /* success */
  }

 fail:
#ifdef ONIG_DEBUG_SEARCH
  fprintf(stderr, "backward_search_range: fail.\n");
#endif
  return 0; /* fail */
}


extern int
onig_search(regex_t* reg, const UChar* str, const UChar* end,
	    const UChar* start, const UChar* range, OnigRegion* region, OnigOptionType option)
{
  int r;
  UChar *s, *prev;
  OnigMatchArg msa;
  const UChar *orig_start = start;
#ifdef USE_MATCH_RANGE_MUST_BE_INSIDE_OF_SPECIFIED_RANGE
  const UChar *orig_range = range;
#endif

#if defined(USE_RECOMPILE_API) && defined(USE_MULTI_THREAD_SYSTEM)
 start:
  THREAD_ATOMIC_START;
  if (ONIG_STATE(reg) >= ONIG_STATE_NORMAL) {
    ONIG_STATE_INC(reg);
    if (IS_NOT_NULL(reg->chain) && ONIG_STATE(reg) == ONIG_STATE_NORMAL) {
      onig_chain_reduce(reg);
      ONIG_STATE_INC(reg);
    }
  }
  else {
    int n;

    THREAD_ATOMIC_END;
    n = 0;
    while (ONIG_STATE(reg) < ONIG_STATE_NORMAL) {
      if (++n > THREAD_PASS_LIMIT_COUNT)
	return ONIGERR_OVER_THREAD_PASS_LIMIT_COUNT;
      THREAD_PASS;
    }
    goto start;
  }
  THREAD_ATOMIC_END;
#endif /* USE_RECOMPILE_API && USE_MULTI_THREAD_SYSTEM */

#ifdef ONIG_DEBUG_SEARCH
  fprintf(stderr,
     "onig_search (entry point): str: %d, end: %d, start: %d, range: %d\n",
     (int )str, (int )(end - str), (int )(start - str), (int )(range - str));
#endif

  if (region
#ifdef USE_POSIX_API_REGION_OPTION
      && !IS_POSIX_REGION(option)
#endif
      ) {
    r = onig_region_resize_clear(region, reg->num_mem + 1);
    if (r) goto finish_no_msa;
  }

  if (start > end || start < str) goto mismatch_no_msa;


#ifdef USE_MATCH_RANGE_MUST_BE_INSIDE_OF_SPECIFIED_RANGE
#ifdef USE_FIND_LONGEST_SEARCH_ALL_OF_RANGE
#define MATCH_AND_RETURN_CHECK(upper_range) \
  r = match_at(reg, str, end, (upper_range), s, prev, &msa); \
  if (r != ONIG_MISMATCH) {\
    if (r >= 0) {\
      if (! IS_FIND_LONGEST(reg->options)) {\
        goto match;\
      }\
    }\
    else goto finish; /* error */ \
  }
#else
#define MATCH_AND_RETURN_CHECK(upper_range) \
  r = match_at(reg, str, end, (upper_range), s, prev, &msa); \
  if (r != ONIG_MISMATCH) {\
    if (r >= 0) {\
      goto match;\
    }\
    else goto finish; /* error */ \
  }
#endif /* USE_FIND_LONGEST_SEARCH_ALL_OF_RANGE */
#else
#ifdef USE_FIND_LONGEST_SEARCH_ALL_OF_RANGE
#define MATCH_AND_RETURN_CHECK(none) \
  r = match_at(reg, str, end, s, prev, &msa);\
  if (r != ONIG_MISMATCH) {\
    if (r >= 0) {\
      if (! IS_FIND_LONGEST(reg->options)) {\
        goto match;\
      }\
    }\
    else goto finish; /* error */ \
  }
#else
#define MATCH_AND_RETURN_CHECK(none) \
  r = match_at(reg, str, end, s, prev, &msa);\
  if (r != ONIG_MISMATCH) {\
    if (r >= 0) {\
      goto match;\
    }\
    else goto finish; /* error */ \
  }
#endif /* USE_FIND_LONGEST_SEARCH_ALL_OF_RANGE */
#endif /* USE_MATCH_RANGE_MUST_BE_INSIDE_OF_SPECIFIED_RANGE */


  /* anchor optimize: resume search range */
  if (reg->anchor != 0 && str < end) {
    UChar *min_semi_end, *max_semi_end;

    if (reg->anchor & ANCHOR_BEGIN_POSITION) {
      /* search start-position only */
    begin_position:
      if (range > start)
	range = start + 1;
      else
	range = start;
    }
    else if (reg->anchor & ANCHOR_BEGIN_BUF) {
      /* search str-position only */
      if (range > start) {
	if (start != str) goto mismatch_no_msa;
	range = str + 1;
      }
      else {
	if (range <= str) {
	  start = str;
	  range = str;
	}
	else
	  goto mismatch_no_msa;
      }
    }
    else if (reg->anchor & ANCHOR_END_BUF) {
      min_semi_end = max_semi_end = (UChar* )end;

    end_buf:
      if ((OnigDistance )(max_semi_end - str) < reg->anchor_dmin)
	goto mismatch_no_msa;

      if (range > start) {
	if ((OnigDistance )(min_semi_end - start) > reg->anchor_dmax) {
	  start = min_semi_end - reg->anchor_dmax;
	  if (start < end)
	    start = onigenc_get_right_adjust_char_head(reg->enc, str, start);
	  else { /* match with empty at end */
	    start = onigenc_get_prev_char_head(reg->enc, str, end);
	  }
	}
	if ((OnigDistance )(max_semi_end - (range - 1)) < reg->anchor_dmin) {
	  range = max_semi_end - reg->anchor_dmin + 1;
	}

	if (start >= range) goto mismatch_no_msa;
      }
      else {
	if ((OnigDistance )(min_semi_end - range) > reg->anchor_dmax) {
	  range = min_semi_end - reg->anchor_dmax;
	}
	if ((OnigDistance )(max_semi_end - start) < reg->anchor_dmin) {
	  start = max_semi_end - reg->anchor_dmin;
	  start = ONIGENC_LEFT_ADJUST_CHAR_HEAD(reg->enc, str, start);
	}
	if (range > start) goto mismatch_no_msa;
      }
    }
    else if (reg->anchor & ANCHOR_SEMI_END_BUF) {
      UChar* pre_end = ONIGENC_STEP_BACK(reg->enc, str, end, 1);

      max_semi_end = (UChar* )end;
      if (ONIGENC_IS_MBC_NEWLINE(reg->enc, pre_end, end)) {
	min_semi_end = pre_end;

#ifdef USE_CRNL_AS_LINE_TERMINATOR
	pre_end = ONIGENC_STEP_BACK(reg->enc, str, pre_end, 1);
	if (IS_NOT_NULL(pre_end) &&
	    ONIGENC_IS_MBC_CRNL(reg->enc, pre_end, end)) {
	  min_semi_end = pre_end;
	}
#endif
	if (min_semi_end > str && start <= min_semi_end) {
	  goto end_buf;
	}
      }
      else {
	min_semi_end = (UChar* )end;
	goto end_buf;
      }
    }
    else if ((reg->anchor & ANCHOR_ANYCHAR_STAR_ML)) {
      goto begin_position;
    }
  }
  else if (str == end) { /* empty string */
    static const UChar* address_for_empty_string = (UChar* )"";

#ifdef ONIG_DEBUG_SEARCH
    fprintf(stderr, "onig_search: empty string.\n");
#endif

    if (reg->threshold_len == 0) {
      start = end = str = address_for_empty_string;
      s = (UChar* )start;
      prev = (UChar* )NULL;

      MATCH_ARG_INIT(msa, option, region, start);
#ifdef USE_COMBINATION_EXPLOSION_CHECK
      msa.state_check_buff = (void* )0;
      msa.state_check_buff_size = 0;   /* NO NEED, for valgrind */
#endif
      MATCH_AND_RETURN_CHECK(end);
      goto mismatch;
    }
    goto mismatch_no_msa;
  }

#ifdef ONIG_DEBUG_SEARCH
  fprintf(stderr, "onig_search(apply anchor): end: %d, start: %d, range: %d\n",
	  (int )(end - str), (int )(start - str), (int )(range - str));
#endif

  MATCH_ARG_INIT(msa, option, region, orig_start);
#ifdef USE_COMBINATION_EXPLOSION_CHECK
  {
    int offset = (MIN(start, range) - str);
    STATE_CHECK_BUFF_INIT(msa, end - str, offset, reg->num_comb_exp_check);
  }
#endif

  s = (UChar* )start;
  if (range > start) {   /* forward search */
    if (s > str)
      prev = onigenc_get_prev_char_head(reg->enc, str, s);
    else
      prev = (UChar* )NULL;

    if (reg->optimize != ONIG_OPTIMIZE_NONE) {
      UChar *sch_range, *low, *high, *low_prev;

      sch_range = (UChar* )range;
      if (reg->dmax != 0) {
	if (reg->dmax == ONIG_INFINITE_DISTANCE)
	  sch_range = (UChar* )end;
	else {
	  sch_range += reg->dmax;
	  if (sch_range > end) sch_range = (UChar* )end;
	}
      }

      if ((end - start) < reg->threshold_len)
        goto mismatch;

      if (reg->dmax != ONIG_INFINITE_DISTANCE) {
	do {
	  if (! forward_search_range(reg, str, end, s, sch_range,
				     &low, &high, &low_prev)) goto mismatch;
	  if (s < low) {
	    s    = low;
	    prev = low_prev;
	  }
	  while (s <= high) {
	    MATCH_AND_RETURN_CHECK(orig_range);
	    prev = s;
	    s += enclen(reg->enc, s);
	  }
	} while (s < range);
	goto mismatch;
      }
      else { /* check only. */
	if (! forward_search_range(reg, str, end, s, sch_range,
				   &low, &high, (UChar** )NULL)) goto mismatch;

        if ((reg->anchor & ANCHOR_ANYCHAR_STAR) != 0) {
          do {
            MATCH_AND_RETURN_CHECK(orig_range);
            prev = s;
            s += enclen(reg->enc, s);

            while (!ONIGENC_IS_MBC_NEWLINE(reg->enc, prev, end) && s < range) {
              prev = s;
              s += enclen(reg->enc, s);
            }
          } while (s < range);
          goto mismatch;
        }
      }
    }

    do {
      MATCH_AND_RETURN_CHECK(orig_range);
      prev = s;
      s += enclen(reg->enc, s);
    } while (s < range);

    if (s == range) { /* because empty match with /$/. */
      MATCH_AND_RETURN_CHECK(orig_range);
    }
  }
  else {  /* backward search */
#ifdef USE_MATCH_RANGE_MUST_BE_INSIDE_OF_SPECIFIED_RANGE
    if (orig_start < end)
      orig_start += enclen(reg->enc, orig_start); /* is upper range */
#endif

    if (reg->optimize != ONIG_OPTIMIZE_NONE) {
      UChar *low, *high, *adjrange, *sch_start;

      if (range < end)
	adjrange = ONIGENC_LEFT_ADJUST_CHAR_HEAD(reg->enc, str, range);
      else
	adjrange = (UChar* )end;

      if (reg->dmax != ONIG_INFINITE_DISTANCE &&
	  (end - range) >= reg->threshold_len) {
	do {
	  sch_start = s + reg->dmax;
	  if (sch_start > end) sch_start = (UChar* )end;
	  if (backward_search_range(reg, str, end, sch_start, range, adjrange,
				    &low, &high) <= 0)
	    goto mismatch;

	  if (s > high)
	    s = high;

	  while (s >= low) {
	    prev = onigenc_get_prev_char_head(reg->enc, str, s);
	    MATCH_AND_RETURN_CHECK(orig_start);
	    s = prev;
	  }
	} while (s >= range);
	goto mismatch;
      }
      else { /* check only. */
	if ((end - range) < reg->threshold_len) goto mismatch;

	sch_start = s;
	if (reg->dmax != 0) {
	  if (reg->dmax == ONIG_INFINITE_DISTANCE)
	    sch_start = (UChar* )end;
	  else {
	    sch_start += reg->dmax;
	    if (sch_start > end) sch_start = (UChar* )end;
	    else
	      sch_start = ONIGENC_LEFT_ADJUST_CHAR_HEAD(reg->enc,
						    start, sch_start);
	  }
	}
	if (backward_search_range(reg, str, end, sch_start, range, adjrange,
				  &low, &high) <= 0) goto mismatch;
      }
    }

    do {
      prev = onigenc_get_prev_char_head(reg->enc, str, s);
      MATCH_AND_RETURN_CHECK(orig_start);
      s = prev;
    } while (s >= range);
  }

 mismatch:
#ifdef USE_FIND_LONGEST_SEARCH_ALL_OF_RANGE
  if (IS_FIND_LONGEST(reg->options)) {
    if (msa.best_len >= 0) {
      s = msa.best_s;
      goto match;
    }
  }
#endif
  r = ONIG_MISMATCH;

 finish:
  MATCH_ARG_FREE(msa);
  ONIG_STATE_DEC_THREAD(reg);

  /* If result is mismatch and no FIND_NOT_EMPTY option,
     then the region is not setted in match_at(). */
  if (IS_FIND_NOT_EMPTY(reg->options) && region
#ifdef USE_POSIX_API_REGION_OPTION
      && !IS_POSIX_REGION(option)
#endif
      ) {
    onig_region_clear(region);
  }

#ifdef ONIG_DEBUG
  if (r != ONIG_MISMATCH)
    fprintf(stderr, "onig_search: error %d\n", r);
#endif
  return r;

 mismatch_no_msa:
  r = ONIG_MISMATCH;
 finish_no_msa:
  ONIG_STATE_DEC_THREAD(reg);
#ifdef ONIG_DEBUG
  if (r != ONIG_MISMATCH)
    fprintf(stderr, "onig_search: error %d\n", r);
#endif
  return r;

 match:
  ONIG_STATE_DEC_THREAD(reg);
  MATCH_ARG_FREE(msa);
  return s - str;
}

extern OnigEncoding
onig_get_encoding(regex_t* reg)
{
  return reg->enc;
}

extern OnigOptionType
onig_get_options(regex_t* reg)
{
  return reg->options;
}

extern  OnigCaseFoldType
onig_get_case_fold_flag(regex_t* reg)
{
  return reg->case_fold_flag;
}

extern OnigSyntaxType*
onig_get_syntax(regex_t* reg)
{
  return reg->syntax;
}

extern int
onig_number_of_captures(regex_t* reg)
{
  return reg->num_mem;
}

extern int
onig_number_of_capture_histories(regex_t* reg)
{
#ifdef USE_CAPTURE_HISTORY
  int i, n;

  n = 0;
  for (i = 0; i <= ONIG_MAX_CAPTURE_HISTORY_GROUP; i++) {
    if (BIT_STATUS_AT(reg->capture_history, i) != 0)
      n++;
  }
  return n;
#else
  return 0;
#endif
}

extern void
onig_copy_encoding(OnigEncoding to, OnigEncoding from)
{
  *to = *from;
}
