	struct notifier_block	nb;
	struct mutex		vma_lock;
	struct list_head	vma_list;
	struct rw_semaphore	memory_lock;
};

#define is_intx(vdev) (vdev->irq_type == VFIO_PCI_INTX_IRQ_INDEX)
#define is_msi(vdev) (vdev->irq_type == VFIO_PCI_MSI_IRQ_INDEX)
extern int vfio_pci_set_power_state(struct vfio_pci_device *vdev,
				    pci_power_t state);

extern bool __vfio_pci_memory_enabled(struct vfio_pci_device *vdev);
extern void vfio_pci_zap_and_down_write_memory_lock(struct vfio_pci_device
						    *vdev);
extern u16 vfio_pci_memory_lock_and_enable(struct vfio_pci_device *vdev);
extern void vfio_pci_memory_unlock_and_restore(struct vfio_pci_device *vdev,
					       u16 cmd);

#ifdef CONFIG_VFIO_PCI_IGD
extern int vfio_pci_igd_init(struct vfio_pci_device *vdev);
#else
static inline int vfio_pci_igd_init(struct vfio_pci_device *vdev)