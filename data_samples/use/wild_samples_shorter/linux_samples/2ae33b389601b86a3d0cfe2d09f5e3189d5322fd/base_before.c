	/* drop port's i2c subdev refcount, i2c handles this itself */
	if (ret == 0) {
		list_add_tail(&port->head, &i2c->ports);
		atomic_dec(&engine->refcount);
	}

	return ret;