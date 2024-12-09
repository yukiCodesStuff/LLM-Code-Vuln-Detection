	if (!extra_response) {
		pr_err("Unable to allocate memory for"
			" struct iscsi_extra_response.\n");
		return -1;
	}
	INIT_LIST_HEAD(&extra_response->er_list);

	strlcpy(extra_response->key, key, sizeof(extra_response->key));
	strlcpy(extra_response->value, NOTUNDERSTOOD,
		sizeof(extra_response->value));

	list_add_tail(&extra_response->er_list,
			&param_list->extra_response_list);
	return 0;
}

static int iscsi_check_for_auth_key(char *key)
{
	/*
	 * RFC 1994
	 */
	if (!strcmp(key, "CHAP_A") || !strcmp(key, "CHAP_I") ||
	    !strcmp(key, "CHAP_C") || !strcmp(key, "CHAP_N") ||
	    !strcmp(key, "CHAP_R"))
		return 1;

	/*
	 * RFC 2945
	 */
	if (!strcmp(key, "SRP_U") || !strcmp(key, "SRP_N") ||
	    !strcmp(key, "SRP_g") || !strcmp(key, "SRP_s") ||
	    !strcmp(key, "SRP_A") || !strcmp(key, "SRP_B") ||
	    !strcmp(key, "SRP_M") || !strcmp(key, "SRP_HM"))
		return 1;

	return 0;
}

static void iscsi_check_proposer_for_optional_reply(struct iscsi_param *param)
{
	if (IS_TYPE_BOOL_AND(param)) {
		if (!strcmp(param->value, NO))
			SET_PSTATE_REPLY_OPTIONAL(param);
	} else if (IS_TYPE_BOOL_OR(param)) {
		if (!strcmp(param->value, YES))
			SET_PSTATE_REPLY_OPTIONAL(param);
		 /*
		  * Required for gPXE iSCSI boot client
		  */
		if (!strcmp(param->name, IMMEDIATEDATA))
			SET_PSTATE_REPLY_OPTIONAL(param);
	} else if (IS_TYPE_NUMBER(param)) {
		if (!strcmp(param->name, MAXRECVDATASEGMENTLENGTH))
			SET_PSTATE_REPLY_OPTIONAL(param);
		/*
		 * The GlobalSAN iSCSI Initiator for MacOSX does
		 * not respond to MaxBurstLength, FirstBurstLength,
		 * DefaultTime2Wait or DefaultTime2Retain parameter keys.
		 * So, we set them to 'reply optional' here, and assume the
		 * the defaults from iscsi_parameters.h if the initiator
		 * is not RFC compliant and the keys are not negotiated.
		 */
		if (!strcmp(param->name, MAXBURSTLENGTH))
			SET_PSTATE_REPLY_OPTIONAL(param);
		if (!strcmp(param->name, FIRSTBURSTLENGTH))
			SET_PSTATE_REPLY_OPTIONAL(param);
		if (!strcmp(param->name, DEFAULTTIME2WAIT))
			SET_PSTATE_REPLY_OPTIONAL(param);
		if (!strcmp(param->name, DEFAULTTIME2RETAIN))
			SET_PSTATE_REPLY_OPTIONAL(param);
		/*
		 * Required for gPXE iSCSI boot client
		 */
		if (!strcmp(param->name, MAXCONNECTIONS))
			SET_PSTATE_REPLY_OPTIONAL(param);
	} else if (IS_PHASE_DECLARATIVE(param))
		SET_PSTATE_REPLY_OPTIONAL(param);
}

static int iscsi_check_boolean_value(struct iscsi_param *param, char *value)
{
	if (strcmp(value, YES) && strcmp(value, NO)) {
		pr_err("Illegal value for \"%s\", must be either"
			" \"%s\" or \"%s\".\n", param->name, YES, NO);
		return -1;
	}

	return 0;
}

static int iscsi_check_numerical_value(struct iscsi_param *param, char *value_ptr)
{
	char *tmpptr;
	int value = 0;

	value = simple_strtoul(value_ptr, &tmpptr, 0);

	if (IS_TYPERANGE_0_TO_2(param)) {
		if ((value < 0) || (value > 2)) {
			pr_err("Illegal value for \"%s\", must be"
				" between 0 and 2.\n", param->name);
			return -1;
		}
		return 0;
	}
	if (IS_TYPERANGE_0_TO_3600(param)) {
		if ((value < 0) || (value > 3600)) {
			pr_err("Illegal value for \"%s\", must be"
				" between 0 and 3600.\n", param->name);
			return -1;
		}
		return 0;
	}
	if (IS_TYPERANGE_0_TO_32767(param)) {
		if ((value < 0) || (value > 32767)) {
			pr_err("Illegal value for \"%s\", must be"
				" between 0 and 32767.\n", param->name);
			return -1;
		}
		return 0;
	}
	if (IS_TYPERANGE_0_TO_65535(param)) {
		if ((value < 0) || (value > 65535)) {
			pr_err("Illegal value for \"%s\", must be"
				" between 0 and 65535.\n", param->name);
			return -1;
		}
		return 0;
	}
	if (IS_TYPERANGE_1_TO_65535(param)) {
		if ((value < 1) || (value > 65535)) {
			pr_err("Illegal value for \"%s\", must be"
				" between 1 and 65535.\n", param->name);
			return -1;
		}
		return 0;
	}
	if (IS_TYPERANGE_2_TO_3600(param)) {
		if ((value < 2) || (value > 3600)) {
			pr_err("Illegal value for \"%s\", must be"
				" between 2 and 3600.\n", param->name);
			return -1;
		}
		return 0;
	}
	if (IS_TYPERANGE_512_TO_16777215(param)) {
		if ((value < 512) || (value > 16777215)) {
			pr_err("Illegal value for \"%s\", must be"
				" between 512 and 16777215.\n", param->name);
			return -1;
		}
		return 0;
	}

	return 0;
}

