
	/* No failures allowed past this point. */

	if (!s->not_found && here->e_value_size && here->e_value_offs) {
		/* Remove the old value. */
		void *first_val = s->base + min_offs;
		size_t offs = le16_to_cpu(here->e_value_offs);
		void *val = s->base + offs;