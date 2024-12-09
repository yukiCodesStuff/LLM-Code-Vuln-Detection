{
	crypto_unregister_shash(&alg);
}

module_init(crct10dif_mod_init);
module_exit(crct10dif_mod_fini);

MODULE_AUTHOR("Tim Chen <tim.c.chen@linux.intel.com>");
MODULE_DESCRIPTION("T10 DIF CRC calculation.");
MODULE_LICENSE("GPL");
MODULE_ALIAS_CRYPTO("crct10dif");