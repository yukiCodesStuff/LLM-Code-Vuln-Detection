														   replace_value, replace_len, &Z_STRLEN(temp_result), case_sensitivity, replace_count);
			}

           str_efree(Z_STRVAL_P(result));
			Z_STRVAL_P(result) = Z_STRVAL(temp_result);
			Z_STRLEN_P(result) = Z_STRLEN(temp_result);

			if (Z_STRLEN_P(result) == 0) {