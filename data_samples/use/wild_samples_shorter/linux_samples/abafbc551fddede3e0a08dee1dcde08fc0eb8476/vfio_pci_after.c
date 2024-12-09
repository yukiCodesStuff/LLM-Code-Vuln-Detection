#include <linux/vfio.h>
#include <linux/vgaarb.h>
#include <linux/nospec.h>
#include <linux/sched/mm.h>

#include "vfio_pci_private.h"

#define DRIVER_VERSION  "0.2"

static void vfio_pci_try_bus_reset(struct vfio_pci_device *vdev);
static void vfio_pci_disable(struct vfio_pci_device *vdev);
static int vfio_pci_try_zap_and_vma_lock_cb(struct pci_dev *pdev, void *data);

/*
 * INTx masking requires the ability to disable INTx signaling via PCI_COMMAND
 * _and_ the ability detect when the device is asserting INTx via PCI_STATUS.
	return 0;
}

struct vfio_devices {
	struct vfio_device **devices;
	int cur_index;
	int max_index;
};

static long vfio_pci_ioctl(void *device_data,
			   unsigned int cmd, unsigned long arg)
{
	struct vfio_pci_device *vdev = device_data;
		{
			void __iomem *io;
			size_t size;
			u16 cmd;

			info.offset = VFIO_PCI_INDEX_TO_OFFSET(info.index);
			info.flags = 0;

			 * Is it really there?  Enable memory decode for
			 * implicit access in pci_map_rom().
			 */
			cmd = vfio_pci_memory_lock_and_enable(vdev);
			io = pci_map_rom(pdev, &size);
			if (io) {
				info.flags = VFIO_REGION_INFO_FLAG_READ;
				pci_unmap_rom(pdev, io);
			} else {
				info.size = 0;
			}
			vfio_pci_memory_unlock_and_restore(vdev, cmd);

			break;
		}
		case VFIO_PCI_VGA_REGION_INDEX:
			if (!vdev->has_vga)
		return ret;

	} else if (cmd == VFIO_DEVICE_RESET) {
		int ret;

		if (!vdev->reset_works)
			return -EINVAL;

		vfio_pci_zap_and_down_write_memory_lock(vdev);
		ret = pci_try_reset_function(vdev->pdev);
		up_write(&vdev->memory_lock);

		return ret;

	} else if (cmd == VFIO_DEVICE_GET_PCI_HOT_RESET_INFO) {
		struct vfio_pci_hot_reset_info hdr;
		struct vfio_pci_fill_info fill = { 0 };
		int32_t *group_fds;
		struct vfio_pci_group_entry *groups;
		struct vfio_pci_group_info info;
		struct vfio_devices devs = { .cur_index = 0 };
		bool slot = false;
		int i, group_idx, mem_idx = 0, count = 0, ret = 0;

		minsz = offsetofend(struct vfio_pci_hot_reset, count);

		if (copy_from_user(&hdr, (void __user *)arg, minsz))
		 * user interface and store the group and iommu ID.  This
		 * ensures the group is held across the reset.
		 */
		for (group_idx = 0; group_idx < hdr.count; group_idx++) {
			struct vfio_group *group;
			struct fd f = fdget(group_fds[group_idx]);
			if (!f.file) {
				ret = -EBADF;
				break;
			}
				break;
			}

			groups[group_idx].group = group;
			groups[group_idx].id =
					vfio_external_user_iommu_id(group);
		}

		kfree(group_fds);

		ret = vfio_pci_for_each_slot_or_bus(vdev->pdev,
						    vfio_pci_validate_devs,
						    &info, slot);
		if (ret)
			goto hot_reset_release;

		devs.max_index = count;
		devs.devices = kcalloc(count, sizeof(struct vfio_device *),
				       GFP_KERNEL);
		if (!devs.devices) {
			ret = -ENOMEM;
			goto hot_reset_release;
		}

		/*
		 * We need to get memory_lock for each device, but devices
		 * can share mmap_sem, therefore we need to zap and hold
		 * the vma_lock for each device, and only then get each
		 * memory_lock.
		 */
		ret = vfio_pci_for_each_slot_or_bus(vdev->pdev,
					    vfio_pci_try_zap_and_vma_lock_cb,
					    &devs, slot);
		if (ret)
			goto hot_reset_release;

		for (; mem_idx < devs.cur_index; mem_idx++) {
			struct vfio_pci_device *tmp;

			tmp = vfio_device_data(devs.devices[mem_idx]);

			ret = down_write_trylock(&tmp->memory_lock);
			if (!ret) {
				ret = -EBUSY;
				goto hot_reset_release;
			}
			mutex_unlock(&tmp->vma_lock);
		}

		/* User has access, do the reset */
		ret = pci_reset_bus(vdev->pdev);

