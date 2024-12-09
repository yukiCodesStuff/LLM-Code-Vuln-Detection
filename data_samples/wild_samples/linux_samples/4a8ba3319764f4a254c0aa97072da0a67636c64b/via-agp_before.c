{
	pci_unregister_driver(&agp_via_pci_driver);
}

module_init(agp_via_init);
module_exit(agp_via_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dave Jones <davej@redhat.com>");