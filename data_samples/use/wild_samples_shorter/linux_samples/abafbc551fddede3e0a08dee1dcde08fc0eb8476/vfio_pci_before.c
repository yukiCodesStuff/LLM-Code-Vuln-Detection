#include <linux/vfio.h>
#include <linux/vgaarb.h>
#include <linux/nospec.h>

#include "vfio_pci_private.h"

#define DRIVER_VERSION  "0.2"

static void vfio_pci_try_bus_reset(struct vfio_pci_device *vdev);
static void vfio_pci_disable(struct vfio_pci_device *vdev);

/*
 * INTx masking requires the ability to disable INTx signaling via PCI_COMMAND
 * _and_ the ability detect when the device is asserting INTx via PCI_STATUS.
	return 0;
}

static long vfio_pci_ioctl(void *device_data,
			   unsigned int cmd, unsigned long arg)
{
	struct vfio_pci_device *vdev = device_data;
		{
			void __iomem *io;
			size_t size;
			u16 orig_cmd;

			info.offset = VFIO_PCI_INDEX_TO_OFFSET(info.index);
			info.flags = 0;

			 * Is it really there?  Enable memory decode for
			 * implicit access in pci_map_rom().
			 */
			pci_read_config_word(pdev, PCI_COMMAND, &orig_cmd);
			pci_write_config_word(pdev, PCI_COMMAND,
					      orig_cmd | PCI_COMMAND_MEMORY);

			io = pci_map_rom(pdev, &size);
			if (io) {
				info.flags = VFIO_REGION_INFO_FLAG_READ;
				pci_unmap_rom(pdev, io);
			} else {
				info.size = 0;
			}

			pci_write_config_word(pdev, PCI_COMMAND, orig_cmd);
			break;
		}
		case VFIO_PCI_VGA_REGION_INDEX:
			if (!vdev->has_vga)
		return ret;

	} else if (cmd == VFIO_DEVICE_RESET) {
		return vdev->reset_works ?
			pci_try_reset_function(vdev->pdev) : -EINVAL;

	} else if (cmd == VFIO_DEVICE_GET_PCI_HOT_RESET_INFO) {
		struct vfio_pci_hot_reset_info hdr;
		struct vfio_pci_fill_info fill = { 0 };
		int32_t *group_fds;
		struct vfio_pci_group_entry *groups;
		struct vfio_pci_group_info info;
		bool slot = false;
		int i, count = 0, ret = 0;

		minsz = offsetofend(struct vfio_pci_hot_reset, count);

		if (copy_from_user(&hdr, (void __user *)arg, minsz))
		 * user interface and store the group and iommu ID.  This
		 * ensures the group is held across the reset.
		 */
		for (i = 0; i < hdr.count; i++) {
			struct vfio_group *group;
			struct fd f = fdget(group_fds[i]);
			if (!f.file) {
				ret = -EBADF;
				break;
			}
				break;
			}

			groups[i].group = group;
			groups[i].id = vfio_external_user_iommu_id(group);
		}

		kfree(group_fds);

		ret = vfio_pci_for_each_slot_or_bus(vdev->pdev,
						    vfio_pci_validate_devs,
						    &info, slot);
		if (!ret)
			/* User has access, do the reset */
			ret = pci_reset_bus(vdev->pdev);

hot_reset_release:
		for (i--; i >= 0; i--)
			vfio_group_put_external_user(groups[i].group);

		kfree(groups);
		return ret;
	} else if (cmd == VFIO_DEVICE_IOEVENTFD) {
	return vfio_pci_rw(device_data, (char __user *)buf, count, ppos, true);
}

static int vfio_pci_add_vma(struct vfio_pci_device *vdev,
			    struct vm_area_struct *vma)
{
	struct vfio_pci_mmap_vma *mmap_vma;

	mmap_vma = kmalloc(sizeof(*mmap_vma), GFP_KERNEL);
		return -ENOMEM;

	mmap_vma->vma = vma;

	mutex_lock(&vdev->vma_lock);
	list_add(&mmap_vma->vma_next, &vdev->vma_list);
	mutex_unlock(&vdev->vma_lock);

	return 0;
}

{
	struct vm_area_struct *vma = vmf->vma;
	struct vfio_pci_device *vdev = vma->vm_private_data;

	if (vfio_pci_add_vma(vdev, vma))
		return VM_FAULT_OOM;

	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			    vma->vm_end - vma->vm_start, vma->vm_page_prot))
		return VM_FAULT_SIGBUS;

	return VM_FAULT_NOPAGE;
}

static const struct vm_operations_struct vfio_pci_mmap_ops = {
	.open = vfio_pci_mmap_open,
	INIT_LIST_HEAD(&vdev->ioeventfds_list);
	mutex_init(&vdev->vma_lock);
	INIT_LIST_HEAD(&vdev->vma_list);

	ret = vfio_add_group_dev(&pdev->dev, &vfio_pci_ops, vdev);
	if (ret)
		goto out_free;
	kref_put_mutex(&reflck->kref, vfio_pci_reflck_release, &reflck_lock);
}

struct vfio_devices {
	struct vfio_device **devices;
	int cur_index;
	int max_index;
};

static int vfio_pci_get_unused_devs(struct pci_dev *pdev, void *data)
{
	struct vfio_devices *devs = data;
	struct vfio_device *device;
	return 0;
}

/*
 * If a bus or slot reset is available for the provided device and:
 *  - All of the devices affected by that bus or slot reset are unused
 *    (!refcnt)