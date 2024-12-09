		MLX5_SET_TO_ONES(fte_match_set_misc, misc,
				 source_eswitch_owner_vhca_id);

	if (attr->match_level == MLX5_MATCH_NONE)
		spec->match_criteria_enable = MLX5_MATCH_MISC_PARAMETERS;
	else
		spec->match_criteria_enable = MLX5_MATCH_OUTER_HEADERS |
					      MLX5_MATCH_MISC_PARAMETERS;

	if (flow_act.action & MLX5_FLOW_CONTEXT_ACTION_DECAP)
		spec->match_criteria_enable |= MLX5_MATCH_INNER_HEADERS;

	if (flow_act.action & MLX5_FLOW_CONTEXT_ACTION_MOD_HDR)
		flow_act.modify_id = attr->mod_hdr_id;
