{
	crypto_unregister_alg(&alg);
}

module_init(salsa20_generic_mod_init);
module_exit(salsa20_generic_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION ("Salsa20 stream cipher algorithm");
MODULE_ALIAS_CRYPTO("salsa20");