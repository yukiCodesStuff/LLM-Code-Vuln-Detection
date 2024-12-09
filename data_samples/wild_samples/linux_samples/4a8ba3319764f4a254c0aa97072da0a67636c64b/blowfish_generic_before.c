{
	crypto_unregister_alg(&alg);
}

module_init(blowfish_mod_init);
module_exit(blowfish_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Blowfish Cipher Algorithm");
MODULE_ALIAS_CRYPTO("blowfish");