{
	crypto_unregister_alg(&camellia_alg);
}

module_init(camellia_init);
module_exit(camellia_fini);

MODULE_DESCRIPTION("Camellia Cipher Algorithm");
MODULE_LICENSE("GPL");
MODULE_ALIAS_CRYPTO("camellia");
MODULE_ALIAS_CRYPTO("camellia-generic");