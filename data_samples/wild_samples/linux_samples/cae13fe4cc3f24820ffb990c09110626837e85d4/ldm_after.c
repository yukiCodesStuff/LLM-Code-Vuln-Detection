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
	if (rec >= f->num) {
		ldm_error("REC value (%d) exceeds NUM value (%d)", rec, f->num);
		return false;
	}