static int iscsi_check_numerical_range_value(struct iscsi_param *param, char *value)
{
	char *left_val_ptr = NULL, *right_val_ptr = NULL;
	char *tilde_ptr = NULL;
	u32 left_val, right_val, local_left_val;

	if (strcmp(param->name, IFMARKINT) &&
	    strcmp(param->name, OFMARKINT)) {
		pr_err("Only parameters \"%s\" or \"%s\" may contain a"
		       " numerical range value.\n", IFMARKINT, OFMARKINT);
		return -1;
	}

	if (IS_PSTATE_PROPOSER(param))
		return 0;

	tilde_ptr = strchr(value, '~');
	if (!tilde_ptr) {
		pr_err("Unable to locate numerical range indicator"
			" \"~\" for \"%s\".\n", param->name);
		return -1;
	}
	*tilde_ptr = '\0';

	left_val_ptr = value;
	right_val_ptr = value + strlen(left_val_ptr) + 1;

	if (iscsi_check_numerical_value(param, left_val_ptr) < 0)
		return -1;
	if (iscsi_check_numerical_value(param, right_val_ptr) < 0)
		return -1;

	left_val = simple_strtoul(left_val_ptr, NULL, 0);
	right_val = simple_strtoul(right_val_ptr, NULL, 0);
	*tilde_ptr = '~';

	if (right_val < left_val) {
		pr_err("Numerical range for parameter \"%s\" contains"
			" a right value which is less than the left.\n",
				param->name);
		return -1;
	}

	/*
	 * For now,  enforce reasonable defaults for [I,O]FMarkInt.
	 */
	tilde_ptr = strchr(param->value, '~');
	if (!tilde_ptr) {
		pr_err("Unable to locate numerical range indicator"
			" \"~\" for \"%s\".\n", param->name);
		return -1;
	}
	*tilde_ptr = '\0';

	left_val_ptr = param->value;
	right_val_ptr = param->value + strlen(left_val_ptr) + 1;

	local_left_val = simple_strtoul(left_val_ptr, NULL, 0);
	*tilde_ptr = '~';

	if (param->set_param) {
		if ((left_val < local_left_val) ||
		    (right_val < local_left_val)) {
			pr_err("Passed value range \"%u~%u\" is below"
				" minimum left value \"%u\" for key \"%s\","
				" rejecting.\n", left_val, right_val,
				local_left_val, param->name);
			return -1;
		}
	} else {
		if ((left_val < local_left_val) &&
		    (right_val < local_left_val)) {
			pr_err("Received value range \"%u~%u\" is"
				" below minimum left value \"%u\" for key"
				" \"%s\", rejecting.\n", left_val, right_val,
				local_left_val, param->name);
			SET_PSTATE_REJECT(param);
			if (iscsi_update_param_value(param, REJECT) < 0)
				return -1;
		}
	}

	return 0;
}

static int iscsi_check_string_or_list_value(struct iscsi_param *param, char *value)
{
	if (IS_PSTATE_PROPOSER(param))
		return 0;

	if (IS_TYPERANGE_AUTH_PARAM(param)) {
		if (strcmp(value, KRB5) && strcmp(value, SPKM1) &&
		    strcmp(value, SPKM2) && strcmp(value, SRP) &&
		    strcmp(value, CHAP) && strcmp(value, NONE)) {
			pr_err("Illegal value for \"%s\", must be"
				" \"%s\", \"%s\", \"%s\", \"%s\", \"%s\""
				" or \"%s\".\n", param->name, KRB5,
					SPKM1, SPKM2, SRP, CHAP, NONE);
			return -1;
		}
	}
	if (IS_TYPERANGE_DIGEST_PARAM(param)) {
		if (strcmp(value, CRC32C) && strcmp(value, NONE)) {
			pr_err("Illegal value for \"%s\", must be"
				" \"%s\" or \"%s\".\n", param->name,
					CRC32C, NONE);
			return -1;
		}
	}
	if (IS_TYPERANGE_SESSIONTYPE(param)) {
		if (strcmp(value, DISCOVERY) && strcmp(value, NORMAL)) {
			pr_err("Illegal value for \"%s\", must be"
				" \"%s\" or \"%s\".\n", param->name,
					DISCOVERY, NORMAL);
			return -1;
		}
	}

	return 0;
}

/*
 *	This function is used to pick a value range number,  currently just
 *	returns the lesser of both right values.
 */
static char *iscsi_get_value_from_number_range(
	struct iscsi_param *param,
	char *value)
{
	char *end_ptr, *tilde_ptr1 = NULL, *tilde_ptr2 = NULL;
	u32 acceptor_right_value, proposer_right_value;

	tilde_ptr1 = strchr(value, '~');
	if (!tilde_ptr1)
		return NULL;
	*tilde_ptr1++ = '\0';
	proposer_right_value = simple_strtoul(tilde_ptr1, &end_ptr, 0);

	tilde_ptr2 = strchr(param->value, '~');
	if (!tilde_ptr2)
		return NULL;
	*tilde_ptr2++ = '\0';
	acceptor_right_value = simple_strtoul(tilde_ptr2, &end_ptr, 0);

	return (acceptor_right_value >= proposer_right_value) ?
		tilde_ptr1 : tilde_ptr2;
}

