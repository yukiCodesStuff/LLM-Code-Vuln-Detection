{
	crypto_unregister_algs(tea_algs, ARRAY_SIZE(tea_algs));
}

MODULE_ALIAS_CRYPTO("tea");
MODULE_ALIAS_CRYPTO("xtea");
MODULE_ALIAS_CRYPTO("xeta");

module_init(tea_mod_init);
module_exit(tea_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TEA, XTEA & XETA Cryptographic Algorithms");