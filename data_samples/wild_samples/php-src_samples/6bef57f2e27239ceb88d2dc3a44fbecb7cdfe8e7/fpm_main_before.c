							if (apache_was_here) {
								/* recall that PATH_INFO won't exist */
								path_info = script_path_translated + ptlen;
								tflag = (slen != 0 && (!orig_path_info || strcmp(orig_path_info, path_info) != 0));
							} else {
								path_info = env_path_info ? env_path_info + pilen - slen : NULL;
								tflag = (orig_path_info != path_info);
							}