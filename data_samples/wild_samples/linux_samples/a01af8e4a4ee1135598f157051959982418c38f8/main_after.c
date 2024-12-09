	} else {
		rcu_read_unlock();
		err = carl9170_mod_virtual_mac(ar, vif_id, vif->addr);

		if (err)
			goto unlock;
	}

unlock:
	if (err && (vif_id >= 0)) {
		vif_priv->active = false;
		bitmap_release_region(&ar->vif_bitmap, vif_id, 0);
		ar->vifs--;
		rcu_assign_pointer(ar->vif_priv[vif_id].vif, NULL);
		list_del_rcu(&vif_priv->list);
		mutex_unlock(&ar->mutex);
		synchronize_rcu();
	} else {
		if (ar->vifs > 1)
			ar->ps.off_override |= PS_OFF_VIF;

		mutex_unlock(&ar->mutex);
	}

	return err;
}