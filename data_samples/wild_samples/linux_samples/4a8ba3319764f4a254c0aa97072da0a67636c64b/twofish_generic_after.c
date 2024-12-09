{
	crypto_unregister_alg(&alg);
}

module_init(twofish_mod_init);
module_exit(twofish_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION ("Twofish Cipher Algorithm");
MODULE_ALIAS_CRYPTO("twofish");
MODULE_ALIAS_CRYPTO("twofish-generic");