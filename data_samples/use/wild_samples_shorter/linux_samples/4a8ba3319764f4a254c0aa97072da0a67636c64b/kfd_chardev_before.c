#include <uapi/linux/kfd_ioctl.h>
#include <linux/time.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <uapi/asm-generic/mman-common.h>
#include <asm/processor.h>
#include "kfd_priv.h"
#include "kfd_device_queue_manager.h"
	if (IS_ERR(process))
		return PTR_ERR(process);

	process->is_32bit_user_mode = is_32bit_user_mode;

	dev_dbg(kfd_device, "process %d opened, compat mode (32 bit) - %d\n",
		process->pasid, process->is_32bit_user_mode);

	kfd_init_apertures(process);

	return 0;
}

static long kfd_ioctl_get_version(struct file *filep, struct kfd_process *p,
					void __user *arg)
{
	struct kfd_ioctl_get_version_args args;
	int err = 0;

	args.major_version = KFD_IOCTL_MAJOR_VERSION;
	args.minor_version = KFD_IOCTL_MINOR_VERSION;

	if (copy_to_user(arg, &args, sizeof(args)))
		err = -EFAULT;

	return err;
}

	return 0;
}

static long kfd_ioctl_create_queue(struct file *filep, struct kfd_process *p,
					void __user *arg)
{
	struct kfd_ioctl_create_queue_args args;
	struct kfd_dev *dev;
	int err = 0;
	unsigned int queue_id;
	struct kfd_process_device *pdd;

	memset(&q_properties, 0, sizeof(struct queue_properties));

	if (copy_from_user(&args, arg, sizeof(args)))
		return -EFAULT;

	pr_debug("kfd: creating queue ioctl\n");

	err = set_queue_properties_from_user(&q_properties, &args);
	if (err)
		return err;

	dev = kfd_device_by_id(args.gpu_id);
	if (dev == NULL)
		return -EINVAL;

	mutex_lock(&p->mutex);

	pdd = kfd_bind_process_to_device(dev, p);
	if (IS_ERR(pdd)) {
		err = PTR_ERR(pdd);
		goto err_bind_process;
	}

	pr_debug("kfd: creating queue for PASID %d on GPU 0x%x\n",
	if (err != 0)
		goto err_create_queue;

	args.queue_id = queue_id;

	/* Return gpu_id as doorbell offset for mmap usage */
	args.doorbell_offset = args.gpu_id << PAGE_SHIFT;

	if (copy_to_user(arg, &args, sizeof(args))) {
		err = -EFAULT;
		goto err_copy_args_out;
	}

	mutex_unlock(&p->mutex);

	pr_debug("kfd: queue id %d was created successfully\n", args.queue_id);

	pr_debug("ring buffer address == 0x%016llX\n",
			args.ring_base_address);

	pr_debug("read ptr address    == 0x%016llX\n",
			args.read_pointer_address);

	pr_debug("write ptr address   == 0x%016llX\n",
			args.write_pointer_address);

	return 0;

err_copy_args_out:
	pqm_destroy_queue(&p->pqm, queue_id);
err_create_queue:
err_bind_process:
	mutex_unlock(&p->mutex);
	return err;
}

static int kfd_ioctl_destroy_queue(struct file *filp, struct kfd_process *p,
					void __user *arg)
{
	int retval;
	struct kfd_ioctl_destroy_queue_args args;

	if (copy_from_user(&args, arg, sizeof(args)))
		return -EFAULT;

	pr_debug("kfd: destroying queue id %d for PASID %d\n",
				args.queue_id,
				p->pasid);

	mutex_lock(&p->mutex);

	retval = pqm_destroy_queue(&p->pqm, args.queue_id);

	mutex_unlock(&p->mutex);
	return retval;
}

static int kfd_ioctl_update_queue(struct file *filp, struct kfd_process *p,
					void __user *arg)
{
	int retval;
	struct kfd_ioctl_update_queue_args args;
	struct queue_properties properties;

	if (copy_from_user(&args, arg, sizeof(args)))
		return -EFAULT;

	if (args.queue_percentage > KFD_MAX_QUEUE_PERCENTAGE) {
		pr_err("kfd: queue percentage must be between 0 to KFD_MAX_QUEUE_PERCENTAGE\n");
		return -EINVAL;
	}

	if (args.queue_priority > KFD_MAX_QUEUE_PRIORITY) {
		pr_err("kfd: queue priority must be between 0 to KFD_MAX_QUEUE_PRIORITY\n");
		return -EINVAL;
	}

	if ((args.ring_base_address) &&
		(!access_ok(VERIFY_WRITE,
			(const void __user *) args.ring_base_address,
			sizeof(uint64_t)))) {
		pr_err("kfd: can't access ring base address\n");
		return -EFAULT;
	}

	if (!is_power_of_2(args.ring_size) && (args.ring_size != 0)) {
		pr_err("kfd: ring size must be a power of 2 or 0\n");
		return -EINVAL;
	}

	properties.queue_address = args.ring_base_address;
	properties.queue_size = args.ring_size;
	properties.queue_percent = args.queue_percentage;
	properties.priority = args.queue_priority;

	pr_debug("kfd: updating queue id %d for PASID %d\n",
			args.queue_id, p->pasid);

	mutex_lock(&p->mutex);

	retval = pqm_update_queue(&p->pqm, args.queue_id, &properties);

	mutex_unlock(&p->mutex);

	return retval;
}

static long kfd_ioctl_set_memory_policy(struct file *filep,
				struct kfd_process *p, void __user *arg)
{
	struct kfd_ioctl_set_memory_policy_args args;
	struct kfd_dev *dev;
	int err = 0;
	struct kfd_process_device *pdd;
	enum cache_policy default_policy, alternate_policy;

	if (copy_from_user(&args, arg, sizeof(args)))
		return -EFAULT;

	if (args.default_policy != KFD_IOC_CACHE_POLICY_COHERENT
	    && args.default_policy != KFD_IOC_CACHE_POLICY_NONCOHERENT) {
		return -EINVAL;
	}

	if (args.alternate_policy != KFD_IOC_CACHE_POLICY_COHERENT
	    && args.alternate_policy != KFD_IOC_CACHE_POLICY_NONCOHERENT) {
		return -EINVAL;
	}

	dev = kfd_device_by_id(args.gpu_id);
	if (dev == NULL)
		return -EINVAL;

	mutex_lock(&p->mutex);

	pdd = kfd_bind_process_to_device(dev, p);
	if (IS_ERR(pdd)) {
		err = PTR_ERR(pdd);
		goto out;
	}

	default_policy = (args.default_policy == KFD_IOC_CACHE_POLICY_COHERENT)
			 ? cache_policy_coherent : cache_policy_noncoherent;

	alternate_policy =
		(args.alternate_policy == KFD_IOC_CACHE_POLICY_COHERENT)
		   ? cache_policy_coherent : cache_policy_noncoherent;

	if (!dev->dqm->set_cache_memory_policy(dev->dqm,
				&pdd->qpd,
				default_policy,
				alternate_policy,
				(void __user *)args.alternate_aperture_base,
				args.alternate_aperture_size))
		err = -EINVAL;

out:
	mutex_unlock(&p->mutex);
	return err;
}

static long kfd_ioctl_get_clock_counters(struct file *filep,
				struct kfd_process *p, void __user *arg)
{
	struct kfd_ioctl_get_clock_counters_args args;
	struct kfd_dev *dev;
	struct timespec time;

	if (copy_from_user(&args, arg, sizeof(args)))
		return -EFAULT;

	dev = kfd_device_by_id(args.gpu_id);
	if (dev == NULL)
		return -EINVAL;

	/* Reading GPU clock counter from KGD */
	args.gpu_clock_counter = kfd2kgd->get_gpu_clock_counter(dev->kgd);

	/* No access to rdtsc. Using raw monotonic time */
	getrawmonotonic(&time);
	args.cpu_clock_counter = (uint64_t)timespec_to_ns(&time);

	get_monotonic_boottime(&time);
	args.system_clock_counter = (uint64_t)timespec_to_ns(&time);

	/* Since the counter is in nano-seconds we use 1GHz frequency */
	args.system_clock_freq = 1000000000;

	if (copy_to_user(arg, &args, sizeof(args)))
		return -EFAULT;

	return 0;
}


static int kfd_ioctl_get_process_apertures(struct file *filp,
				struct kfd_process *p, void __user *arg)
{
	struct kfd_ioctl_get_process_apertures_args args;
	struct kfd_process_device_apertures *pAperture;
	struct kfd_process_device *pdd;

	dev_dbg(kfd_device, "get apertures for PASID %d", p->pasid);

	if (copy_from_user(&args, arg, sizeof(args)))
		return -EFAULT;

	args.num_of_nodes = 0;

	mutex_lock(&p->mutex);

	/*if the process-device list isn't empty*/
		/* Run over all pdd of the process */
		pdd = kfd_get_first_process_device_data(p);
		do {
			pAperture = &args.process_apertures[args.num_of_nodes];
			pAperture->gpu_id = pdd->dev->id;
			pAperture->lds_base = pdd->lds_base;
			pAperture->lds_limit = pdd->lds_limit;
			pAperture->gpuvm_base = pdd->gpuvm_base;
			pAperture->scratch_limit = pdd->scratch_limit;

			dev_dbg(kfd_device,
				"node id %u\n", args.num_of_nodes);
			dev_dbg(kfd_device,
				"gpu id %u\n", pdd->dev->id);
			dev_dbg(kfd_device,
				"lds_base %llX\n", pdd->lds_base);
			dev_dbg(kfd_device,
				"scratch_limit %llX\n", pdd->scratch_limit);

			args.num_of_nodes++;
		} while ((pdd = kfd_get_next_process_device_data(p, pdd)) != NULL &&
				(args.num_of_nodes < NUM_OF_SUPPORTED_GPUS));
	}

	mutex_unlock(&p->mutex);

	if (copy_to_user(arg, &args, sizeof(args)))
		return -EFAULT;

	return 0;
}

