	}

unlock:
	if (err && (vif_id != -1)) {
		vif_priv->active = false;
		bitmap_release_region(&ar->vif_bitmap, vif_id, 0);
		ar->vifs--;
		rcu_assign_pointer(ar->vif_priv[vif_id].vif, NULL);