{
	crypto_unregister_alg(&alg);
}

module_init(cast5_mod_init);
module_exit(cast5_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Cast5 Cipher Algorithm");
MODULE_ALIAS_CRYPTO("cast5");