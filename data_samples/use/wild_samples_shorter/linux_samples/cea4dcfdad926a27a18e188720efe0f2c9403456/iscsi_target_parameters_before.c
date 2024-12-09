	}
	INIT_LIST_HEAD(&extra_response->er_list);

	strncpy(extra_response->key, key, strlen(key) + 1);
	strncpy(extra_response->value, NOTUNDERSTOOD,
			strlen(NOTUNDERSTOOD) + 1);

	list_add_tail(&extra_response->er_list,
			&param_list->extra_response_list);
	return 0;

		if (phase & PHASE_SECURITY) {
			if (iscsi_check_for_auth_key(key) > 0) {
				char *tmpptr = key + strlen(key);
				*tmpptr = '=';
				kfree(tmpbuf);
				return 1;
			}
		}