			     TPM_DIGEST_SIZE);

	/* sensitive */
	tpm_buf_append_u16(&buf, 4 + TPM_DIGEST_SIZE + payload->key_len);

	tpm_buf_append_u16(&buf, TPM_DIGEST_SIZE);
	tpm_buf_append(&buf, options->blobauth, TPM_DIGEST_SIZE);
	tpm_buf_append_u16(&buf, payload->key_len);
	tpm_buf_append(&buf, payload->key, payload->key_len);

	/* public */
	tpm_buf_append_u16(&buf, 14);

		       u32 blob_handle)
{
	struct tpm_buf buf;
	int rc;

	rc = tpm_buf_init(&buf, TPM2_ST_SESSIONS, TPM2_CC_UNSEAL);
	if (rc)
		rc = -EPERM;

	if (!rc) {
		payload->key_len = be16_to_cpup(
			(__be16 *) &buf.data[TPM_HEADER_SIZE + 4]);

		memcpy(payload->key, &buf.data[TPM_HEADER_SIZE + 6],
		       payload->key_len);
	}

	tpm_buf_destroy(&buf);
	return rc;