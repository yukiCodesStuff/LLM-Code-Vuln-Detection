		struct drm_connector_state *conn_state;
		struct vmw_connector_state *vmw_conn_state;

		if (!du->pref_active) {
			ret = -EINVAL;
			goto clean;
		}

				      user_fence_rep)
{
	struct vmw_fence_obj *fence = NULL;
	uint32_t handle;
	int ret;

	if (file_priv || user_fence_rep || vmw_validation_has_bos(ctx) ||
	    out_fence)
		ret = vmw_execbuf_fence_commands(file_priv, dev_priv, &fence,