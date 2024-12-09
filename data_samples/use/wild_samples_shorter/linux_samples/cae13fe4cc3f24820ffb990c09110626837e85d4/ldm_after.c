
	list_add_tail (&f->list, frags);
found:
	if (rec >= f->num) {
		ldm_error("REC value (%d) exceeds NUM value (%d)", rec, f->num);
		return false;
	}

	if (f->map & (1 << rec)) {
		ldm_error ("Duplicate VBLK, part %d.", rec);
		f->map &= 0x7F;			/* Mark the group as broken */
		return false;