{
	if (--intel_private.refcount)
		return;

	if (intel_private.pcidev)
		pci_dev_put(intel_private.pcidev);
	if (intel_private.bridge_dev)
		pci_dev_put(intel_private.bridge_dev);
	intel_private.driver = NULL;
}
EXPORT_SYMBOL(intel_gmch_remove);

MODULE_AUTHOR("Dave Jones <davej@redhat.com>");
MODULE_LICENSE("GPL and additional rights");