static char *iscsi_check_valuelist_for_support(
	struct iscsi_param *param,
	char *value)
{
	char *tmp1 = NULL, *tmp2 = NULL;
	char *acceptor_values = NULL, *proposer_values = NULL;

	acceptor_values = param->value;
	proposer_values = value;

	do {
		if (!proposer_values)
			return NULL;
		tmp1 = strchr(proposer_values, ',');
		if (tmp1)
			*tmp1 = '\0';
		acceptor_values = param->value;
		do {
			if (!acceptor_values) {
				if (tmp1)
					*tmp1 = ',';
				return NULL;
			}
			tmp2 = strchr(acceptor_values, ',');
			if (tmp2)
				*tmp2 = '\0';
			if (!strcmp(acceptor_values, proposer_values)) {
				if (tmp2)
					*tmp2 = ',';
				goto out;
			}
			if (tmp2)
				*tmp2++ = ',';

			acceptor_values = tmp2;
		} while (acceptor_values);
		if (tmp1)
			*tmp1++ = ',';
		proposer_values = tmp1;
	} while (proposer_values);

out:
	return proposer_values;
}

static int iscsi_check_acceptor_state(struct iscsi_param *param, char *value,
				struct iscsi_conn *conn)
{
	u8 acceptor_boolean_value = 0, proposer_boolean_value = 0;
	char *negoitated_value = NULL;

	if (IS_PSTATE_ACCEPTOR(param)) {
		pr_err("Received key \"%s\" twice, protocol error.\n",
				param->name);
		return -1;
	}

	if (IS_PSTATE_REJECT(param))
		return 0;

	if (IS_TYPE_BOOL_AND(param)) {
		if (!strcmp(value, YES))
			proposer_boolean_value = 1;
		if (!strcmp(param->value, YES))
			acceptor_boolean_value = 1;
		if (acceptor_boolean_value && proposer_boolean_value)
			do {} while (0);
		else {
			if (iscsi_update_param_value(param, NO) < 0)
				return -1;
			if (!proposer_boolean_value)
				SET_PSTATE_REPLY_OPTIONAL(param);
		}
	} else if (IS_TYPE_BOOL_OR(param)) {
		if (!strcmp(value, YES))
			proposer_boolean_value = 1;
		if (!strcmp(param->value, YES))
			acceptor_boolean_value = 1;
		if (acceptor_boolean_value || proposer_boolean_value) {
			if (iscsi_update_param_value(param, YES) < 0)
				return -1;
			if (proposer_boolean_value)
				SET_PSTATE_REPLY_OPTIONAL(param);
		}
	} else if (IS_TYPE_NUMBER(param)) {
		char *tmpptr, buf[11];
		u32 acceptor_value = simple_strtoul(param->value, &tmpptr, 0);
		u32 proposer_value = simple_strtoul(value, &tmpptr, 0);

		memset(buf, 0, sizeof(buf));

		if (!strcmp(param->name, MAXCONNECTIONS) ||
		    !strcmp(param->name, MAXBURSTLENGTH) ||
		    !strcmp(param->name, FIRSTBURSTLENGTH) ||
		    !strcmp(param->name, MAXOUTSTANDINGR2T) ||
		    !strcmp(param->name, DEFAULTTIME2RETAIN) ||
		    !strcmp(param->name, ERRORRECOVERYLEVEL)) {
			if (proposer_value > acceptor_value) {
				sprintf(buf, "%u", acceptor_value);
				if (iscsi_update_param_value(param,
						&buf[0]) < 0)
					return -1;
			} else {
				if (iscsi_update_param_value(param, value) < 0)
					return -1;
			}
		} else if (!strcmp(param->name, DEFAULTTIME2WAIT)) {
			if (acceptor_value > proposer_value) {
				sprintf(buf, "%u", acceptor_value);
				if (iscsi_update_param_value(param,
						&buf[0]) < 0)
					return -1;
			} else {
				if (iscsi_update_param_value(param, value) < 0)
					return -1;
			}
		} else {
			if (iscsi_update_param_value(param, value) < 0)
				return -1;
		}

		if (!strcmp(param->name, MAXRECVDATASEGMENTLENGTH)) {
			struct iscsi_param *param_mxdsl;
			unsigned long long tmp;
			int rc;

			rc = strict_strtoull(param->value, 0, &tmp);
			if (rc < 0)
				return -1;

			conn->conn_ops->MaxRecvDataSegmentLength = tmp;
			pr_debug("Saving op->MaxRecvDataSegmentLength from"
				" original initiator received value: %u\n",
				conn->conn_ops->MaxRecvDataSegmentLength);

			param_mxdsl = iscsi_find_param_from_key(
						MAXXMITDATASEGMENTLENGTH,
						conn->param_list);
			if (!param_mxdsl)
				return -1;

			rc = iscsi_update_param_value(param,
						param_mxdsl->value);
			if (rc < 0)
				return -1;

			pr_debug("Updated %s to target MXDSL value: %s\n",
					param->name, param->value);
		}

	} else if (IS_TYPE_NUMBER_RANGE(param)) {
		negoitated_value = iscsi_get_value_from_number_range(
					param, value);
		if (!negoitated_value)
			return -1;
		if (iscsi_update_param_value(param, negoitated_value) < 0)
			return -1;
	} else if (IS_TYPE_VALUE_LIST(param)) {
		negoitated_value = iscsi_check_valuelist_for_support(
					param, value);
		if (!negoitated_value) {
			pr_err("Proposer's value list \"%s\" contains"
				" no valid values from Acceptor's value list"
				" \"%s\".\n", value, param->value);
			return -1;
		}
		if (iscsi_update_param_value(param, negoitated_value) < 0)
			return -1;
	} else if (IS_PHASE_DECLARATIVE(param)) {
		if (iscsi_update_param_value(param, value) < 0)
			return -1;
		SET_PSTATE_REPLY_OPTIONAL(param);
	}

