	crypto_unregister_algs(tea_algs, ARRAY_SIZE(tea_algs));
}

MODULE_ALIAS_CRYPTO("xtea");
MODULE_ALIAS_CRYPTO("xeta");

module_init(tea_mod_init);