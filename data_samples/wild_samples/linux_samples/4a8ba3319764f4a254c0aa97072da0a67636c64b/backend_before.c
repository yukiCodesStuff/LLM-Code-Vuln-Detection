{
	if (!strcmp(s,"off"))
		agp_off = 1;
	if (!strcmp(s,"try_unsupported"))
		agp_try_unsupported_boot = 1;
	return 1;
}
__setup("agp=", agp_setup);
#endif

MODULE_AUTHOR("Dave Jones <davej@redhat.com>");
MODULE_DESCRIPTION("AGP GART driver");
MODULE_LICENSE("GPL and additional rights");
MODULE_ALIAS_MISCDEV(AGPGART_MINOR);

module_init(agp_init);
module_exit(agp_exit);
