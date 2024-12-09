{
	crypto_unregister_shashes(sha512_algs, ARRAY_SIZE(sha512_algs));
}

module_init(sha512_generic_mod_init);
module_exit(sha512_generic_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SHA-512 and SHA-384 Secure Hash Algorithms");

MODULE_ALIAS_CRYPTO("sha384");
MODULE_ALIAS_CRYPTO("sha512");