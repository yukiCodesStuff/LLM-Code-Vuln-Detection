{
	crypto_unregister_alg(&krng_alg);
	return;
}

module_init(krng_mod_init);
module_exit(krng_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel Random Number Generator");
MODULE_ALIAS_CRYPTO("stdrng");
MODULE_ALIAS_CRYPTO("krng");