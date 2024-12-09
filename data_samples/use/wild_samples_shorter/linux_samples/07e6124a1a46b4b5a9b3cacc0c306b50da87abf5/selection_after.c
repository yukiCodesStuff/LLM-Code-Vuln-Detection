#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <linux/uaccess.h>
static int sel_end;
static int sel_buffer_lth;
static char *sel_buffer;
static DEFINE_MUTEX(sel_lock);

/* clear_selection, highlight and highlight_pointer can be called
   from interrupt (via scrollback/front) */

	char *bp, *obp;
	int i, ps, pe, multiplier;
	u32 c;
	int mode, ret = 0;

	poke_blanked_console();

	v->xs = min_t(u16, v->xs - 1, vc->vc_cols - 1);
	if (ps > pe)	/* make sel_start <= sel_end */
		swap(ps, pe);

	mutex_lock(&sel_lock);
	if (sel_cons != vc_cons[fg_console].d) {
		clear_selection();
		sel_cons = vc_cons[fg_console].d;
	}
			break;
		case TIOCL_SELPOINTER:
			highlight_pointer(pe);
			goto unlock;
		default:
			ret = -EINVAL;
			goto unlock;
	}

	/* remove the pointer */
	highlight_pointer(-1);
	else if (new_sel_start == sel_start)
	{
		if (new_sel_end == sel_end)	/* no action required */
			goto unlock;
		else if (new_sel_end > sel_end)	/* extend to right */
			highlight(sel_end + 2, new_sel_end);
		else				/* contract from right */
			highlight(new_sel_end + 2, sel_end);
	if (!bp) {
		printk(KERN_WARNING "selection: kmalloc() failed\n");
		clear_selection();
		ret = -ENOMEM;
		goto unlock;
	}
	kfree(sel_buffer);
	sel_buffer = bp;

		}
	}
	sel_buffer_lth = bp - sel_buffer;
unlock:
	mutex_unlock(&sel_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(set_selection_kernel);

/* Insert the contents of the selection buffer into the
	tty_buffer_lock_exclusive(&vc->port);

	add_wait_queue(&vc->paste_wait, &wait);
	mutex_lock(&sel_lock);
	while (sel_buffer && sel_buffer_lth > pasted) {
		set_current_state(TASK_INTERRUPTIBLE);
		if (signal_pending(current)) {
			ret = -EINTR;
			break;
		}
		if (tty_throttled(tty)) {
			mutex_unlock(&sel_lock);
			schedule();
			mutex_lock(&sel_lock);
			continue;
		}
		__set_current_state(TASK_RUNNING);
		count = sel_buffer_lth - pasted;
					      count);
		pasted += count;
	}
	mutex_unlock(&sel_lock);
	remove_wait_queue(&vc->paste_wait, &wait);
	__set_current_state(TASK_RUNNING);

	tty_buffer_unlock_exclusive(&vc->port);