hot_reset_release:
		for (i = 0; i < devs.cur_index; i++) {
			struct vfio_device *device;
			struct vfio_pci_device *tmp;

			device = devs.devices[i];
			tmp = vfio_device_data(device);

			if (i < mem_idx)
				up_write(&tmp->memory_lock);
			else
				mutex_unlock(&tmp->vma_lock);
			vfio_device_put(device);
		}
		kfree(devs.devices);

		for (group_idx--; group_idx >= 0; group_idx--)
			vfio_group_put_external_user(groups[group_idx].group);

		kfree(groups);
		return ret;
	} else if (cmd == VFIO_DEVICE_IOEVENTFD) {
	return vfio_pci_rw(device_data, (char __user *)buf, count, ppos, true);
}

/* Return 1 on zap and vma_lock acquired, 0 on contention (only with @try) */
static int vfio_pci_zap_and_vma_lock(struct vfio_pci_device *vdev, bool try)
{
	struct vfio_pci_mmap_vma *mmap_vma, *tmp;

	/*
	 * Lock ordering:
	 * vma_lock is nested under mmap_sem for vm_ops callback paths.
	 * The memory_lock semaphore is used by both code paths calling
	 * into this function to zap vmas and the vm_ops.fault callback
	 * to protect the memory enable state of the device.
	 *
	 * When zapping vmas we need to maintain the mmap_sem => vma_lock
	 * ordering, which requires using vma_lock to walk vma_list to
	 * acquire an mm, then dropping vma_lock to get the mmap_sem and
	 * reacquiring vma_lock.  This logic is derived from similar
	 * requirements in uverbs_user_mmap_disassociate().
	 *
	 * mmap_sem must always be the top-level lock when it is taken.
	 * Therefore we can only hold the memory_lock write lock when
	 * vma_list is empty, as we'd need to take mmap_sem to clear
	 * entries.  vma_list can only be guaranteed empty when holding
	 * vma_lock, thus memory_lock is nested under vma_lock.
	 *
	 * This enables the vm_ops.fault callback to acquire vma_lock,
	 * followed by memory_lock read lock, while already holding
	 * mmap_sem without risk of deadlock.
	 */
	while (1) {
		struct mm_struct *mm = NULL;

		if (try) {
			if (!mutex_trylock(&vdev->vma_lock))
				return 0;
		} else {
			mutex_lock(&vdev->vma_lock);
		}
		while (!list_empty(&vdev->vma_list)) {
			mmap_vma = list_first_entry(&vdev->vma_list,
						    struct vfio_pci_mmap_vma,
						    vma_next);
			mm = mmap_vma->vma->vm_mm;
			if (mmget_not_zero(mm))
				break;

			list_del(&mmap_vma->vma_next);
			kfree(mmap_vma);
			mm = NULL;
		}
		if (!mm)
			return 1;
		mutex_unlock(&vdev->vma_lock);

		if (try) {
			if (!down_read_trylock(&mm->mmap_sem)) {
				mmput(mm);
				return 0;
			}
		} else {
			down_read(&mm->mmap_sem);
		}
		if (mmget_still_valid(mm)) {
			if (try) {
				if (!mutex_trylock(&vdev->vma_lock)) {
					up_read(&mm->mmap_sem);
					mmput(mm);
					return 0;
				}
			} else {
				mutex_lock(&vdev->vma_lock);
			}
			list_for_each_entry_safe(mmap_vma, tmp,
						 &vdev->vma_list, vma_next) {
				struct vm_area_struct *vma = mmap_vma->vma;

				if (vma->vm_mm != mm)
					continue;

				list_del(&mmap_vma->vma_next);
				kfree(mmap_vma);

				zap_vma_ptes(vma, vma->vm_start,
					     vma->vm_end - vma->vm_start);
			}
			mutex_unlock(&vdev->vma_lock);
		}
		up_read(&mm->mmap_sem);
		mmput(mm);
	}
}

