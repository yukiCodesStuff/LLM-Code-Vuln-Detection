{
#ifndef MODULE
	if (gart_iommu_aperture)
		return;
#endif
	if (aperture_resource)
		release_resource(aperture_resource);
	pci_unregister_driver(&agp_amd64_pci_driver);
}

module_init(agp_amd64_mod_init);
module_exit(agp_amd64_cleanup);

MODULE_AUTHOR("Dave Jones, Andi Kleen");
module_param(agp_try_unsupported, bool, 0);
MODULE_LICENSE("GPL");