	return 0;
}

static int iscsi_check_proposer_state(struct iscsi_param *param, char *value)
{
	if (IS_PSTATE_RESPONSE_GOT(param)) {
		pr_err("Received key \"%s\" twice, protocol error.\n",
				param->name);
		return -1;
	}

	if (IS_TYPE_NUMBER_RANGE(param)) {
		u32 left_val = 0, right_val = 0, recieved_value = 0;
		char *left_val_ptr = NULL, *right_val_ptr = NULL;
		char *tilde_ptr = NULL;

		if (!strcmp(value, IRRELEVANT) || !strcmp(value, REJECT)) {
			if (iscsi_update_param_value(param, value) < 0)
				return -1;
			return 0;
		}

		tilde_ptr = strchr(value, '~');
		if (tilde_ptr) {
			pr_err("Illegal \"~\" in response for \"%s\".\n",
					param->name);
			return -1;
		}
		tilde_ptr = strchr(param->value, '~');
		if (!tilde_ptr) {
			pr_err("Unable to locate numerical range"
				" indicator \"~\" for \"%s\".\n", param->name);
			return -1;
		}
		*tilde_ptr = '\0';

		left_val_ptr = param->value;
		right_val_ptr = param->value + strlen(left_val_ptr) + 1;
		left_val = simple_strtoul(left_val_ptr, NULL, 0);
		right_val = simple_strtoul(right_val_ptr, NULL, 0);
		recieved_value = simple_strtoul(value, NULL, 0);

		*tilde_ptr = '~';

		if ((recieved_value < left_val) ||
		    (recieved_value > right_val)) {
			pr_err("Illegal response \"%s=%u\", value must"
				" be between %u and %u.\n", param->name,
				recieved_value, left_val, right_val);
			return -1;
		}
	} else if (IS_TYPE_VALUE_LIST(param)) {
		char *comma_ptr = NULL, *tmp_ptr = NULL;

		comma_ptr = strchr(value, ',');
		if (comma_ptr) {
			pr_err("Illegal \",\" in response for \"%s\".\n",
					param->name);
			return -1;
		}

		tmp_ptr = iscsi_check_valuelist_for_support(param, value);
		if (!tmp_ptr)
			return -1;
	}

	if (iscsi_update_param_value(param, value) < 0)
		return -1;

	return 0;
}

static int iscsi_check_value(struct iscsi_param *param, char *value)
{
	char *comma_ptr = NULL;

	if (!strcmp(value, REJECT)) {
		if (!strcmp(param->name, IFMARKINT) ||
		    !strcmp(param->name, OFMARKINT)) {
			/*
			 * Reject is not fatal for [I,O]FMarkInt,  and causes
			 * [I,O]FMarker to be reset to No. (See iSCSI v20 A.3.2)
			 */
			SET_PSTATE_REJECT(param);
			return 0;
		}
		pr_err("Received %s=%s\n", param->name, value);
		return -1;
	}
	if (!strcmp(value, IRRELEVANT)) {
		pr_debug("Received %s=%s\n", param->name, value);
		SET_PSTATE_IRRELEVANT(param);
		return 0;
	}
	if (!strcmp(value, NOTUNDERSTOOD)) {
		if (!IS_PSTATE_PROPOSER(param)) {
			pr_err("Received illegal offer %s=%s\n",
				param->name, value);
			return -1;
		}

/* #warning FIXME: Add check for X-ExtensionKey here */
		pr_err("Standard iSCSI key \"%s\" cannot be answered"
			" with \"%s\", protocol error.\n", param->name, value);
		return -1;
	}

	do {
		comma_ptr = NULL;
		comma_ptr = strchr(value, ',');

		if (comma_ptr && !IS_TYPE_VALUE_LIST(param)) {
			pr_err("Detected value separator \",\", but"
				" key \"%s\" does not allow a value list,"
				" protocol error.\n", param->name);
			return -1;
		}
		if (comma_ptr)
			*comma_ptr = '\0';

		if (strlen(value) > VALUE_MAXLEN) {
			pr_err("Value for key \"%s\" exceeds %d,"
				" protocol error.\n", param->name,
				VALUE_MAXLEN);
			return -1;
		}

		if (IS_TYPE_BOOL_AND(param) || IS_TYPE_BOOL_OR(param)) {
			if (iscsi_check_boolean_value(param, value) < 0)
				return -1;
		} else if (IS_TYPE_NUMBER(param)) {
			if (iscsi_check_numerical_value(param, value) < 0)
				return -1;
		} else if (IS_TYPE_NUMBER_RANGE(param)) {
			if (iscsi_check_numerical_range_value(param, value) < 0)
				return -1;
		} else if (IS_TYPE_STRING(param) || IS_TYPE_VALUE_LIST(param)) {
			if (iscsi_check_string_or_list_value(param, value) < 0)
				return -1;
		} else {
			pr_err("Huh? 0x%02x\n", param->type);
			return -1;
		}

		if (comma_ptr)
			*comma_ptr++ = ',';

		value = comma_ptr;
	} while (value);

	return 0;
}

