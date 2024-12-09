					} else {
						SEPARATE_ZVAL_IF_NOT_REF(elem);
						convert_scalar_to_number(*elem TSRMLS_CC);
						d = (Z_TYPE_PP(elem) == IS_DOUBLE)
							? Z_DVAL_PP(elem)
							: (double)Z_LVAL_PP(elem);
					}
					formattable.setDouble(d);
					break;
				}
			case Formattable::kLong:
				{
					int32_t tInt32;
retry_klong:
					if (Z_TYPE_PP(elem) == IS_DOUBLE) {
						if (Z_DVAL_PP(elem) > (double)INT32_MAX ||
								Z_DVAL_PP(elem) < (double)INT32_MIN) {
							intl_errors_set(&err, U_ILLEGAL_ARGUMENT_ERROR,
								"Found PHP float with absolute value too large for "
								"32 bit integer argument", 0 TSRMLS_CC);
						} else {
							tInt32 = (int32_t)Z_DVAL_PP(elem);
						}
					} else if (Z_TYPE_PP(elem) == IS_LONG) {
						if (Z_LVAL_PP(elem) > INT32_MAX ||
								Z_LVAL_PP(elem) < INT32_MIN) {
							intl_errors_set(&err, U_ILLEGAL_ARGUMENT_ERROR,
								"Found PHP integer with absolute value too large "
								"for 32 bit integer argument", 0 TSRMLS_CC);
						} else {
							tInt32 = (int32_t)Z_LVAL_PP(elem);
						}
					} else {
						SEPARATE_ZVAL_IF_NOT_REF(elem);
						convert_scalar_to_number(*elem TSRMLS_CC);
						goto retry_klong;
					}
					formattable.setLong(tInt32);
					break;
				}
			case Formattable::kInt64:
				{
					int64_t tInt64;
retry_kint64:
					if (Z_TYPE_PP(elem) == IS_DOUBLE) {
						if (Z_DVAL_PP(elem) > (double)U_INT64_MAX ||
								Z_DVAL_PP(elem) < (double)U_INT64_MIN) {
							intl_errors_set(&err, U_ILLEGAL_ARGUMENT_ERROR,
								"Found PHP float with absolute value too large for "
								"64 bit integer argument", 0 TSRMLS_CC);
						} else {
							tInt64 = (int64_t)Z_DVAL_PP(elem);
						}
					} else if (Z_TYPE_PP(elem) == IS_LONG) {
						/* assume long is not wider than 64 bits */
						tInt64 = (int64_t)Z_LVAL_PP(elem);
					} else {
						SEPARATE_ZVAL_IF_NOT_REF(elem);
						convert_scalar_to_number(*elem TSRMLS_CC);
						goto retry_kint64;
					}
					formattable.setInt64(tInt64);
					break;
				}
			case Formattable::kDate:
				{
					double dd = intl_zval_to_millis(*elem, &err, "msgfmt_format" TSRMLS_CC);
					if (U_FAILURE(err.code)) {
						char *message, *key_char;
						int key_len;
						UErrorCode status = UErrorCode();
						if (intl_charFromString(key, &key_char, &key_len,
								&status) == SUCCESS) {
							spprintf(&message, 0, "The argument for key '%s' "
								"cannot be used as a date or time", key_char);
							intl_errors_set(&err, err.code, message, 1 TSRMLS_CC);
							efree(key_char);
							efree(message);
						}
						continue;
					}
					formattable.setDate(dd);
					break;
				}
			default:
				intl_errors_set(&err, U_ILLEGAL_ARGUMENT_ERROR,
					"Found unsupported argument type", 0 TSRMLS_CC);
				break;
			}
		} else {
			/* We couldn't find any information about the argument in the pattern, this
			 * means it's an extra argument. So convert it to a number if it's a number or
			 * bool or null and to a string if it's anything else except arrays . */
			switch (Z_TYPE_PP(elem)) {
			case IS_DOUBLE:
				formattable.setDouble(Z_DVAL_PP(elem));
				break;
			case IS_BOOL:
				convert_to_long_ex(elem);
				/* Intentional fallthrough */
			case IS_LONG:
				formattable.setInt64((int64_t)Z_LVAL_PP(elem));
				break;
			case IS_NULL:
				formattable.setInt64((int64_t)0);
				break;
			case IS_STRING:
			case IS_OBJECT:
				goto string_arg;
			default:
				{
					char *message, *key_char;
					int key_len;
					UErrorCode status = UErrorCode();
					if (intl_charFromString(key, &key_char, &key_len,
							&status) == SUCCESS) {
						spprintf(&message, 0, "No strategy to convert the "
							"value given for the argument with key '%s' "
							"is available", key_char);
						intl_errors_set(&err,
							U_ILLEGAL_ARGUMENT_ERROR, message, 1 TSRMLS_CC);
						efree(key_char);
						efree(message);
					}
				}
			}
		}
	} // visiting each argument

	if (U_FAILURE(err.code)) {
		return;
	}

	UnicodeString resultStr;
	FieldPosition fieldPosition(0);

	/* format the message */
	mf->format(farg_names.empty() ? NULL : &farg_names[0],
		fargs.empty() ? NULL : &fargs[0], arg_count, resultStr, err.code);

	if (U_FAILURE(err.code)) {
		intl_errors_set(&err, err.code,
			"Call to ICU MessageFormat::format() has failed", 0 TSRMLS_CC);
		return;
	}

	*formatted_len = resultStr.length();
	*formatted = eumalloc(*formatted_len+1);
	resultStr.extract(*formatted, *formatted_len+1, err.code);
	if (U_FAILURE(err.code)) {
		intl_errors_set(&err, err.code,
			"Error copying format() result", 0 TSRMLS_CC);
		return;
	}
}

