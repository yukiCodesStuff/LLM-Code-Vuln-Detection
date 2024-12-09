{
	pci_unregister_driver(&agp_ali_pci_driver);
}

module_init(agp_ali_init);
module_exit(agp_ali_cleanup);

MODULE_AUTHOR("Dave Jones");
MODULE_LICENSE("GPL and additional rights");
