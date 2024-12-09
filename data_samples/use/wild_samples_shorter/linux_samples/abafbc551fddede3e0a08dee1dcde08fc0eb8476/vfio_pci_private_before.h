	struct notifier_block	nb;
	struct mutex		vma_lock;
	struct list_head	vma_list;
};

#define is_intx(vdev) (vdev->irq_type == VFIO_PCI_INTX_IRQ_INDEX)
#define is_msi(vdev) (vdev->irq_type == VFIO_PCI_MSI_IRQ_INDEX)
extern int vfio_pci_set_power_state(struct vfio_pci_device *vdev,
				    pci_power_t state);

#ifdef CONFIG_VFIO_PCI_IGD
extern int vfio_pci_igd_init(struct vfio_pci_device *vdev);
#else
static inline int vfio_pci_igd_init(struct vfio_pci_device *vdev)