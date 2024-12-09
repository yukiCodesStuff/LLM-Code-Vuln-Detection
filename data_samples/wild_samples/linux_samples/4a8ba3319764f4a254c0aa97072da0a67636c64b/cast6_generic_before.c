{
	crypto_unregister_alg(&alg);
}

module_init(cast6_mod_init);
module_exit(cast6_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Cast6 Cipher Algorithm");
MODULE_ALIAS_CRYPTO("cast6");