static struct iscsi_param *__iscsi_check_key(
	char *key,
	int sender,
	struct iscsi_param_list *param_list)
{
	struct iscsi_param *param;

	if (strlen(key) > KEY_MAXLEN) {
		pr_err("Length of key name \"%s\" exceeds %d.\n",
			key, KEY_MAXLEN);
		return NULL;
	}

	param = iscsi_find_param_from_key(key, param_list);
	if (!param)
		return NULL;

	if ((sender & SENDER_INITIATOR) && !IS_SENDER_INITIATOR(param)) {
		pr_err("Key \"%s\" may not be sent to %s,"
			" protocol error.\n", param->name,
			(sender & SENDER_RECEIVER) ? "target" : "initiator");
		return NULL;
	}

	if ((sender & SENDER_TARGET) && !IS_SENDER_TARGET(param)) {
		pr_err("Key \"%s\" may not be sent to %s,"
			" protocol error.\n", param->name,
			(sender & SENDER_RECEIVER) ? "initiator" : "target");
		return NULL;
	}

	return param;
}

static struct iscsi_param *iscsi_check_key(
	char *key,
	int phase,
	int sender,
	struct iscsi_param_list *param_list)
{
	struct iscsi_param *param;
	/*
	 * Key name length must not exceed 63 bytes. (See iSCSI v20 5.1)
	 */
	if (strlen(key) > KEY_MAXLEN) {
		pr_err("Length of key name \"%s\" exceeds %d.\n",
			key, KEY_MAXLEN);
		return NULL;
	}

	param = iscsi_find_param_from_key(key, param_list);
	if (!param)
		return NULL;

	if ((sender & SENDER_INITIATOR) && !IS_SENDER_INITIATOR(param)) {
		pr_err("Key \"%s\" may not be sent to %s,"
			" protocol error.\n", param->name,
			(sender & SENDER_RECEIVER) ? "target" : "initiator");
		return NULL;
	}
	if ((sender & SENDER_TARGET) && !IS_SENDER_TARGET(param)) {
		pr_err("Key \"%s\" may not be sent to %s,"
				" protocol error.\n", param->name,
			(sender & SENDER_RECEIVER) ? "initiator" : "target");
		return NULL;
	}

	if (IS_PSTATE_ACCEPTOR(param)) {
		pr_err("Key \"%s\" received twice, protocol error.\n",
				key);
		return NULL;
	}

	if (!phase)
		return param;

	if (!(param->phase & phase)) {
		pr_err("Key \"%s\" may not be negotiated during ",
				param->name);
		switch (phase) {
		case PHASE_SECURITY:
			pr_debug("Security phase.\n");
			break;
		case PHASE_OPERATIONAL:
			pr_debug("Operational phase.\n");
			break;
		default:
			pr_debug("Unknown phase.\n");
		}
		return NULL;
	}

	return param;
}

static int iscsi_enforce_integrity_rules(
	u8 phase,
	struct iscsi_param_list *param_list)
{
	char *tmpptr;
	u8 DataSequenceInOrder = 0;
	u8 ErrorRecoveryLevel = 0, SessionType = 0;
	u8 IFMarker = 0, OFMarker = 0;
	u8 IFMarkInt_Reject = 1, OFMarkInt_Reject = 1;
	u32 FirstBurstLength = 0, MaxBurstLength = 0;
	struct iscsi_param *param = NULL;

	list_for_each_entry(param, &param_list->param_list, p_list) {
		if (!(param->phase & phase))
			continue;
		if (!strcmp(param->name, SESSIONTYPE))
			if (!strcmp(param->value, NORMAL))
				SessionType = 1;
		if (!strcmp(param->name, ERRORRECOVERYLEVEL))
			ErrorRecoveryLevel = simple_strtoul(param->value,
					&tmpptr, 0);
		if (!strcmp(param->name, DATASEQUENCEINORDER))
			if (!strcmp(param->value, YES))
				DataSequenceInOrder = 1;
		if (!strcmp(param->name, MAXBURSTLENGTH))
			MaxBurstLength = simple_strtoul(param->value,
					&tmpptr, 0);
		if (!strcmp(param->name, IFMARKER))
			if (!strcmp(param->value, YES))
				IFMarker = 1;
		if (!strcmp(param->name, OFMARKER))
			if (!strcmp(param->value, YES))
				OFMarker = 1;
		if (!strcmp(param->name, IFMARKINT))
			if (!strcmp(param->value, REJECT))
				IFMarkInt_Reject = 1;
		if (!strcmp(param->name, OFMARKINT))
			if (!strcmp(param->value, REJECT))
				OFMarkInt_Reject = 1;
	}

