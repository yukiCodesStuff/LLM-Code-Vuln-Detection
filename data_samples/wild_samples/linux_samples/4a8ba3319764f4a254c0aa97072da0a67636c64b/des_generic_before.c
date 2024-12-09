	.cra_u			=	{ .cipher = {
	.cia_min_keysize	=	DES3_EDE_KEY_SIZE,
	.cia_max_keysize	=	DES3_EDE_KEY_SIZE,
	.cia_setkey		=	des3_ede_setkey,
	.cia_encrypt		=	des3_ede_encrypt,
	.cia_decrypt		=	des3_ede_decrypt } }
} };

MODULE_ALIAS_CRYPTO("des3_ede");

static int __init des_generic_mod_init(void)
{
	return crypto_register_algs(des_algs, ARRAY_SIZE(des_algs));
}
{
	crypto_unregister_algs(des_algs, ARRAY_SIZE(des_algs));
}

module_init(des_generic_mod_init);
module_exit(des_generic_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DES & Triple DES EDE Cipher Algorithms");
MODULE_AUTHOR("Dag Arne Osvik <da@osvik.no>");
MODULE_ALIAS("des");