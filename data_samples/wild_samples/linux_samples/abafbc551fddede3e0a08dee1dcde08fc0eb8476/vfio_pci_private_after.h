struct vfio_pci_device {
	struct pci_dev		*pdev;
	void __iomem		*barmap[PCI_STD_NUM_BARS];
	bool			bar_mmap_supported[PCI_STD_NUM_BARS];
	u8			*pci_config_map;
	u8			*vconfig;
	struct perm_bits	*msi_perm;
	spinlock_t		irqlock;
	struct mutex		igate;
	struct vfio_pci_irq_ctx	*ctx;
	int			num_ctx;
	int			irq_type;
	int			num_regions;
	struct vfio_pci_region	*region;
	u8			msi_qmax;
	u8			msix_bar;
	u16			msix_size;
	u32			msix_offset;
	u32			rbar[7];
	bool			pci_2_3;
	bool			virq_disabled;
	bool			reset_works;
	bool			extended_caps;
	bool			bardirty;
	bool			has_vga;
	bool			needs_reset;
	bool			nointx;
	bool			needs_pm_restore;
	struct pci_saved_state	*pci_saved_state;
	struct pci_saved_state	*pm_save;
	struct vfio_pci_reflck	*reflck;
	int			refcnt;
	int			ioeventfds_nr;
	struct eventfd_ctx	*err_trigger;
	struct eventfd_ctx	*req_trigger;
	struct list_head	dummy_resources_list;
	struct mutex		ioeventfds_lock;
	struct list_head	ioeventfds_list;
	struct vfio_pci_vf_token	*vf_token;
	struct notifier_block	nb;
	struct mutex		vma_lock;
	struct list_head	vma_list;
	struct rw_semaphore	memory_lock;
};

#define is_intx(vdev) (vdev->irq_type == VFIO_PCI_INTX_IRQ_INDEX)
#define is_msi(vdev) (vdev->irq_type == VFIO_PCI_MSI_IRQ_INDEX)
#define is_msix(vdev) (vdev->irq_type == VFIO_PCI_MSIX_IRQ_INDEX)
#define is_irq_none(vdev) (!(is_intx(vdev) || is_msi(vdev) || is_msix(vdev)))
#define irq_is(vdev, type) (vdev->irq_type == type)

extern void vfio_pci_intx_mask(struct vfio_pci_device *vdev);
extern void vfio_pci_intx_unmask(struct vfio_pci_device *vdev);

extern int vfio_pci_set_irqs_ioctl(struct vfio_pci_device *vdev,
				   uint32_t flags, unsigned index,
				   unsigned start, unsigned count, void *data);

extern ssize_t vfio_pci_config_rw(struct vfio_pci_device *vdev,
				  char __user *buf, size_t count,
				  loff_t *ppos, bool iswrite);

extern ssize_t vfio_pci_bar_rw(struct vfio_pci_device *vdev, char __user *buf,
			       size_t count, loff_t *ppos, bool iswrite);

extern ssize_t vfio_pci_vga_rw(struct vfio_pci_device *vdev, char __user *buf,
			       size_t count, loff_t *ppos, bool iswrite);

extern long vfio_pci_ioeventfd(struct vfio_pci_device *vdev, loff_t offset,
			       uint64_t data, int count, int fd);

extern int vfio_pci_init_perm_bits(void);
extern void vfio_pci_uninit_perm_bits(void);

extern int vfio_config_init(struct vfio_pci_device *vdev);
extern void vfio_config_free(struct vfio_pci_device *vdev);

extern int vfio_pci_register_dev_region(struct vfio_pci_device *vdev,
					unsigned int type, unsigned int subtype,
					const struct vfio_pci_regops *ops,
					size_t size, u32 flags, void *data);

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
{
	return -ENODEV;
}
struct vfio_pci_device {
	struct pci_dev		*pdev;
	void __iomem		*barmap[PCI_STD_NUM_BARS];
	bool			bar_mmap_supported[PCI_STD_NUM_BARS];
	u8			*pci_config_map;
	u8			*vconfig;
	struct perm_bits	*msi_perm;
	spinlock_t		irqlock;
	struct mutex		igate;
	struct vfio_pci_irq_ctx	*ctx;
	int			num_ctx;
	int			irq_type;
	int			num_regions;
	struct vfio_pci_region	*region;
	u8			msi_qmax;
	u8			msix_bar;
	u16			msix_size;
	u32			msix_offset;
	u32			rbar[7];
	bool			pci_2_3;
	bool			virq_disabled;
	bool			reset_works;
	bool			extended_caps;
	bool			bardirty;
	bool			has_vga;
	bool			needs_reset;
	bool			nointx;
	bool			needs_pm_restore;
	struct pci_saved_state	*pci_saved_state;
	struct pci_saved_state	*pm_save;
	struct vfio_pci_reflck	*reflck;
	int			refcnt;
	int			ioeventfds_nr;
	struct eventfd_ctx	*err_trigger;
	struct eventfd_ctx	*req_trigger;
	struct list_head	dummy_resources_list;
	struct mutex		ioeventfds_lock;
	struct list_head	ioeventfds_list;
	struct vfio_pci_vf_token	*vf_token;
	struct notifier_block	nb;
	struct mutex		vma_lock;
	struct list_head	vma_list;
	struct rw_semaphore	memory_lock;
};

#define is_intx(vdev) (vdev->irq_type == VFIO_PCI_INTX_IRQ_INDEX)
#define is_msi(vdev) (vdev->irq_type == VFIO_PCI_MSI_IRQ_INDEX)
#define is_msix(vdev) (vdev->irq_type == VFIO_PCI_MSIX_IRQ_INDEX)
#define is_irq_none(vdev) (!(is_intx(vdev) || is_msi(vdev) || is_msix(vdev)))
#define irq_is(vdev, type) (vdev->irq_type == type)

extern void vfio_pci_intx_mask(struct vfio_pci_device *vdev);
extern void vfio_pci_intx_unmask(struct vfio_pci_device *vdev);

extern int vfio_pci_set_irqs_ioctl(struct vfio_pci_device *vdev,
				   uint32_t flags, unsigned index,
				   unsigned start, unsigned count, void *data);

extern ssize_t vfio_pci_config_rw(struct vfio_pci_device *vdev,
				  char __user *buf, size_t count,
				  loff_t *ppos, bool iswrite);

extern ssize_t vfio_pci_bar_rw(struct vfio_pci_device *vdev, char __user *buf,
			       size_t count, loff_t *ppos, bool iswrite);

extern ssize_t vfio_pci_vga_rw(struct vfio_pci_device *vdev, char __user *buf,
			       size_t count, loff_t *ppos, bool iswrite);

extern long vfio_pci_ioeventfd(struct vfio_pci_device *vdev, loff_t offset,
			       uint64_t data, int count, int fd);

extern int vfio_pci_init_perm_bits(void);
extern void vfio_pci_uninit_perm_bits(void);

extern int vfio_config_init(struct vfio_pci_device *vdev);
extern void vfio_config_free(struct vfio_pci_device *vdev);

extern int vfio_pci_register_dev_region(struct vfio_pci_device *vdev,
					unsigned int type, unsigned int subtype,
					const struct vfio_pci_regops *ops,
					size_t size, u32 flags, void *data);

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
{
	return -ENODEV;
}