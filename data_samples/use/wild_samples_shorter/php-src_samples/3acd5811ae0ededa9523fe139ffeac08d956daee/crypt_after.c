		} else if (
				salt[0] == '$' &&
				salt[1] == '2' &&
			    (salt[2] != 'a' && salt[2] != 'x') ||
				salt[3] == '$' &&
				salt[4] >= '0' && salt[4] <= '3' &&
				salt[5] >= '0' && salt[5] <= '9' &&
				salt[6] == '$') {