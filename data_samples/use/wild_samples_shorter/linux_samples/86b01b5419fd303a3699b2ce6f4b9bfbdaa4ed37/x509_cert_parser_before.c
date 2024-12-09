	return cert;

error_decode:
	kfree(cert->pub->key);
	kfree(ctx);
error_no_ctx:
	x509_free_certificate(cert);
error_no_cert: