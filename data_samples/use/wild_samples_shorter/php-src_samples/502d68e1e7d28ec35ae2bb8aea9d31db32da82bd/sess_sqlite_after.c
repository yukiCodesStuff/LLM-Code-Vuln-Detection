		case SQLITE_ROW:
			if (rowdata[0] != NULL) {
				*vallen = strlen(rowdata[0]);
				if (*vallen) {
					*val = emalloc(*vallen);
					*vallen = sqlite_decode_binary(rowdata[0], *val);
					(*val)[*vallen] = '\0';
				} else {
					*val = STR_EMPTY_ALLOC();
				}
			}
			break;
		default:
			sqlite_freemem(error);