	list_for_each_entry(param, &param_list->param_list, p_list) {
		if (!(param->phase & phase))
			continue;
		if (!SessionType && (!IS_PSTATE_ACCEPTOR(param) &&
		     (strcmp(param->name, IFMARKER) &&
		      strcmp(param->name, OFMARKER) &&
		      strcmp(param->name, IFMARKINT) &&
		      strcmp(param->name, OFMARKINT))))
			continue;
		if (!strcmp(param->name, MAXOUTSTANDINGR2T) &&
		    DataSequenceInOrder && (ErrorRecoveryLevel > 0)) {
			if (strcmp(param->value, "1")) {
				if (iscsi_update_param_value(param, "1") < 0)
					return -1;
				pr_debug("Reset \"%s\" to \"%s\".\n",
					param->name, param->value);
			}
		}
		if (!strcmp(param->name, MAXCONNECTIONS) && !SessionType) {
			if (strcmp(param->value, "1")) {
				if (iscsi_update_param_value(param, "1") < 0)
					return -1;
				pr_debug("Reset \"%s\" to \"%s\".\n",
					param->name, param->value);
			}
		}
		if (!strcmp(param->name, FIRSTBURSTLENGTH)) {
			FirstBurstLength = simple_strtoul(param->value,
					&tmpptr, 0);
			if (FirstBurstLength > MaxBurstLength) {
				char tmpbuf[11];
				memset(tmpbuf, 0, sizeof(tmpbuf));
				sprintf(tmpbuf, "%u", MaxBurstLength);
				if (iscsi_update_param_value(param, tmpbuf))
					return -1;
				pr_debug("Reset \"%s\" to \"%s\".\n",
					param->name, param->value);
			}
		}
		if (!strcmp(param->name, IFMARKER) && IFMarkInt_Reject) {
			if (iscsi_update_param_value(param, NO) < 0)
				return -1;
			IFMarker = 0;
			pr_debug("Reset \"%s\" to \"%s\".\n",
					param->name, param->value);
		}
		if (!strcmp(param->name, OFMARKER) && OFMarkInt_Reject) {
			if (iscsi_update_param_value(param, NO) < 0)
				return -1;
			OFMarker = 0;
			pr_debug("Reset \"%s\" to \"%s\".\n",
					 param->name, param->value);
		}
		if (!strcmp(param->name, IFMARKINT) && !IFMarker) {
			if (!strcmp(param->value, REJECT))
				continue;
			param->state &= ~PSTATE_NEGOTIATE;
			if (iscsi_update_param_value(param, IRRELEVANT) < 0)
				return -1;
			pr_debug("Reset \"%s\" to \"%s\".\n",
					param->name, param->value);
		}
		if (!strcmp(param->name, OFMARKINT) && !OFMarker) {
			if (!strcmp(param->value, REJECT))
				continue;
			param->state &= ~PSTATE_NEGOTIATE;
			if (iscsi_update_param_value(param, IRRELEVANT) < 0)
				return -1;
			pr_debug("Reset \"%s\" to \"%s\".\n",
					param->name, param->value);
		}
	}

	return 0;
}

int iscsi_decode_text_input(
	u8 phase,
	u8 sender,
	char *textbuf,
	u32 length,
	struct iscsi_conn *conn)
{
	struct iscsi_param_list *param_list = conn->param_list;
	char *tmpbuf, *start = NULL, *end = NULL;

	tmpbuf = kzalloc(length + 1, GFP_KERNEL);
	if (!tmpbuf) {
		pr_err("Unable to allocate memory for tmpbuf.\n");
		return -1;
	}

	memcpy(tmpbuf, textbuf, length);
	tmpbuf[length] = '\0';
	start = tmpbuf;
	end = (start + length);

	while (start < end) {
		char *key, *value;
		struct iscsi_param *param;

		if (iscsi_extract_key_value(start, &key, &value) < 0) {
			kfree(tmpbuf);
			return -1;
		}

		pr_debug("Got key: %s=%s\n", key, value);

		if (phase & PHASE_SECURITY) {
			if (iscsi_check_for_auth_key(key) > 0) {
				kfree(tmpbuf);
				return 1;
			}
		}

		param = iscsi_check_key(key, phase, sender, param_list);
		if (!param) {
			if (iscsi_add_notunderstood_response(key,
					value, param_list) < 0) {
				kfree(tmpbuf);
				return -1;
			}
			start += strlen(key) + strlen(value) + 2;
			continue;
		}
		if (iscsi_check_value(param, value) < 0) {
			kfree(tmpbuf);
			return -1;
		}

		start += strlen(key) + strlen(value) + 2;

		if (IS_PSTATE_PROPOSER(param)) {
			if (iscsi_check_proposer_state(param, value) < 0) {
				kfree(tmpbuf);
				return -1;
			}
			SET_PSTATE_RESPONSE_GOT(param);
		} else {
			if (iscsi_check_acceptor_state(param, value, conn) < 0) {
				kfree(tmpbuf);
				return -1;
			}
			SET_PSTATE_ACCEPTOR(param);
		}
	}

	kfree(tmpbuf);
	return 0;
}

int iscsi_encode_text_output(
	u8 phase,
	u8 sender,
	char *textbuf,
	u32 *length,
	struct iscsi_param_list *param_list)
{
	char *output_buf = NULL;
	struct iscsi_extra_response *er;
	struct iscsi_param *param;

	output_buf = textbuf + *length;

	if (iscsi_enforce_integrity_rules(phase, param_list) < 0)
		return -1;

	list_for_each_entry(param, &param_list->param_list, p_list) {
		if (!(param->sender & sender))
			continue;
		if (IS_PSTATE_ACCEPTOR(param) &&
		    !IS_PSTATE_RESPONSE_SENT(param) &&
		    !IS_PSTATE_REPLY_OPTIONAL(param) &&
		    (param->phase & phase)) {
			*length += sprintf(output_buf, "%s=%s",
				param->name, param->value);
			*length += 1;
			output_buf = textbuf + *length;
			SET_PSTATE_RESPONSE_SENT(param);
			pr_debug("Sending key: %s=%s\n",
				param->name, param->value);
			continue;
		}
		if (IS_PSTATE_NEGOTIATE(param) &&
		    !IS_PSTATE_ACCEPTOR(param) &&
		    !IS_PSTATE_PROPOSER(param) &&
		    (param->phase & phase)) {
			*length += sprintf(output_buf, "%s=%s",
				param->name, param->value);
			*length += 1;
			output_buf = textbuf + *length;
			SET_PSTATE_PROPOSER(param);
			iscsi_check_proposer_for_optional_reply(param);
			pr_debug("Sending key: %s=%s\n",
				param->name, param->value);
		}
	}

