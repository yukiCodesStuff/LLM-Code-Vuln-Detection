#include <linux/slab.h>
#include <linux/irqnr.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/cpuhotplug.h>
#include <linux/atomic.h>
#include <linux/ktime.h>
struct irq_info {
	struct list_head list;
	struct list_head eoi_list;
	short refcnt;
	u8 spurious_cnt;
	u8 is_accounted;
	short type;		/* type: IRQT_* */
 */
static DEFINE_MUTEX(irq_mapping_update_lock);

/*
 * Lock protecting event handling loop against removing event channels.
 * Adding of event channels is no issue as the associated IRQ becomes active
 * only after everything is setup (before request_[threaded_]irq() the handler
 * can't be entered for an event, as the event channel will be unmasked only
 * then).
 */
static DEFINE_RWLOCK(evtchn_rwlock);

/*
 * Lock hierarchy:
 *
 * irq_mapping_update_lock
 *   evtchn_rwlock
 *     IRQ-desc lock
 *       percpu eoi_list_lock
 *         irq_info->lock
 */

static LIST_HEAD(xen_irq_list_head);

	info->is_accounted = 1;
}

/* Constructors for packed IRQ information. */
static int xen_irq_info_common_setup(struct irq_info *info,
				     unsigned irq,
				     enum xen_irq_type type,

	eoi = container_of(to_delayed_work(work), struct lateeoi_work, delayed);

	read_lock_irqsave(&evtchn_rwlock, flags);

	while (true) {
		spin_lock(&eoi->eoi_list_lock);

		info = list_first_entry_or_null(&eoi->eoi_list, struct irq_info,
						eoi_list);

		if (info == NULL || now < info->eoi_time) {
			spin_unlock(&eoi->eoi_list_lock);
			break;
		}

		list_del_init(&info->eoi_list);

		spin_unlock(&eoi->eoi_list_lock);

		info->eoi_time = 0;

		xen_irq_lateeoi_locked(info, false);
	}

	if (info)
		mod_delayed_work_on(info->eoi_cpu, system_wq,
				    &eoi->delayed, info->eoi_time - now);

	read_unlock_irqrestore(&evtchn_rwlock, flags);
}

static void xen_cpu_init_eoi(unsigned int cpu)
{
void xen_irq_lateeoi(unsigned int irq, unsigned int eoi_flags)
{
	struct irq_info *info;
	unsigned long flags;

	read_lock_irqsave(&evtchn_rwlock, flags);

	info = info_for_irq(irq);

	if (info)
		xen_irq_lateeoi_locked(info, eoi_flags & XEN_EOI_FLAG_SPURIOUS);

	read_unlock_irqrestore(&evtchn_rwlock, flags);
}
EXPORT_SYMBOL_GPL(xen_irq_lateeoi);

static void xen_irq_init(unsigned irq)

	info->type = IRQT_UNBOUND;
	info->refcnt = -1;

	set_info_for_irq(irq, info);
	/*
	 * Interrupt affinity setting can be immediate. No point
static void xen_free_irq(unsigned irq)
{
	struct irq_info *info = info_for_irq(irq);
	unsigned long flags;

	if (WARN_ON(!info))
		return;

	write_lock_irqsave(&evtchn_rwlock, flags);

	if (!list_empty(&info->eoi_list))
		lateeoi_list_del(info);

	list_del(&info->list);

	set_info_for_irq(irq, NULL);

	WARN_ON(info->refcnt > 0);

	write_unlock_irqrestore(&evtchn_rwlock, flags);

	kfree(info);

	/* Legacy IRQ descriptors are managed by the arch. */
	if (irq < nr_legacy_irqs())
		return;

	irq_free_desc(irq);
}

/* Not called for lateeoi events. */
static void event_handler_exit(struct irq_info *info)
	int cpu = smp_processor_id();
	struct evtchn_loop_ctrl ctrl = { 0 };

	read_lock(&evtchn_rwlock);

	do {
		vcpu_info->evtchn_upcall_pending = 0;


	} while (vcpu_info->evtchn_upcall_pending);

	read_unlock(&evtchn_rwlock);

	/*
	 * Increment irq_epoch only now to defer EOIs only for
	 * xen_irq_lateeoi() invocations occurring from inside the loop