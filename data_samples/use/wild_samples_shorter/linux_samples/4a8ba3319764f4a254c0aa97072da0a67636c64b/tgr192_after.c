	crypto_unregister_shashes(tgr_algs, ARRAY_SIZE(tgr_algs));
}

MODULE_ALIAS_CRYPTO("tgr192");
MODULE_ALIAS_CRYPTO("tgr160");
MODULE_ALIAS_CRYPTO("tgr128");

module_init(tgr192_mod_init);