	list_for_each_entry(er, &param_list->extra_response_list, er_list) {
		*length += sprintf(output_buf, "%s=%s", er->key, er->value);
		*length += 1;
		output_buf = textbuf + *length;
		pr_debug("Sending key: %s=%s\n", er->key, er->value);
	}
	iscsi_release_extra_responses(param_list);

	return 0;
}

int iscsi_check_negotiated_keys(struct iscsi_param_list *param_list)
{
	int ret = 0;
	struct iscsi_param *param;

	list_for_each_entry(param, &param_list->param_list, p_list) {
		if (IS_PSTATE_NEGOTIATE(param) &&
		    IS_PSTATE_PROPOSER(param) &&
		    !IS_PSTATE_RESPONSE_GOT(param) &&
		    !IS_PSTATE_REPLY_OPTIONAL(param) &&
		    !IS_PHASE_DECLARATIVE(param)) {
			pr_err("No response for proposed key \"%s\".\n",
					param->name);
			ret = -1;
		}
	}

	return ret;
}

int iscsi_change_param_value(
	char *keyvalue,
	struct iscsi_param_list *param_list,
	int check_key)
{
	char *key = NULL, *value = NULL;
	struct iscsi_param *param;
	int sender = 0;

	if (iscsi_extract_key_value(keyvalue, &key, &value) < 0)
		return -1;

	if (!check_key) {
		param = __iscsi_check_key(keyvalue, sender, param_list);
		if (!param)
			return -1;
	} else {
		param = iscsi_check_key(keyvalue, 0, sender, param_list);
		if (!param)
			return -1;

		param->set_param = 1;
		if (iscsi_check_value(param, value) < 0) {
			param->set_param = 0;
			return -1;
		}
		param->set_param = 0;
	}

	if (iscsi_update_param_value(param, value) < 0)
		return -1;

	return 0;
}

void iscsi_set_connection_parameters(
	struct iscsi_conn_ops *ops,
	struct iscsi_param_list *param_list)
{
	char *tmpptr;
	struct iscsi_param *param;

	pr_debug("---------------------------------------------------"
			"---------------\n");
	list_for_each_entry(param, &param_list->param_list, p_list) {
		/*
		 * Special case to set MAXXMITDATASEGMENTLENGTH from the
		 * target requested MaxRecvDataSegmentLength, even though
		 * this key is not sent over the wire.
		 */
		if (!strcmp(param->name, MAXXMITDATASEGMENTLENGTH)) {
			if (param_list->iser == true)
				continue;

			ops->MaxXmitDataSegmentLength =
				simple_strtoul(param->value, &tmpptr, 0);
			pr_debug("MaxXmitDataSegmentLength:     %s\n",
				param->value);
		}

		if (!IS_PSTATE_ACCEPTOR(param) && !IS_PSTATE_PROPOSER(param))
			continue;
		if (!strcmp(param->name, AUTHMETHOD)) {
			pr_debug("AuthMethod:                   %s\n",
				param->value);
		} else if (!strcmp(param->name, HEADERDIGEST)) {
			ops->HeaderDigest = !strcmp(param->value, CRC32C);
			pr_debug("HeaderDigest:                 %s\n",
				param->value);
		} else if (!strcmp(param->name, DATADIGEST)) {
			ops->DataDigest = !strcmp(param->value, CRC32C);
			pr_debug("DataDigest:                   %s\n",
				param->value);
		} else if (!strcmp(param->name, MAXRECVDATASEGMENTLENGTH)) {
			/*
			 * At this point iscsi_check_acceptor_state() will have
			 * set ops->MaxRecvDataSegmentLength from the original
			 * initiator provided value.
			 */
			pr_debug("MaxRecvDataSegmentLength:     %u\n",
				ops->MaxRecvDataSegmentLength);
		} else if (!strcmp(param->name, OFMARKER)) {
			ops->OFMarker = !strcmp(param->value, YES);
			pr_debug("OFMarker:                     %s\n",
				param->value);
		} else if (!strcmp(param->name, IFMARKER)) {
			ops->IFMarker = !strcmp(param->value, YES);
			pr_debug("IFMarker:                     %s\n",
				param->value);
		} else if (!strcmp(param->name, OFMARKINT)) {
			ops->OFMarkInt =
				simple_strtoul(param->value, &tmpptr, 0);
			pr_debug("OFMarkInt:                    %s\n",
				param->value);
		} else if (!strcmp(param->name, IFMARKINT)) {
			ops->IFMarkInt =
				simple_strtoul(param->value, &tmpptr, 0);
			pr_debug("IFMarkInt:                    %s\n",
				param->value);
		} else if (!strcmp(param->name, INITIATORRECVDATASEGMENTLENGTH)) {
			ops->InitiatorRecvDataSegmentLength =
				simple_strtoul(param->value, &tmpptr, 0);
			pr_debug("InitiatorRecvDataSegmentLength: %s\n",
				param->value);
			ops->MaxRecvDataSegmentLength =
					ops->InitiatorRecvDataSegmentLength;
			pr_debug("Set MRDSL from InitiatorRecvDataSegmentLength\n");
		} else if (!strcmp(param->name, TARGETRECVDATASEGMENTLENGTH)) {
			ops->TargetRecvDataSegmentLength =
				simple_strtoul(param->value, &tmpptr, 0);
			pr_debug("TargetRecvDataSegmentLength:  %s\n",
				param->value);
			ops->MaxXmitDataSegmentLength =
					ops->TargetRecvDataSegmentLength;
			pr_debug("Set MXDSL from TargetRecvDataSegmentLength\n");
		}
	}
	pr_debug("----------------------------------------------------"
			"--------------\n");
}

