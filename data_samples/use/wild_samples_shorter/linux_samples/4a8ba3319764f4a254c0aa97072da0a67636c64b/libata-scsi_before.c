		rbuf[15] = lowest_aligned;

		if (ata_id_has_trim(args->id)) {
			rbuf[14] |= 0x80; /* TPE */

			if (ata_id_has_zero_after_trim(args->id))
				rbuf[14] |= 0x40; /* TPRZ */
		}
	}

	return 0;
}

/**