	if (index_key->type == &key_type_keyring)
		up_write(&keyring_serialise_link_sem);

	if (edit) {
		if (!edit->dead_leaf) {
			key_payload_reserve(keyring,
				keyring->datalen - KEYQUOTA_LINK_BYTES);
		}
		assoc_array_cancel_edit(edit);
	}
	up_write(&keyring->sem);
}