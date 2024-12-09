#include <linux/mutex.h>
#include <linux/export.h>
#include <linux/pm_runtime.h>

#include "power.h"

static DEFINE_MUTEX(dev_pm_qos_mtx);
	struct pm_qos_flags *pqf;
	s32 val;

	if (!qos)
		return PM_QOS_FLAGS_UNDEFINED;

	pqf = &qos->flags;
	if (list_empty(&pqf->list))
 */
s32 __dev_pm_qos_read_value(struct device *dev)
{
	return dev->power.qos ? pm_qos_read_value(&dev->power.qos->latency) : 0;
}

/**
 * dev_pm_qos_read_value - Get PM QoS constraint for a given device (locked).
	return 0;
}

/**
 * dev_pm_qos_constraints_init - Initalize device's PM QoS constraints pointer.
 * @dev: target device
 *
 * Called from the device PM subsystem during device insertion under
 * device_pm_lock().
 */
void dev_pm_qos_constraints_init(struct device *dev)
{
	mutex_lock(&dev_pm_qos_mtx);
	dev->power.qos = NULL;
	dev->power.power_state = PMSG_ON;
	mutex_unlock(&dev_pm_qos_mtx);
}

/**
 * dev_pm_qos_constraints_destroy
 * @dev: target device
	struct pm_qos_constraints *c;
	struct pm_qos_flags *f;

	/*
	 * If the device's PM QoS resume latency limit or PM QoS flags have been
	 * exposed to user space, they have to be hidden at this point.
	 */
	dev_pm_qos_hide_latency_limit(dev);
	dev_pm_qos_hide_flags(dev);

	mutex_lock(&dev_pm_qos_mtx);

	dev->power.power_state = PMSG_INVALID;
	qos = dev->power.qos;
	if (!qos)
		goto out;

	}

	spin_lock_irq(&dev->power.lock);
	dev->power.qos = NULL;
	spin_unlock_irq(&dev->power.lock);

	kfree(c->notifiers);
	kfree(qos);
		 "%s() called for already added request\n", __func__))
		return -EINVAL;

	req->dev = dev;

	mutex_lock(&dev_pm_qos_mtx);

	if (!dev->power.qos) {
		if (dev->power.power_state.event == PM_EVENT_INVALID) {
			/* The device has been removed from the system. */
			req->dev = NULL;
			ret = -ENODEV;
			goto out;
		} else {
			/*
			 * Allocate the constraints data on the first call to
			 * add_request, i.e. only if the data is not already
			 * allocated and if the device has not been removed.
			 */
			ret = dev_pm_qos_constraints_allocate(dev);
		}
	}

	if (!ret) {
		req->type = type;
		ret = apply_constraint(req, PM_QOS_ADD_REQ, value);
	}

 out:
	mutex_unlock(&dev_pm_qos_mtx);

	return ret;
}
	s32 curr_value;
	int ret = 0;

	if (!req->dev->power.qos)
		return -ENODEV;

	switch(req->type) {
	case DEV_PM_QOS_LATENCY:
{
	int ret;

	if (!req) /*guard against callers passing in null */
		return -EINVAL;

	if (WARN(!dev_pm_qos_request_active(req),
		 "%s() called for unknown object\n", __func__))
		return -EINVAL;

	mutex_lock(&dev_pm_qos_mtx);
	ret = __dev_pm_qos_update_request(req, new_value);
	mutex_unlock(&dev_pm_qos_mtx);

	return ret;
}
EXPORT_SYMBOL_GPL(dev_pm_qos_update_request);

/**
 * dev_pm_qos_remove_request - modifies an existing qos request
 * @req: handle to request list element
 */
int dev_pm_qos_remove_request(struct dev_pm_qos_request *req)
{
	int ret = 0;

	if (!req) /*guard against callers passing in null */
		return -EINVAL;

	if (WARN(!dev_pm_qos_request_active(req),
		 "%s() called for unknown object\n", __func__))
		return -EINVAL;

	mutex_lock(&dev_pm_qos_mtx);

	if (req->dev->power.qos) {
		ret = apply_constraint(req, PM_QOS_REMOVE_REQ,
				       PM_QOS_DEFAULT_VALUE);
		memset(req, 0, sizeof(*req));
	} else {
		/* Return if the device has been removed */
		ret = -ENODEV;
	}

	mutex_unlock(&dev_pm_qos_mtx);
	return ret;
}
EXPORT_SYMBOL_GPL(dev_pm_qos_remove_request);

	mutex_lock(&dev_pm_qos_mtx);

	if (!dev->power.qos)
		ret = dev->power.power_state.event != PM_EVENT_INVALID ?
			dev_pm_qos_constraints_allocate(dev) : -ENODEV;

	if (!ret)
		ret = blocking_notifier_chain_register(
				dev->power.qos->latency.notifiers, notifier);
	mutex_lock(&dev_pm_qos_mtx);

	/* Silently return if the constraints object is not present. */
	if (dev->power.qos)
		retval = blocking_notifier_chain_unregister(
				dev->power.qos->latency.notifiers,
				notifier);

static void __dev_pm_qos_drop_user_request(struct device *dev,
					   enum dev_pm_qos_req_type type)
{
	switch(type) {
	case DEV_PM_QOS_LATENCY:
		dev_pm_qos_remove_request(dev->power.qos->latency_req);
		dev->power.qos->latency_req = NULL;
		break;
	case DEV_PM_QOS_FLAGS:
		dev_pm_qos_remove_request(dev->power.qos->flags_req);
		dev->power.qos->flags_req = NULL;
		break;
	}
}

/**
 * dev_pm_qos_expose_latency_limit - Expose PM QoS latency limit to user space.
	if (!device_is_registered(dev) || value < 0)
		return -EINVAL;

	if (dev->power.qos && dev->power.qos->latency_req)
		return -EEXIST;

	req = kzalloc(sizeof(*req), GFP_KERNEL);
	if (!req)
		return -ENOMEM;

	ret = dev_pm_qos_add_request(dev, req, DEV_PM_QOS_LATENCY, value);
	if (ret < 0)
		return ret;

	dev->power.qos->latency_req = req;
	ret = pm_qos_sysfs_add_latency(dev);
	if (ret)
		__dev_pm_qos_drop_user_request(dev, DEV_PM_QOS_LATENCY);

	return ret;
}
EXPORT_SYMBOL_GPL(dev_pm_qos_expose_latency_limit);

/**
 * dev_pm_qos_hide_latency_limit - Hide PM QoS latency limit from user space.
 * @dev: Device whose PM QoS latency limit is to be hidden from user space.
 */
void dev_pm_qos_hide_latency_limit(struct device *dev)
{
	if (dev->power.qos && dev->power.qos->latency_req) {
		pm_qos_sysfs_remove_latency(dev);
		__dev_pm_qos_drop_user_request(dev, DEV_PM_QOS_LATENCY);
	}
}
EXPORT_SYMBOL_GPL(dev_pm_qos_hide_latency_limit);

/**
	if (!device_is_registered(dev))
		return -EINVAL;

	if (dev->power.qos && dev->power.qos->flags_req)
		return -EEXIST;

	req = kzalloc(sizeof(*req), GFP_KERNEL);
	if (!req)
		return -ENOMEM;

	pm_runtime_get_sync(dev);
	ret = dev_pm_qos_add_request(dev, req, DEV_PM_QOS_FLAGS, val);
	if (ret < 0)
		goto fail;

	dev->power.qos->flags_req = req;
	ret = pm_qos_sysfs_add_flags(dev);
	if (ret)
		__dev_pm_qos_drop_user_request(dev, DEV_PM_QOS_FLAGS);

fail:
	pm_runtime_put(dev);
	return ret;
}
EXPORT_SYMBOL_GPL(dev_pm_qos_expose_flags);

/**
 * dev_pm_qos_hide_flags - Hide PM QoS flags of a device from user space.
 * @dev: Device whose PM QoS flags are to be hidden from user space.
 */
void dev_pm_qos_hide_flags(struct device *dev)
{
	if (dev->power.qos && dev->power.qos->flags_req) {
		pm_qos_sysfs_remove_flags(dev);
		pm_runtime_get_sync(dev);
		__dev_pm_qos_drop_user_request(dev, DEV_PM_QOS_FLAGS);
		pm_runtime_put(dev);
	}
}
EXPORT_SYMBOL_GPL(dev_pm_qos_hide_flags);

/**
	s32 value;
	int ret;

	if (!dev->power.qos || !dev->power.qos->flags_req)
		return -EINVAL;

	pm_runtime_get_sync(dev);
	mutex_lock(&dev_pm_qos_mtx);

	value = dev_pm_qos_requested_flags(dev);
	if (set)
		value |= mask;
	else

	ret = __dev_pm_qos_update_request(dev->power.qos->flags_req, value);

	mutex_unlock(&dev_pm_qos_mtx);
	pm_runtime_put(dev);

	return ret;
}
#endif /* CONFIG_PM_RUNTIME */