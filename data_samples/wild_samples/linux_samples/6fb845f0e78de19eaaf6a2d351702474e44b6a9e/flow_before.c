const struct uapi_definition mlx5_ib_flow_defs[] = {
	UAPI_DEF_CHAIN_OBJ_TREE_NAMED(
		MLX5_IB_OBJECT_FLOW_MATCHER,
		UAPI_DEF_IS_OBJ_SUPPORTED(flow_is_supported)),
	UAPI_DEF_CHAIN_OBJ_TREE(
		UVERBS_OBJECT_FLOW,
		&mlx5_ib_fs,
		UAPI_DEF_IS_OBJ_SUPPORTED(flow_is_supported)),
	UAPI_DEF_CHAIN_OBJ_TREE(UVERBS_OBJECT_FLOW_ACTION,
				&mlx5_ib_flow_actions),
	{},
};