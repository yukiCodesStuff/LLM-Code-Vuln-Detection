{
	pci_unregister_driver(&agp_ati_pci_driver);
}

module_init(agp_ati_init);
module_exit(agp_ati_cleanup);

MODULE_AUTHOR("Dave Jones");
MODULE_LICENSE("GPL and additional rights");
