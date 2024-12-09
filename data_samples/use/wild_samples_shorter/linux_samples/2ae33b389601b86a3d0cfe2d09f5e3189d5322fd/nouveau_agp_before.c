	if (drm->agp.stat == UNKNOWN) {
		if (!nouveau_agpmode)
			return false;
		return true;
	}

	return (drm->agp.stat == ENABLED);