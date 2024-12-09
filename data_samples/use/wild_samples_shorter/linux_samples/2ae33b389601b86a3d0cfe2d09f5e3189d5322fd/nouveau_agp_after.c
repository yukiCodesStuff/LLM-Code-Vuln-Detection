	if (drm->agp.stat == UNKNOWN) {
		if (!nouveau_agpmode)
			return false;
#ifdef __powerpc__
		/* Disable AGP by default on all PowerPC machines for
		 * now -- At least some UniNorth-2 AGP bridges are
		 * known to be broken: DMA from the host to the card
		 * works just fine, but writeback from the card to the
		 * host goes straight to memory untranslated bypassing
		 * the GATT somehow, making them quite painful to deal
		 * with...
		 */
		if (nouveau_agpmode == -1)
			return false;
#endif
		return true;
	}

	return (drm->agp.stat == ENABLED);