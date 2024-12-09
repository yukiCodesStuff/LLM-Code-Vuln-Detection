		rbuf[15] = lowest_aligned;

		if (ata_id_has_trim(args->id)) {
			rbuf[14] |= 0x80; /* LBPME */

			if (ata_id_has_zero_after_trim(args->id) &&
			    dev->horkage & ATA_HORKAGE_ZERO_AFTER_TRIM) {
				ata_dev_info(dev, "Enabling discard_zeroes_data\n");
				rbuf[14] |= 0x40; /* LBPRZ */
			}
		}
	}
	return 0;
}

/**