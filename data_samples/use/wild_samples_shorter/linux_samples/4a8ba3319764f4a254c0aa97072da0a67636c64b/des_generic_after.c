	.cia_decrypt		=	des3_ede_decrypt } }
} };

static int __init des_generic_mod_init(void)
{
	return crypto_register_algs(des_algs, ARRAY_SIZE(des_algs));
}
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DES & Triple DES EDE Cipher Algorithms");
MODULE_AUTHOR("Dag Arne Osvik <da@osvik.no>");
MODULE_ALIAS_CRYPTO("des");
MODULE_ALIAS_CRYPTO("des-generic");
MODULE_ALIAS_CRYPTO("des3_ede");
MODULE_ALIAS_CRYPTO("des3_ede-generic");