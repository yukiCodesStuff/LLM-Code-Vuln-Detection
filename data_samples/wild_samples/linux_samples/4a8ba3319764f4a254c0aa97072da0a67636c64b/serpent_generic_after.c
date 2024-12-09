{
	crypto_unregister_algs(srp_algs, ARRAY_SIZE(srp_algs));
}

module_init(serpent_mod_init);
module_exit(serpent_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Serpent and tnepres (kerneli compatible serpent reversed) Cipher Algorithm");
MODULE_AUTHOR("Dag Arne Osvik <osvik@ii.uib.no>");
MODULE_ALIAS_CRYPTO("tnepres");
MODULE_ALIAS_CRYPTO("serpent");
MODULE_ALIAS_CRYPTO("serpent-generic");