void vfio_pci_zap_and_down_write_memory_lock(struct vfio_pci_device *vdev)
{
	vfio_pci_zap_and_vma_lock(vdev, false);
	down_write(&vdev->memory_lock);
	mutex_unlock(&vdev->vma_lock);
}

u16 vfio_pci_memory_lock_and_enable(struct vfio_pci_device *vdev)
{
	u16 cmd;

	down_write(&vdev->memory_lock);
	pci_read_config_word(vdev->pdev, PCI_COMMAND, &cmd);
	if (!(cmd & PCI_COMMAND_MEMORY))
		pci_write_config_word(vdev->pdev, PCI_COMMAND,
				      cmd | PCI_COMMAND_MEMORY);

	return cmd;
}

void vfio_pci_memory_unlock_and_restore(struct vfio_pci_device *vdev, u16 cmd)
{
	pci_write_config_word(vdev->pdev, PCI_COMMAND, cmd);
	up_write(&vdev->memory_lock);
}

/* Caller holds vma_lock */
static int __vfio_pci_add_vma(struct vfio_pci_device *vdev,
			      struct vm_area_struct *vma)
{
	struct vfio_pci_mmap_vma *mmap_vma;

	mmap_vma = kmalloc(sizeof(*mmap_vma), GFP_KERNEL);
		return -ENOMEM;

	mmap_vma->vma = vma;
	list_add(&mmap_vma->vma_next, &vdev->vma_list);

	return 0;
}

{
	struct vm_area_struct *vma = vmf->vma;
	struct vfio_pci_device *vdev = vma->vm_private_data;
	vm_fault_t ret = VM_FAULT_NOPAGE;

	mutex_lock(&vdev->vma_lock);
	down_read(&vdev->memory_lock);

	if (!__vfio_pci_memory_enabled(vdev)) {
		ret = VM_FAULT_SIGBUS;
		mutex_unlock(&vdev->vma_lock);
		goto up_out;
	}

	if (__vfio_pci_add_vma(vdev, vma)) {
		ret = VM_FAULT_OOM;
		mutex_unlock(&vdev->vma_lock);
		goto up_out;
	}

	mutex_unlock(&vdev->vma_lock);

	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			    vma->vm_end - vma->vm_start, vma->vm_page_prot))
		ret = VM_FAULT_SIGBUS;

up_out:
	up_read(&vdev->memory_lock);
	return ret;
}

static const struct vm_operations_struct vfio_pci_mmap_ops = {
	.open = vfio_pci_mmap_open,
	INIT_LIST_HEAD(&vdev->ioeventfds_list);
	mutex_init(&vdev->vma_lock);
	INIT_LIST_HEAD(&vdev->vma_list);
	init_rwsem(&vdev->memory_lock);

	ret = vfio_add_group_dev(&pdev->dev, &vfio_pci_ops, vdev);
	if (ret)
		goto out_free;
	kref_put_mutex(&reflck->kref, vfio_pci_reflck_release, &reflck_lock);
}

static int vfio_pci_get_unused_devs(struct pci_dev *pdev, void *data)
{
	struct vfio_devices *devs = data;
	struct vfio_device *device;
	return 0;
}

static int vfio_pci_try_zap_and_vma_lock_cb(struct pci_dev *pdev, void *data)
{
	struct vfio_devices *devs = data;
	struct vfio_device *device;
	struct vfio_pci_device *vdev;

	if (devs->cur_index == devs->max_index)
		return -ENOSPC;

	device = vfio_device_get_from_dev(&pdev->dev);
	if (!device)
		return -EINVAL;

	if (pci_dev_driver(pdev) != &vfio_pci_driver) {
		vfio_device_put(device);
		return -EBUSY;
	}

	vdev = vfio_device_data(device);

	/*
	 * Locking multiple devices is prone to deadlock, runaway and
	 * unwind if we hit contention.
	 */
	if (!vfio_pci_zap_and_vma_lock(vdev, true)) {
		vfio_device_put(device);
		return -EBUSY;
	}

	devs->devices[devs->cur_index++] = device;
	return 0;
}

/*
 * If a bus or slot reset is available for the provided device and:
 *  - All of the devices affected by that bus or slot reset are unused
 *    (!refcnt)