	crypto_unregister_shashes(wp_algs, ARRAY_SIZE(wp_algs));
}

MODULE_ALIAS_CRYPTO("wp384");
MODULE_ALIAS_CRYPTO("wp256");

module_init(wp512_mod_init);