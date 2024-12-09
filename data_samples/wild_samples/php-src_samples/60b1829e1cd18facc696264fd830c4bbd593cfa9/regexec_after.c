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