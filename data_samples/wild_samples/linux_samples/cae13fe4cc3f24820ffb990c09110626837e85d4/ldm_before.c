	if (!f) {
		ldm_crit ("Out of memory.");
		return false;
	}

	f->group = group;
	f->num   = num;
	f->rec   = rec;
	f->map   = 0xFF << num;

	list_add_tail (&f->list, frags);
found:
	if (f->map & (1 << rec)) {
		ldm_error ("Duplicate VBLK, part %d.", rec);
		f->map &= 0x7F;			/* Mark the group as broken */
		return false;
	}