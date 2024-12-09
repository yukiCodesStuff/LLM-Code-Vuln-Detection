	int state = 0;
	int crlf_state = -1;
	char *token = NULL;
	size_t token_pos;
	zend_string *fld_name, *fld_val;

	ps = str;
	icnt = str_len;
				}

				if (state == 0 || state == 1) {
					if(token) {
						fld_name = zend_string_init(token, token_pos, 0);
					}
					state = 2;
				} else {

					case 3:
						if (crlf_state == -1) {
							if(token) {
								fld_val = zend_string_init(token, token_pos, 0);
							}

							if (fld_name != NULL && fld_val != NULL) {
		state = 3;
	}
	if (state == 3) {
		if(token) {
			fld_val = zend_string_init(token, token_pos, 0);
		}
		if (fld_name != NULL && fld_val != NULL) {
			zval val;