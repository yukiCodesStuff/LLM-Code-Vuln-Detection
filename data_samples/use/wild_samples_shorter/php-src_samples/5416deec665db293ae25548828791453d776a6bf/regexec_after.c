    else {
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
					       (pprev ? pprev : str), *low);
	}
      }
      }
    }
    /* no needs to adjust *high, *high is used as range check only */
    *high = p - reg->dmin;