static long kfd_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	struct kfd_process *process;
	long err = -EINVAL;

	dev_dbg(kfd_device,
		"ioctl cmd 0x%x (#%d), arg 0x%lx\n",
		cmd, _IOC_NR(cmd), arg);

	process = kfd_get_process(current);
	if (IS_ERR(process))
		return PTR_ERR(process);

	switch (cmd) {
	case KFD_IOC_GET_VERSION:
		err = kfd_ioctl_get_version(filep, process, (void __user *)arg);
		break;
	case KFD_IOC_CREATE_QUEUE:
		err = kfd_ioctl_create_queue(filep, process,
						(void __user *)arg);
		break;

	case KFD_IOC_DESTROY_QUEUE:
		err = kfd_ioctl_destroy_queue(filep, process,
						(void __user *)arg);
		break;

	case KFD_IOC_SET_MEMORY_POLICY:
		err = kfd_ioctl_set_memory_policy(filep, process,
						(void __user *)arg);
		break;

	case KFD_IOC_GET_CLOCK_COUNTERS:
		err = kfd_ioctl_get_clock_counters(filep, process,
						(void __user *)arg);
		break;

	case KFD_IOC_GET_PROCESS_APERTURES:
		err = kfd_ioctl_get_process_apertures(filep, process,
						(void __user *)arg);
		break;

	case KFD_IOC_UPDATE_QUEUE:
		err = kfd_ioctl_update_queue(filep, process,
						(void __user *)arg);
		break;

	default:
		dev_err(kfd_device,
			"unknown ioctl cmd 0x%x, arg 0x%lx)\n",
			cmd, arg);
		err = -EINVAL;
		break;
	}

	if (err < 0)
		dev_err(kfd_device,
			"ioctl error %ld for ioctl cmd 0x%x (#%d)\n",
			err, cmd, _IOC_NR(cmd));

	return err;
}

static int kfd_mmap(struct file *filp, struct vm_area_struct *vma)
{