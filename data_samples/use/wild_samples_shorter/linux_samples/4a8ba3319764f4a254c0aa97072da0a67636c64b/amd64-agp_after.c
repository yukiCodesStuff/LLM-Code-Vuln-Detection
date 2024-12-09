module_init(agp_amd64_mod_init);
module_exit(agp_amd64_cleanup);

MODULE_AUTHOR("Dave Jones, Andi Kleen");
module_param(agp_try_unsupported, bool, 0);
MODULE_LICENSE("GPL");