#define cleanup_zvals() for(int j=i;j>=0;j--) { zval_ptr_dtor((*args)+i); }

U_CFUNC void umsg_parse_helper(UMessageFormat *fmt, int *count, zval ***args, UChar *source, int source_len, UErrorCode *status)
{
    UnicodeString srcString(source, source_len);
    Formattable *fargs = ((const MessageFormat*)fmt)->parse(srcString, *count, *status);

	if(U_FAILURE(*status)) {
		return;
	}

	*args = (zval **)safe_emalloc(*count, sizeof(zval *), 0);

    // assign formattables to varargs
    for(int32_t i = 0; i < *count; i++) {
	    int64_t aInt64;
		double aDate;
		UnicodeString temp;
		char *stmp;
		int stmp_len;

		ALLOC_INIT_ZVAL((*args)[i]);

		switch(fargs[i].getType()) {
        case Formattable::kDate:
			aDate = ((double)fargs[i].getDate())/U_MILLIS_PER_SECOND;
			ZVAL_DOUBLE((*args)[i], aDate);
            break;

        case Formattable::kDouble:
			ZVAL_DOUBLE((*args)[i], (double)fargs[i].getDouble());
            break;

        case Formattable::kLong:
			ZVAL_LONG((*args)[i], fargs[i].getLong());
            break;

        case Formattable::kInt64:
            aInt64 = fargs[i].getInt64();
			if(aInt64 > LONG_MAX || aInt64 < -LONG_MAX) {
				ZVAL_DOUBLE((*args)[i], (double)aInt64);
			} else {
				ZVAL_LONG((*args)[i], (long)aInt64);
			}
            break;

        case Formattable::kString:
            fargs[i].getString(temp);
			intl_convert_utf16_to_utf8(&stmp, &stmp_len, temp.getBuffer(), temp.length(), status);
			if(U_FAILURE(*status)) {
				cleanup_zvals();
				return;
			}
			ZVAL_STRINGL((*args)[i], stmp, stmp_len, 0);
            break;

        case Formattable::kObject:
        case Formattable::kArray:
            *status = U_ILLEGAL_ARGUMENT_ERROR;
			cleanup_zvals();
            break;
        }
    }
	delete[] fargs;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
					} else {
						SEPARATE_ZVAL_IF_NOT_REF(elem);
						convert_scalar_to_number(*elem TSRMLS_CC);
						goto retry_klong;
					}
					formattable.setLong(tInt32);
					break;
				}
			case Formattable::kInt64:
				{
					int64_t tInt64;
retry_kint64:
					if (Z_TYPE_PP(elem) == IS_DOUBLE) {
						if (Z_DVAL_PP(elem) > (double)U_INT64_MAX ||
								Z_DVAL_PP(elem) < (double)U_INT64_MIN) {
							intl_errors_set(&err, U_ILLEGAL_ARGUMENT_ERROR,
								"Found PHP float with absolute value too large for "
								"64 bit integer argument", 0 TSRMLS_CC);
						} else {
							tInt64 = (int64_t)Z_DVAL_PP(elem);
						}
					} else if (Z_TYPE_PP(elem) == IS_LONG) {
						/* assume long is not wider than 64 bits */
						tInt64 = (int64_t)Z_LVAL_PP(elem);
					} else {
						SEPARATE_ZVAL_IF_NOT_REF(elem);
						convert_scalar_to_number(*elem TSRMLS_CC);
						goto retry_kint64;
					}
					formattable.setInt64(tInt64);
					break;
				}
			case Formattable::kDate:
				{
					double dd = intl_zval_to_millis(*elem, &err, "msgfmt_format" TSRMLS_CC);
					if (U_FAILURE(err.code)) {
						char *message, *key_char;
						int key_len;
						UErrorCode status = UErrorCode();
						if (intl_charFromString(key, &key_char, &key_len,
								&status) == SUCCESS) {
							spprintf(&message, 0, "The argument for key '%s' "
								"cannot be used as a date or time", key_char);
							intl_errors_set(&err, err.code, message, 1 TSRMLS_CC);
							efree(key_char);
							efree(message);
						}
						continue;
					}
					formattable.setDate(dd);
					break;
				}
			default:
				intl_errors_set(&err, U_ILLEGAL_ARGUMENT_ERROR,
					"Found unsupported argument type", 0 TSRMLS_CC);
				break;
			}
		} else {
			/* We couldn't find any information about the argument in the pattern, this
			 * means it's an extra argument. So convert it to a number if it's a number or
			 * bool or null and to a string if it's anything else except arrays . */
			switch (Z_TYPE_PP(elem)) {
			case IS_DOUBLE:
				formattable.setDouble(Z_DVAL_PP(elem));
				break;
			case IS_BOOL:
				convert_to_long_ex(elem);
				/* Intentional fallthrough */
			case IS_LONG:
				formattable.setInt64((int64_t)Z_LVAL_PP(elem));
				break;
			case IS_NULL:
				formattable.setInt64((int64_t)0);
				break;
			case IS_STRING:
			case IS_OBJECT:
				goto string_arg;
			default:
				{
					char *message, *key_char;
					int key_len;
					UErrorCode status = UErrorCode();
					if (intl_charFromString(key, &key_char, &key_len,
							&status) == SUCCESS) {
						spprintf(&message, 0, "No strategy to convert the "
							"value given for the argument with key '%s' "
							"is available", key_char);
						intl_errors_set(&err,
							U_ILLEGAL_ARGUMENT_ERROR, message, 1 TSRMLS_CC);
						efree(key_char);
						efree(message);
					}
				}
			}
		}
	} // visiting each argument

	if (U_FAILURE(err.code)) {
		return;
	}

	UnicodeString resultStr;
	FieldPosition fieldPosition(0);

	/* format the message */
	mf->format(farg_names.empty() ? NULL : &farg_names[0],
		fargs.empty() ? NULL : &fargs[0], arg_count, resultStr, err.code);

	if (U_FAILURE(err.code)) {
		intl_errors_set(&err, err.code,
			"Call to ICU MessageFormat::format() has failed", 0 TSRMLS_CC);
		return;
	}

	*formatted_len = resultStr.length();
	*formatted = eumalloc(*formatted_len+1);
	resultStr.extract(*formatted, *formatted_len+1, err.code);
	if (U_FAILURE(err.code)) {
		intl_errors_set(&err, err.code,
			"Error copying format() result", 0 TSRMLS_CC);
		return;
	}
}

#define cleanup_zvals() for(int j=i;j>=0;j--) { zval_ptr_dtor((*args)+i); }