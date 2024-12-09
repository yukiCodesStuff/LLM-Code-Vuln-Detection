			} else if (Z_STRLEN_PP(search_entry) > 1) {
				Z_STRVAL(temp_result) = php_str_to_str_ex(Z_STRVAL_P(result), Z_STRLEN_P(result),
														   Z_STRVAL_PP(search_entry), Z_STRLEN_PP(search_entry),
														   replace_value, replace_len, &Z_STRLEN(temp_result), case_sensitivity, replace_count);
			}

           str_efree(Z_STRVAL_P(result));
			Z_STRVAL_P(result) = Z_STRVAL(temp_result);
			Z_STRLEN_P(result) = Z_STRLEN(temp_result);

			if (Z_STRLEN_P(result) == 0) {
				return;
			}

			zend_hash_move_forward(Z_ARRVAL_P(search));
		}
	} else {
		if (Z_STRLEN_P(search) == 1) {
			php_char_to_str_ex(Z_STRVAL_PP(subject),
							Z_STRLEN_PP(subject),
							Z_STRVAL_P(search)[0],
							Z_STRVAL_P(replace),
							Z_STRLEN_P(replace),
							result,
							case_sensitivity,
							replace_count);
		} else if (Z_STRLEN_P(search) > 1) {
			Z_STRVAL_P(result) = php_str_to_str_ex(Z_STRVAL_PP(subject), Z_STRLEN_PP(subject),
													Z_STRVAL_P(search), Z_STRLEN_P(search),
													Z_STRVAL_P(replace), Z_STRLEN_P(replace), &Z_STRLEN_P(result), case_sensitivity, replace_count);
		} else {
			MAKE_COPY_ZVAL(subject, result);
		}
	}