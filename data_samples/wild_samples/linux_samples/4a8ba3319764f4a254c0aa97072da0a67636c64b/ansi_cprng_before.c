{
	crypto_unregister_algs(rng_algs, ARRAY_SIZE(rng_algs));
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Software Pseudo Random Number Generator");
MODULE_AUTHOR("Neil Horman <nhorman@tuxdriver.com>");
module_param(dbg, int, 0);
MODULE_PARM_DESC(dbg, "Boolean to enable debugging (0/1 == off/on)");
module_init(prng_mod_init);
module_exit(prng_mod_fini);
MODULE_ALIAS_CRYPTO("stdrng");