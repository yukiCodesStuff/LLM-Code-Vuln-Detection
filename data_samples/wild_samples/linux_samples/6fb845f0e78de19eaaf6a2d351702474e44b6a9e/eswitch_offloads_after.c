	if (flow_act.action & MLX5_FLOW_CONTEXT_ACTION_COUNT) {
		dest[i].type = MLX5_FLOW_DESTINATION_TYPE_COUNTER;
		dest[i].counter_id = mlx5_fc_id(attr->counter);
		i++;
	}

	misc = MLX5_ADDR_OF(fte_match_param, spec->match_value, misc_parameters);
	MLX5_SET(fte_match_set_misc, misc, source_port, attr->in_rep->vport);

	if (MLX5_CAP_ESW(esw->dev, merged_eswitch))
		MLX5_SET(fte_match_set_misc, misc,
			 source_eswitch_owner_vhca_id,
			 MLX5_CAP_GEN(attr->in_mdev, vhca_id));

	misc = MLX5_ADDR_OF(fte_match_param, spec->match_criteria, misc_parameters);
	MLX5_SET_TO_ONES(fte_match_set_misc, misc, source_port);
	if (MLX5_CAP_ESW(esw->dev, merged_eswitch))
		MLX5_SET_TO_ONES(fte_match_set_misc, misc,
				 source_eswitch_owner_vhca_id);

	spec->match_criteria_enable = MLX5_MATCH_MISC_PARAMETERS;
	if (flow_act.action & MLX5_FLOW_CONTEXT_ACTION_DECAP) {
		if (attr->tunnel_match_level != MLX5_MATCH_NONE)
			spec->match_criteria_enable |= MLX5_MATCH_OUTER_HEADERS;
		if (attr->match_level != MLX5_MATCH_NONE)
			spec->match_criteria_enable |= MLX5_MATCH_INNER_HEADERS;
	} else if (attr->match_level != MLX5_MATCH_NONE) {
		spec->match_criteria_enable |= MLX5_MATCH_OUTER_HEADERS;
	}

	if (flow_act.action & MLX5_FLOW_CONTEXT_ACTION_MOD_HDR)
		flow_act.modify_id = attr->mod_hdr_id;

	fdb = esw_get_prio_table(esw, attr->chain, attr->prio, !!split);
	if (IS_ERR(fdb)) {
		rule = ERR_CAST(fdb);
		goto err_esw_get;
	}

	rule = mlx5_add_flow_rules(fdb, spec, &flow_act, dest, i);
	if (IS_ERR(rule))
		goto err_add_rule;
	else
		esw->offloads.num_flows++;

	return rule;

err_add_rule:
	esw_put_prio_table(esw, attr->chain, attr->prio, !!split);
err_esw_get:
	if (attr->dest_chain)
		esw_put_prio_table(esw, attr->dest_chain, 1, 0);
err_create_goto_table:
	return rule;
}