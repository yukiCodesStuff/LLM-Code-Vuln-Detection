			} else {
				RETVAL_STRING(output, 1);
			}

			memset(output, 0, PHP_MAX_SALT_LEN + 1);
			efree(output);
		} else if (
				salt[0] == '$' &&
				salt[1] == '2' &&
			    (salt[2] != 'a' && salt[2] != 'x') ||
				salt[3] == '$' &&
				salt[4] >= '0' && salt[4] <= '3' &&
				salt[5] >= '0' && salt[5] <= '9' &&
				salt[6] == '$') {
			char output[PHP_MAX_SALT_LEN + 1];

			memset(output, 0, PHP_MAX_SALT_LEN + 1);

			crypt_res = php_crypt_blowfish_rn(str, salt, output, sizeof(output));
			if (!crypt_res) {
				if (salt[0]=='*' && salt[1]=='0') {
					RETVAL_STRING("*1", 1);
				} else {
					RETVAL_STRING("*0", 1);
				}
			} else {
				RETVAL_STRING(output, 1);
			}

			memset(output, 0, PHP_MAX_SALT_LEN + 1);
		} else {
			memset(&buffer, 0, sizeof(buffer));
			_crypt_extended_init_r();

			crypt_res = _crypt_extended_r(str, salt, &buffer);
			if (!crypt_res) {
				if (salt[0]=='*' && salt[1]=='0') {
					RETURN_STRING("*1", 1);
				} else {
					RETURN_STRING("*0", 1);
				}
			} else {
				RETURN_STRING(crypt_res, 1);
			}
		}
	}
#else

# if defined(HAVE_CRYPT_R) && (defined(_REENTRANT) || defined(_THREAD_SAFE))
	{
#  if defined(CRYPT_R_STRUCT_CRYPT_DATA)
		struct crypt_data buffer;
		memset(&buffer, 0, sizeof(buffer));
#  elif defined(CRYPT_R_CRYPTD)
		CRYPTD buffer;
#  else
#    error Data struct used by crypt_r() is unknown. Please report.
#  endif
		crypt_res = crypt_r(str, salt, &buffer);
		if (!crypt_res) {
				if (salt[0]=='*' && salt[1]=='0') {
					RETURN_STRING("*1", 1);
				} else {
					RETURN_STRING("*0", 1);
				}
		} else {
			RETURN_STRING(crypt_res, 1);
		}
	}
# endif
#endif
}
/* }}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */