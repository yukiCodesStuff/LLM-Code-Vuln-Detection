__setup("agp=", agp_setup);
#endif

MODULE_AUTHOR("Dave Jones, Jeff Hartmann");
MODULE_DESCRIPTION("AGP GART driver");
MODULE_LICENSE("GPL and additional rights");
MODULE_ALIAS_MISCDEV(AGPGART_MINOR);
