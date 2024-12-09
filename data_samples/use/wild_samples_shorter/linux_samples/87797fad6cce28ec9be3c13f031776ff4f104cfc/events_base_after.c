#include <linux/slab.h>
#include <linux/irqnr.h>
#include <linux/pci.h>
#include <linux/rcupdate.h>
#include <linux/spinlock.h>
#include <linux/cpuhotplug.h>
#include <linux/atomic.h>
#include <linux/ktime.h>
struct irq_info {
	struct list_head list;
	struct list_head eoi_list;
	struct rcu_work rwork;
	short refcnt;
	u8 spurious_cnt;
	u8 is_accounted;
	short type;		/* type: IRQT_* */
 */
static DEFINE_MUTEX(irq_mapping_update_lock);

/*
 * Lock hierarchy:
 *
 * irq_mapping_update_lock
 *   IRQ-desc lock
 *     percpu eoi_list_lock
 *       irq_info->lock
 */

static LIST_HEAD(xen_irq_list_head);

	info->is_accounted = 1;
}

static void delayed_free_irq(struct work_struct *work)
{
	struct irq_info *info = container_of(to_rcu_work(work), struct irq_info,
					     rwork);
	unsigned int irq = info->irq;

	/* Remove the info pointer only now, with no potential users left. */
	set_info_for_irq(irq, NULL);

	kfree(info);

	/* Legacy IRQ descriptors are managed by the arch. */
	if (irq >= nr_legacy_irqs())
		irq_free_desc(irq);
}

/* Constructors for packed IRQ information. */
static int xen_irq_info_common_setup(struct irq_info *info,
				     unsigned irq,
				     enum xen_irq_type type,

	eoi = container_of(to_delayed_work(work), struct lateeoi_work, delayed);

	rcu_read_lock();

	while (true) {
		spin_lock_irqsave(&eoi->eoi_list_lock, flags);

		info = list_first_entry_or_null(&eoi->eoi_list, struct irq_info,
						eoi_list);

		if (info == NULL)
			break;

		if (now < info->eoi_time) {
			mod_delayed_work_on(info->eoi_cpu, system_wq,
					    &eoi->delayed,
					    info->eoi_time - now);
			break;
		}

		list_del_init(&info->eoi_list);

		spin_unlock_irqrestore(&eoi->eoi_list_lock, flags);

		info->eoi_time = 0;

		xen_irq_lateeoi_locked(info, false);
	}

	spin_unlock_irqrestore(&eoi->eoi_list_lock, flags);

	rcu_read_unlock();
}

static void xen_cpu_init_eoi(unsigned int cpu)
{
void xen_irq_lateeoi(unsigned int irq, unsigned int eoi_flags)
{
	struct irq_info *info;

	rcu_read_lock();

	info = info_for_irq(irq);

	if (info)
		xen_irq_lateeoi_locked(info, eoi_flags & XEN_EOI_FLAG_SPURIOUS);

	rcu_read_unlock();
}
EXPORT_SYMBOL_GPL(xen_irq_lateeoi);

static void xen_irq_init(unsigned irq)

	info->type = IRQT_UNBOUND;
	info->refcnt = -1;
	INIT_RCU_WORK(&info->rwork, delayed_free_irq);

	set_info_for_irq(irq, info);
	/*
	 * Interrupt affinity setting can be immediate. No point
static void xen_free_irq(unsigned irq)
{
	struct irq_info *info = info_for_irq(irq);

	if (WARN_ON(!info))
		return;

	if (!list_empty(&info->eoi_list))
		lateeoi_list_del(info);

	list_del(&info->list);

	WARN_ON(info->refcnt > 0);

	queue_rcu_work(system_wq, &info->rwork);
}

/* Not called for lateeoi events. */
static void event_handler_exit(struct irq_info *info)
	int cpu = smp_processor_id();
	struct evtchn_loop_ctrl ctrl = { 0 };

	/*
	 * When closing an event channel the associated IRQ must not be freed
	 * until all cpus have left the event handling loop. This is ensured
	 * by taking the rcu_read_lock() while handling events, as freeing of
	 * the IRQ is handled via queue_rcu_work() _after_ closing the event
	 * channel.
	 */
	rcu_read_lock();

	do {
		vcpu_info->evtchn_upcall_pending = 0;


	} while (vcpu_info->evtchn_upcall_pending);

	rcu_read_unlock();

	/*
	 * Increment irq_epoch only now to defer EOIs only for
	 * xen_irq_lateeoi() invocations occurring from inside the loop