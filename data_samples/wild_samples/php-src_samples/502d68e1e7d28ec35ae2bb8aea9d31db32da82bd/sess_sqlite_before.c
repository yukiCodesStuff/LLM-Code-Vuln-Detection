			if (rowdata[0] != NULL) {
				*vallen = strlen(rowdata[0]);
				*val = emalloc(*vallen);
				*vallen = sqlite_decode_binary(rowdata[0], *val);
				(*val)[*vallen] = '\0';
			}