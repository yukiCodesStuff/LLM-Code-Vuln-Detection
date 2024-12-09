{
	crypto_unregister_shashes(sha256_algs, ARRAY_SIZE(sha256_algs));
}

module_init(sha256_generic_mod_init);
module_exit(sha256_generic_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SHA-224 and SHA-256 Secure Hash Algorithm");

MODULE_ALIAS_CRYPTO("sha224");
MODULE_ALIAS_CRYPTO("sha256");