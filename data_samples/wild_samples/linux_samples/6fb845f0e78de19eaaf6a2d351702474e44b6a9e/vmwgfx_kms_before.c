				      new_crtc_state, i) {
		struct vmw_display_unit *du = vmw_crtc_to_du(crtc);
		struct drm_connector *connector;
		struct drm_connector_state *conn_state;
		struct vmw_connector_state *vmw_conn_state;

		if (!du->pref_active) {
			ret = -EINVAL;
			goto clean;
		}
{
	struct vmw_fence_obj *fence = NULL;
	uint32_t handle;
	int ret;

	if (file_priv || user_fence_rep || vmw_validation_has_bos(ctx) ||
	    out_fence)
		ret = vmw_execbuf_fence_commands(file_priv, dev_priv, &fence,
						 file_priv ? &handle : NULL);
	vmw_validation_done(ctx, fence);
	if (file_priv)
		vmw_execbuf_copy_fence_user(dev_priv, vmw_fpriv(file_priv),
					    ret, user_fence_rep, fence,
					    handle, -1, NULL);
	if (out_fence)
		*out_fence = fence;
	else
		vmw_fence_obj_unreference(&fence);
}