void iscsi_set_session_parameters(
	struct iscsi_sess_ops *ops,
	struct iscsi_param_list *param_list,
	int leading)
{
	char *tmpptr;
	struct iscsi_param *param;

	pr_debug("----------------------------------------------------"
			"--------------\n");
	list_for_each_entry(param, &param_list->param_list, p_list) {
		if (!IS_PSTATE_ACCEPTOR(param) && !IS_PSTATE_PROPOSER(param))
			continue;
		if (!strcmp(param->name, INITIATORNAME)) {
			if (!param->value)
				continue;
			if (leading)
				snprintf(ops->InitiatorName,
						sizeof(ops->InitiatorName),
						"%s", param->value);
			pr_debug("InitiatorName:                %s\n",
				param->value);
		} else if (!strcmp(param->name, INITIATORALIAS)) {
			if (!param->value)
				continue;
			snprintf(ops->InitiatorAlias,
						sizeof(ops->InitiatorAlias),
						"%s", param->value);
			pr_debug("InitiatorAlias:               %s\n",
				param->value);
		} else if (!strcmp(param->name, TARGETNAME)) {
			if (!param->value)
				continue;
			if (leading)
				snprintf(ops->TargetName,
						sizeof(ops->TargetName),
						"%s", param->value);
			pr_debug("TargetName:                   %s\n",
				param->value);
		} else if (!strcmp(param->name, TARGETALIAS)) {
			if (!param->value)
				continue;
			snprintf(ops->TargetAlias, sizeof(ops->TargetAlias),
					"%s", param->value);
			pr_debug("TargetAlias:                  %s\n",
				param->value);
		} else if (!strcmp(param->name, TARGETPORTALGROUPTAG)) {
			ops->TargetPortalGroupTag =
				simple_strtoul(param->value, &tmpptr, 0);
			pr_debug("TargetPortalGroupTag:         %s\n",
				param->value);
		} else if (!strcmp(param->name, MAXCONNECTIONS)) {
			ops->MaxConnections =
				simple_strtoul(param->value, &tmpptr, 0);
			pr_debug("MaxConnections:               %s\n",
				param->value);
		} else if (!strcmp(param->name, INITIALR2T)) {
			ops->InitialR2T = !strcmp(param->value, YES);
			 pr_debug("InitialR2T:                   %s\n",
				param->value);
		} else if (!strcmp(param->name, IMMEDIATEDATA)) {
			ops->ImmediateData = !strcmp(param->value, YES);
			pr_debug("ImmediateData:                %s\n",
				param->value);
		} else if (!strcmp(param->name, MAXBURSTLENGTH)) {
			ops->MaxBurstLength =
				simple_strtoul(param->value, &tmpptr, 0);
			pr_debug("MaxBurstLength:               %s\n",
				param->value);
		} else if (!strcmp(param->name, FIRSTBURSTLENGTH)) {
			ops->FirstBurstLength =
				simple_strtoul(param->value, &tmpptr, 0);
			pr_debug("FirstBurstLength:             %s\n",
				param->value);
		} else if (!strcmp(param->name, DEFAULTTIME2WAIT)) {
			ops->DefaultTime2Wait =
				simple_strtoul(param->value, &tmpptr, 0);
			pr_debug("DefaultTime2Wait:             %s\n",
				param->value);
		} else if (!strcmp(param->name, DEFAULTTIME2RETAIN)) {
			ops->DefaultTime2Retain =
				simple_strtoul(param->value, &tmpptr, 0);
			pr_debug("DefaultTime2Retain:           %s\n",
				param->value);
		} else if (!strcmp(param->name, MAXOUTSTANDINGR2T)) {
			ops->MaxOutstandingR2T =
				simple_strtoul(param->value, &tmpptr, 0);
			pr_debug("MaxOutstandingR2T:            %s\n",
				param->value);
		} else if (!strcmp(param->name, DATAPDUINORDER)) {
			ops->DataPDUInOrder = !strcmp(param->value, YES);
			pr_debug("DataPDUInOrder:               %s\n",
				param->value);
		} else if (!strcmp(param->name, DATASEQUENCEINORDER)) {
			ops->DataSequenceInOrder = !strcmp(param->value, YES);
			pr_debug("DataSequenceInOrder:          %s\n",
				param->value);
		} else if (!strcmp(param->name, ERRORRECOVERYLEVEL)) {
			ops->ErrorRecoveryLevel =
				simple_strtoul(param->value, &tmpptr, 0);
			pr_debug("ErrorRecoveryLevel:           %s\n",
				param->value);
		} else if (!strcmp(param->name, SESSIONTYPE)) {
			ops->SessionType = !strcmp(param->value, DISCOVERY);
			pr_debug("SessionType:                  %s\n",
				param->value);
		} else if (!strcmp(param->name, RDMAEXTENSIONS)) {
			ops->RDMAExtensions = !strcmp(param->value, YES);
			pr_debug("RDMAExtensions:               %s\n",
				param->value);
		}
	}
	pr_debug("----------------------------------------------------"
			"--------------\n");

}
		if (phase & PHASE_SECURITY) {
			if (iscsi_check_for_auth_key(key) > 0) {
				kfree(tmpbuf);
				return 1;
			}
		}