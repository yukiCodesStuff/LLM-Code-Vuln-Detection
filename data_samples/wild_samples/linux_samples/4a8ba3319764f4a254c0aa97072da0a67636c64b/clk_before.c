	if (clk->ops->determine_rate) {
		parent_hw = parent ? parent->hw : NULL;
		new_rate = clk->ops->determine_rate(clk->hw, rate,
						    &best_parent_rate,
						    &parent_hw);
		parent = parent_hw->clk;
	} else if (clk->ops->round_rate) {
		new_rate = clk->ops->round_rate(clk->hw, rate,
						&best_parent_rate);
	} else if (!parent || !(clk->flags & CLK_SET_RATE_PARENT)) {
		/* pass-through clock without adjustable parent */
		clk->new_rate = clk->rate;
		return NULL;
	} else {
		/* pass-through clock with adjustable parent */
		top = clk_calc_new_rates(parent, rate);
		new_rate = parent->new_rate;
		goto out;
	}

	/* some clocks must be gated to change parent */
	if (parent != old_parent &&
	    (clk->flags & CLK_SET_PARENT_GATE) && clk->prepare_count) {
		pr_debug("%s: %s not gated but wants to reparent\n",
			 __func__, clk->name);
		return NULL;
	}

	/* try finding the new parent index */
	if (parent) {
		p_index = clk_fetch_parent_index(clk, parent);
		if (p_index < 0) {
			pr_debug("%s: clk %s can not be parent of clk %s\n",
				 __func__, parent->name, clk->name);
			return NULL;
		}
	}

	if ((clk->flags & CLK_SET_RATE_PARENT) && parent &&
	    best_parent_rate != parent->rate)
		top = clk_calc_new_rates(parent, best_parent_rate);

out:
	clk_calc_subtree(clk, new_rate, parent, p_index);

	return top;
}

/*
 * Notify about rate changes in a subtree. Always walk down the whole tree
 * so that in case of an error we can walk down the whole tree again and
 * abort the change.
 */
static struct clk *clk_propagate_rate_change(struct clk *clk, unsigned long event)
{
	struct clk *child, *tmp_clk, *fail_clk = NULL;
	int ret = NOTIFY_DONE;

	if (clk->rate == clk->new_rate)
		return NULL;

	if (clk->notifier_count) {
		ret = __clk_notify(clk, event, clk->rate, clk->new_rate);
		if (ret & NOTIFY_STOP_MASK)
			fail_clk = clk;
	}

	hlist_for_each_entry(child, &clk->children, child_node) {
		/* Skip children who will be reparented to another clock */
		if (child->new_parent && child->new_parent != clk)
			continue;
		tmp_clk = clk_propagate_rate_change(child, event);
		if (tmp_clk)
			fail_clk = tmp_clk;
	}

	/* handle the new child who might not be in clk->children yet */
	if (clk->new_child) {
		tmp_clk = clk_propagate_rate_change(clk->new_child, event);
		if (tmp_clk)
			fail_clk = tmp_clk;
	}

	return fail_clk;
}

/*
 * walk down a subtree and set the new rates notifying the rate
 * change on the way
 */
static void clk_change_rate(struct clk *clk)
{
	struct clk *child;
	struct hlist_node *tmp;
	unsigned long old_rate;
	unsigned long best_parent_rate = 0;
	bool skip_set_rate = false;
	struct clk *old_parent;

	old_rate = clk->rate;

	if (clk->new_parent)
		best_parent_rate = clk->new_parent->rate;
	else if (clk->parent)
		best_parent_rate = clk->parent->rate;

	if (clk->new_parent && clk->new_parent != clk->parent) {
		old_parent = __clk_set_parent_before(clk, clk->new_parent);

		if (clk->ops->set_rate_and_parent) {
			skip_set_rate = true;
			clk->ops->set_rate_and_parent(clk->hw, clk->new_rate,
					best_parent_rate,
					clk->new_parent_index);
		} else if (clk->ops->set_parent) {
			clk->ops->set_parent(clk->hw, clk->new_parent_index);
		}

		__clk_set_parent_after(clk, clk->new_parent, old_parent);
	}

	if (!skip_set_rate && clk->ops->set_rate)
		clk->ops->set_rate(clk->hw, clk->new_rate, best_parent_rate);

	clk->rate = clk_recalc(clk, best_parent_rate);

	if (clk->notifier_count && old_rate != clk->rate)
		__clk_notify(clk, POST_RATE_CHANGE, old_rate, clk->rate);

	/*
	 * Use safe iteration, as change_rate can actually swap parents
	 * for certain clock types.
	 */
	hlist_for_each_entry_safe(child, tmp, &clk->children, child_node) {
		/* Skip children who will be reparented to another clock */
		if (child->new_parent && child->new_parent != clk)
			continue;
		clk_change_rate(child);
	}

	/* handle the new child who might not be in clk->children yet */
	if (clk->new_child)
		clk_change_rate(clk->new_child);
}

/**
 * clk_set_rate - specify a new rate for clk
 * @clk: the clk whose rate is being changed
 * @rate: the new rate for clk
 *
 * In the simplest case clk_set_rate will only adjust the rate of clk.
 *
 * Setting the CLK_SET_RATE_PARENT flag allows the rate change operation to
 * propagate up to clk's parent; whether or not this happens depends on the
 * outcome of clk's .round_rate implementation.  If *parent_rate is unchanged
 * after calling .round_rate then upstream parent propagation is ignored.  If
 * *parent_rate comes back with a new rate for clk's parent then we propagate
 * up to clk's parent and set its rate.  Upward propagation will continue
 * until either a clk does not support the CLK_SET_RATE_PARENT flag or
 * .round_rate stops requesting changes to clk's parent_rate.
 *
 * Rate changes are accomplished via tree traversal that also recalculates the
 * rates for the clocks and fires off POST_RATE_CHANGE notifiers.
 *
 * Returns 0 on success, -EERROR otherwise.
 */
int clk_set_rate(struct clk *clk, unsigned long rate)
{
	struct clk *top, *fail_clk;
	int ret = 0;

	if (!clk)
		return 0;

	/* prevent racing with updates to the clock topology */
	clk_prepare_lock();

	/* bail early if nothing to do */
	if (rate == clk_get_rate(clk))
		goto out;

	if ((clk->flags & CLK_SET_RATE_GATE) && clk->prepare_count) {
		ret = -EBUSY;
		goto out;
	}

	/* calculate new rates and get the topmost changed clock */
	top = clk_calc_new_rates(clk, rate);
	if (!top) {
		ret = -EINVAL;
		goto out;
	}

	/* notify that we are about to change rates */
	fail_clk = clk_propagate_rate_change(top, PRE_RATE_CHANGE);
	if (fail_clk) {
		pr_debug("%s: failed to set %s rate\n", __func__,
				fail_clk->name);
		clk_propagate_rate_change(top, ABORT_RATE_CHANGE);
		ret = -EBUSY;
		goto out;
	}

	/* change the rates */
	clk_change_rate(top);

out:
	clk_prepare_unlock();

	return ret;
}
EXPORT_SYMBOL_GPL(clk_set_rate);

/**
 * clk_get_parent - return the parent of a clk
 * @clk: the clk whose parent gets returned
 *
 * Simply returns clk->parent.  Returns NULL if clk is NULL.
 */
struct clk *clk_get_parent(struct clk *clk)
{
	struct clk *parent;

	clk_prepare_lock();
	parent = __clk_get_parent(clk);
	clk_prepare_unlock();

	return parent;
}
EXPORT_SYMBOL_GPL(clk_get_parent);

/*
 * .get_parent is mandatory for clocks with multiple possible parents.  It is
 * optional for single-parent clocks.  Always call .get_parent if it is
 * available and WARN if it is missing for multi-parent clocks.
 *
 * For single-parent clocks without .get_parent, first check to see if the
 * .parents array exists, and if so use it to avoid an expensive tree
 * traversal.  If .parents does not exist then walk the tree with __clk_lookup.
 */
static struct clk *__clk_init_parent(struct clk *clk)
{
	struct clk *ret = NULL;
	u8 index;

	/* handle the trivial cases */

	if (!clk->num_parents)
		goto out;

	if (clk->num_parents == 1) {
		if (IS_ERR_OR_NULL(clk->parent))
			clk->parent = __clk_lookup(clk->parent_names[0]);
		ret = clk->parent;
		goto out;
	}

	if (!clk->ops->get_parent) {
		WARN(!clk->ops->get_parent,
			"%s: multi-parent clocks must implement .get_parent\n",
			__func__);
		goto out;
	};

	/*
	 * Do our best to cache parent clocks in clk->parents.  This prevents
	 * unnecessary and expensive calls to __clk_lookup.  We don't set
	 * clk->parent here; that is done by the calling function
	 */

	index = clk->ops->get_parent(clk->hw);

	if (!clk->parents)
		clk->parents =
			kcalloc(clk->num_parents, sizeof(struct clk *),
					GFP_KERNEL);

	ret = clk_get_parent_by_index(clk, index);

out:
	return ret;
}

void __clk_reparent(struct clk *clk, struct clk *new_parent)
{
	clk_reparent(clk, new_parent);
	__clk_recalc_accuracies(clk);
	__clk_recalc_rates(clk, POST_RATE_CHANGE);
}

/**
 * clk_set_parent - switch the parent of a mux clk
 * @clk: the mux clk whose input we are switching
 * @parent: the new input to clk
 *
 * Re-parent clk to use parent as its new input source.  If clk is in
 * prepared state, the clk will get enabled for the duration of this call. If
 * that's not acceptable for a specific clk (Eg: the consumer can't handle
 * that, the reparenting is glitchy in hardware, etc), use the
 * CLK_SET_PARENT_GATE flag to allow reparenting only when clk is unprepared.
 *
 * After successfully changing clk's parent clk_set_parent will update the
 * clk topology, sysfs topology and propagate rate recalculation via
 * __clk_recalc_rates.
 *
 * Returns 0 on success, -EERROR otherwise.
 */
int clk_set_parent(struct clk *clk, struct clk *parent)
{
	int ret = 0;
	int p_index = 0;
	unsigned long p_rate = 0;

	if (!clk)
		return 0;

	/* verify ops for for multi-parent clks */
	if ((clk->num_parents > 1) && (!clk->ops->set_parent))
		return -ENOSYS;

	/* prevent racing with updates to the clock topology */
	clk_prepare_lock();

	if (clk->parent == parent)
		goto out;

	/* check that we are allowed to re-parent if the clock is in use */
	if ((clk->flags & CLK_SET_PARENT_GATE) && clk->prepare_count) {
		ret = -EBUSY;
		goto out;
	}

	/* try finding the new parent index */
	if (parent) {
		p_index = clk_fetch_parent_index(clk, parent);
		p_rate = parent->rate;
		if (p_index < 0) {
			pr_debug("%s: clk %s can not be parent of clk %s\n",
					__func__, parent->name, clk->name);
			ret = p_index;
			goto out;
		}
	}

	/* propagate PRE_RATE_CHANGE notifications */
	ret = __clk_speculate_rates(clk, p_rate);

	/* abort if a driver objects */
	if (ret & NOTIFY_STOP_MASK)
		goto out;

	/* do the re-parent */
	ret = __clk_set_parent(clk, parent, p_index);

	/* propagate rate an accuracy recalculation accordingly */
	if (ret) {
		__clk_recalc_rates(clk, ABORT_RATE_CHANGE);
	} else {
		__clk_recalc_rates(clk, POST_RATE_CHANGE);
		__clk_recalc_accuracies(clk);
	}

out:
	clk_prepare_unlock();

	return ret;
}
EXPORT_SYMBOL_GPL(clk_set_parent);

/**
 * clk_set_phase - adjust the phase shift of a clock signal
 * @clk: clock signal source
 * @degrees: number of degrees the signal is shifted
 *
 * Shifts the phase of a clock signal by the specified
 * degrees. Returns 0 on success, -EERROR otherwise.
 *
 * This function makes no distinction about the input or reference
 * signal that we adjust the clock signal phase against. For example
 * phase locked-loop clock signal generators we may shift phase with
 * respect to feedback clock signal input, but for other cases the
 * clock phase may be shifted with respect to some other, unspecified
 * signal.
 *
 * Additionally the concept of phase shift does not propagate through
 * the clock tree hierarchy, which sets it apart from clock rates and
 * clock accuracy. A parent clock phase attribute does not have an
 * impact on the phase attribute of a child clock.
 */
int clk_set_phase(struct clk *clk, int degrees)
{
	int ret = 0;

	if (!clk)
		goto out;

	/* sanity check degrees */
	degrees %= 360;
	if (degrees < 0)
		degrees += 360;

	clk_prepare_lock();

	if (!clk->ops->set_phase)
		goto out_unlock;

	ret = clk->ops->set_phase(clk->hw, degrees);

	if (!ret)
		clk->phase = degrees;

out_unlock:
	clk_prepare_unlock();

out:
	return ret;
}

/**
 * clk_get_phase - return the phase shift of a clock signal
 * @clk: clock signal source
 *
 * Returns the phase shift of a clock node in degrees, otherwise returns
 * -EERROR.
 */
int clk_get_phase(struct clk *clk)
{
	int ret = 0;

	if (!clk)
		goto out;

	clk_prepare_lock();
	ret = clk->phase;
	clk_prepare_unlock();

out:
	return ret;
}

/**
 * __clk_init - initialize the data structures in a struct clk
 * @dev:	device initializing this clk, placeholder for now
 * @clk:	clk being initialized
 *
 * Initializes the lists in struct clk, queries the hardware for the
 * parent and rate and sets them both.
 */
int __clk_init(struct device *dev, struct clk *clk)
{
	int i, ret = 0;
	struct clk *orphan;
	struct hlist_node *tmp2;

	if (!clk)
		return -EINVAL;

	clk_prepare_lock();

	/* check to see if a clock with this name is already registered */
	if (__clk_lookup(clk->name)) {
		pr_debug("%s: clk %s already initialized\n",
				__func__, clk->name);
		ret = -EEXIST;
		goto out;
	}

	/* check that clk_ops are sane.  See Documentation/clk.txt */
	if (clk->ops->set_rate &&
	    !((clk->ops->round_rate || clk->ops->determine_rate) &&
	      clk->ops->recalc_rate)) {
		pr_warning("%s: %s must implement .round_rate or .determine_rate in addition to .recalc_rate\n",
				__func__, clk->name);
		ret = -EINVAL;
		goto out;
	}

	if (clk->ops->set_parent && !clk->ops->get_parent) {
		pr_warning("%s: %s must implement .get_parent & .set_parent\n",
				__func__, clk->name);
		ret = -EINVAL;
		goto out;
	}

	if (clk->ops->set_rate_and_parent &&
			!(clk->ops->set_parent && clk->ops->set_rate)) {
		pr_warn("%s: %s must implement .set_parent & .set_rate\n",
				__func__, clk->name);
		ret = -EINVAL;
		goto out;
	}

	/* throw a WARN if any entries in parent_names are NULL */
	for (i = 0; i < clk->num_parents; i++)
		WARN(!clk->parent_names[i],
				"%s: invalid NULL in %s's .parent_names\n",
				__func__, clk->name);

	/*
	 * Allocate an array of struct clk *'s to avoid unnecessary string
	 * look-ups of clk's possible parents.  This can fail for clocks passed
	 * in to clk_init during early boot; thus any access to clk->parents[]
	 * must always check for a NULL pointer and try to populate it if
	 * necessary.
	 *
	 * If clk->parents is not NULL we skip this entire block.  This allows
	 * for clock drivers to statically initialize clk->parents.
	 */
	if (clk->num_parents > 1 && !clk->parents) {
		clk->parents = kcalloc(clk->num_parents, sizeof(struct clk *),
					GFP_KERNEL);
		/*
		 * __clk_lookup returns NULL for parents that have not been
		 * clk_init'd; thus any access to clk->parents[] must check
		 * for a NULL pointer.  We can always perform lazy lookups for
		 * missing parents later on.
		 */
		if (clk->parents)
			for (i = 0; i < clk->num_parents; i++)
				clk->parents[i] =
					__clk_lookup(clk->parent_names[i]);
	}

	clk->parent = __clk_init_parent(clk);

	/*
	 * Populate clk->parent if parent has already been __clk_init'd.  If
	 * parent has not yet been __clk_init'd then place clk in the orphan
	 * list.  If clk has set the CLK_IS_ROOT flag then place it in the root
	 * clk list.
	 *
	 * Every time a new clk is clk_init'd then we walk the list of orphan
	 * clocks and re-parent any that are children of the clock currently
	 * being clk_init'd.
	 */
	if (clk->parent)
		hlist_add_head(&clk->child_node,
				&clk->parent->children);
	else if (clk->flags & CLK_IS_ROOT)
		hlist_add_head(&clk->child_node, &clk_root_list);
	else
		hlist_add_head(&clk->child_node, &clk_orphan_list);

	/*
	 * Set clk's accuracy.  The preferred method is to use
	 * .recalc_accuracy. For simple clocks and lazy developers the default
	 * fallback is to use the parent's accuracy.  If a clock doesn't have a
	 * parent (or is orphaned) then accuracy is set to zero (perfect
	 * clock).
	 */
	if (clk->ops->recalc_accuracy)
		clk->accuracy = clk->ops->recalc_accuracy(clk->hw,
					__clk_get_accuracy(clk->parent));
	else if (clk->parent)
		clk->accuracy = clk->parent->accuracy;
	else
		clk->accuracy = 0;

	/*
	 * Set clk's phase.
	 * Since a phase is by definition relative to its parent, just
	 * query the current clock phase, or just assume it's in phase.
	 */
	if (clk->ops->get_phase)
		clk->phase = clk->ops->get_phase(clk->hw);
	else
		clk->phase = 0;

	/*
	 * Set clk's rate.  The preferred method is to use .recalc_rate.  For
	 * simple clocks and lazy developers the default fallback is to use the
	 * parent's rate.  If a clock doesn't have a parent (or is orphaned)
	 * then rate is set to zero.
	 */
	if (clk->ops->recalc_rate)
		clk->rate = clk->ops->recalc_rate(clk->hw,
				__clk_get_rate(clk->parent));
	else if (clk->parent)
		clk->rate = clk->parent->rate;
	else
		clk->rate = 0;

	/*
	 * walk the list of orphan clocks and reparent any that are children of
	 * this clock
	 */
	hlist_for_each_entry_safe(orphan, tmp2, &clk_orphan_list, child_node) {
		if (orphan->num_parents && orphan->ops->get_parent) {
			i = orphan->ops->get_parent(orphan->hw);
			if (!strcmp(clk->name, orphan->parent_names[i]))
				__clk_reparent(orphan, clk);
			continue;
		}

		for (i = 0; i < orphan->num_parents; i++)
			if (!strcmp(clk->name, orphan->parent_names[i])) {
				__clk_reparent(orphan, clk);
				break;
			}
	 }

	/*
	 * optional platform-specific magic
	 *
	 * The .init callback is not used by any of the basic clock types, but
	 * exists for weird hardware that must perform initialization magic.
	 * Please consider other ways of solving initialization problems before
	 * using this callback, as its use is discouraged.
	 */
	if (clk->ops->init)
		clk->ops->init(clk->hw);

	kref_init(&clk->ref);
out:
	clk_prepare_unlock();

	if (!ret)
		clk_debug_register(clk);

	return ret;
}

/**
 * __clk_register - register a clock and return a cookie.
 *
 * Same as clk_register, except that the .clk field inside hw shall point to a
 * preallocated (generally statically allocated) struct clk. None of the fields
 * of the struct clk need to be initialized.
 *
 * The data pointed to by .init and .clk field shall NOT be marked as init
 * data.
 *
 * __clk_register is only exposed via clk-private.h and is intended for use with
 * very large numbers of clocks that need to be statically initialized.  It is
 * a layering violation to include clk-private.h from any code which implements
 * a clock's .ops; as such any statically initialized clock data MUST be in a
 * separate C file from the logic that implements its operations.  Returns 0
 * on success, otherwise an error code.
 */
struct clk *__clk_register(struct device *dev, struct clk_hw *hw)
{
	int ret;
	struct clk *clk;

	clk = hw->clk;
	clk->name = hw->init->name;
	clk->ops = hw->init->ops;
	clk->hw = hw;
	clk->flags = hw->init->flags;
	clk->parent_names = hw->init->parent_names;
	clk->num_parents = hw->init->num_parents;
	if (dev && dev->driver)
		clk->owner = dev->driver->owner;
	else
		clk->owner = NULL;

	ret = __clk_init(dev, clk);
	if (ret)
		return ERR_PTR(ret);

	return clk;
}
EXPORT_SYMBOL_GPL(__clk_register);

/**
 * clk_register - allocate a new clock, register it and return an opaque cookie
 * @dev: device that is registering this clock
 * @hw: link to hardware-specific clock data
 *
 * clk_register is the primary interface for populating the clock tree with new
 * clock nodes.  It returns a pointer to the newly allocated struct clk which
 * cannot be dereferenced by driver code but may be used in conjuction with the
 * rest of the clock API.  In the event of an error clk_register will return an
 * error code; drivers must test for an error code after calling clk_register.
 */
struct clk *clk_register(struct device *dev, struct clk_hw *hw)
{
	int i, ret;
	struct clk *clk;

	clk = kzalloc(sizeof(*clk), GFP_KERNEL);
	if (!clk) {
		pr_err("%s: could not allocate clk\n", __func__);
		ret = -ENOMEM;
		goto fail_out;
	}

	clk->name = kstrdup(hw->init->name, GFP_KERNEL);
	if (!clk->name) {
		pr_err("%s: could not allocate clk->name\n", __func__);
		ret = -ENOMEM;
		goto fail_name;
	}
	clk->ops = hw->init->ops;
	if (dev && dev->driver)
		clk->owner = dev->driver->owner;
	clk->hw = hw;
	clk->flags = hw->init->flags;
	clk->num_parents = hw->init->num_parents;
	hw->clk = clk;

	/* allocate local copy in case parent_names is __initdata */
	clk->parent_names = kcalloc(clk->num_parents, sizeof(char *),
					GFP_KERNEL);

	if (!clk->parent_names) {
		pr_err("%s: could not allocate clk->parent_names\n", __func__);
		ret = -ENOMEM;
		goto fail_parent_names;
	}


	/* copy each string name in case parent_names is __initdata */
	for (i = 0; i < clk->num_parents; i++) {
		clk->parent_names[i] = kstrdup(hw->init->parent_names[i],
						GFP_KERNEL);
		if (!clk->parent_names[i]) {
			pr_err("%s: could not copy parent_names\n", __func__);
			ret = -ENOMEM;
			goto fail_parent_names_copy;
		}
	}

	ret = __clk_init(dev, clk);
	if (!ret)
		return clk;

fail_parent_names_copy:
	while (--i >= 0)
		kfree(clk->parent_names[i]);
	kfree(clk->parent_names);
fail_parent_names:
	kfree(clk->name);
fail_name:
	kfree(clk);
fail_out:
	return ERR_PTR(ret);
}
EXPORT_SYMBOL_GPL(clk_register);

/*
 * Free memory allocated for a clock.
 * Caller must hold prepare_lock.
 */
static void __clk_release(struct kref *ref)
{
	struct clk *clk = container_of(ref, struct clk, ref);
	int i = clk->num_parents;

	kfree(clk->parents);
	while (--i >= 0)
		kfree(clk->parent_names[i]);

	kfree(clk->parent_names);
	kfree(clk->name);
	kfree(clk);
}

/*
 * Empty clk_ops for unregistered clocks. These are used temporarily
 * after clk_unregister() was called on a clock and until last clock
 * consumer calls clk_put() and the struct clk object is freed.
 */
static int clk_nodrv_prepare_enable(struct clk_hw *hw)
{
	return -ENXIO;
}

static void clk_nodrv_disable_unprepare(struct clk_hw *hw)
{
	WARN_ON_ONCE(1);
}

static int clk_nodrv_set_rate(struct clk_hw *hw, unsigned long rate,
					unsigned long parent_rate)
{
	return -ENXIO;
}

static int clk_nodrv_set_parent(struct clk_hw *hw, u8 index)
{
	return -ENXIO;
}

static const struct clk_ops clk_nodrv_ops = {
	.enable		= clk_nodrv_prepare_enable,
	.disable	= clk_nodrv_disable_unprepare,
	.prepare	= clk_nodrv_prepare_enable,
	.unprepare	= clk_nodrv_disable_unprepare,
	.set_rate	= clk_nodrv_set_rate,
	.set_parent	= clk_nodrv_set_parent,
};

/**
 * clk_unregister - unregister a currently registered clock
 * @clk: clock to unregister
 */
void clk_unregister(struct clk *clk)
{
	unsigned long flags;

	if (!clk || WARN_ON_ONCE(IS_ERR(clk)))
		return;

	clk_debug_unregister(clk);

	clk_prepare_lock();

	if (clk->ops == &clk_nodrv_ops) {
		pr_err("%s: unregistered clock: %s\n", __func__, clk->name);
		return;
	}
	/*
	 * Assign empty clock ops for consumers that might still hold
	 * a reference to this clock.
	 */
	flags = clk_enable_lock();
	clk->ops = &clk_nodrv_ops;
	clk_enable_unlock(flags);

	if (!hlist_empty(&clk->children)) {
		struct clk *child;
		struct hlist_node *t;

		/* Reparent all children to the orphan list. */
		hlist_for_each_entry_safe(child, t, &clk->children, child_node)
			clk_set_parent(child, NULL);
	}

	hlist_del_init(&clk->child_node);

	if (clk->prepare_count)
		pr_warn("%s: unregistering prepared clock: %s\n",
					__func__, clk->name);
	kref_put(&clk->ref, __clk_release);

	clk_prepare_unlock();
}
EXPORT_SYMBOL_GPL(clk_unregister);

static void devm_clk_release(struct device *dev, void *res)
{
	clk_unregister(*(struct clk **)res);
}

/**
 * devm_clk_register - resource managed clk_register()
 * @dev: device that is registering this clock
 * @hw: link to hardware-specific clock data
 *
 * Managed clk_register(). Clocks returned from this function are
 * automatically clk_unregister()ed on driver detach. See clk_register() for
 * more information.
 */
struct clk *devm_clk_register(struct device *dev, struct clk_hw *hw)
{
	struct clk *clk;
	struct clk **clkp;

	clkp = devres_alloc(devm_clk_release, sizeof(*clkp), GFP_KERNEL);
	if (!clkp)
		return ERR_PTR(-ENOMEM);

	clk = clk_register(dev, hw);
	if (!IS_ERR(clk)) {
		*clkp = clk;
		devres_add(dev, clkp);
	} else {
		devres_free(clkp);
	}

	return clk;
}
EXPORT_SYMBOL_GPL(devm_clk_register);

static int devm_clk_match(struct device *dev, void *res, void *data)
{
	struct clk *c = res;
	if (WARN_ON(!c))
		return 0;
	return c == data;
}

/**
 * devm_clk_unregister - resource managed clk_unregister()
 * @clk: clock to unregister
 *
 * Deallocate a clock allocated with devm_clk_register(). Normally
 * this function will not need to be called and the resource management
 * code will ensure that the resource is freed.
 */
void devm_clk_unregister(struct device *dev, struct clk *clk)
{
	WARN_ON(devres_release(dev, devm_clk_release, devm_clk_match, clk));
}
EXPORT_SYMBOL_GPL(devm_clk_unregister);

/*
 * clkdev helpers
 */
int __clk_get(struct clk *clk)
{
	if (clk) {
		if (!try_module_get(clk->owner))
			return 0;

		kref_get(&clk->ref);
	}
	return 1;
}

void __clk_put(struct clk *clk)
{
	struct module *owner;

	if (!clk || WARN_ON_ONCE(IS_ERR(clk)))
		return;

	clk_prepare_lock();
	owner = clk->owner;
	kref_put(&clk->ref, __clk_release);
	clk_prepare_unlock();

	module_put(owner);
}

/***        clk rate change notifiers        ***/

/**
 * clk_notifier_register - add a clk rate change notifier
 * @clk: struct clk * to watch
 * @nb: struct notifier_block * with callback info
 *
 * Request notification when clk's rate changes.  This uses an SRCU
 * notifier because we want it to block and notifier unregistrations are
 * uncommon.  The callbacks associated with the notifier must not
 * re-enter into the clk framework by calling any top-level clk APIs;
 * this will cause a nested prepare_lock mutex.
 *
 * In all notification cases cases (pre, post and abort rate change) the
 * original clock rate is passed to the callback via struct
 * clk_notifier_data.old_rate and the new frequency is passed via struct
 * clk_notifier_data.new_rate.
 *
 * clk_notifier_register() must be called from non-atomic context.
 * Returns -EINVAL if called with null arguments, -ENOMEM upon
 * allocation failure; otherwise, passes along the return value of
 * srcu_notifier_chain_register().
 */
int clk_notifier_register(struct clk *clk, struct notifier_block *nb)
{
	struct clk_notifier *cn;
	int ret = -ENOMEM;

	if (!clk || !nb)
		return -EINVAL;

	clk_prepare_lock();

	/* search the list of notifiers for this clk */
	list_for_each_entry(cn, &clk_notifier_list, node)
		if (cn->clk == clk)
			break;

	/* if clk wasn't in the notifier list, allocate new clk_notifier */
	if (cn->clk != clk) {
		cn = kzalloc(sizeof(struct clk_notifier), GFP_KERNEL);
		if (!cn)
			goto out;

		cn->clk = clk;
		srcu_init_notifier_head(&cn->notifier_head);

		list_add(&cn->node, &clk_notifier_list);
	}

	ret = srcu_notifier_chain_register(&cn->notifier_head, nb);

	clk->notifier_count++;

out:
	clk_prepare_unlock();

	return ret;
}
EXPORT_SYMBOL_GPL(clk_notifier_register);

/**
 * clk_notifier_unregister - remove a clk rate change notifier
 * @clk: struct clk *
 * @nb: struct notifier_block * with callback info
 *
 * Request no further notification for changes to 'clk' and frees memory
 * allocated in clk_notifier_register.
 *
 * Returns -EINVAL if called with null arguments; otherwise, passes
 * along the return value of srcu_notifier_chain_unregister().
 */
int clk_notifier_unregister(struct clk *clk, struct notifier_block *nb)
{
	struct clk_notifier *cn = NULL;
	int ret = -EINVAL;

	if (!clk || !nb)
		return -EINVAL;

	clk_prepare_lock();

	list_for_each_entry(cn, &clk_notifier_list, node)
		if (cn->clk == clk)
			break;

	if (cn->clk == clk) {
		ret = srcu_notifier_chain_unregister(&cn->notifier_head, nb);

		clk->notifier_count--;

		/* XXX the notifier code should handle this better */
		if (!cn->notifier_head.head) {
			srcu_cleanup_notifier_head(&cn->notifier_head);
			list_del(&cn->node);
			kfree(cn);
		}

	} else {
		ret = -ENOENT;
	}

	clk_prepare_unlock();

	return ret;
}
EXPORT_SYMBOL_GPL(clk_notifier_unregister);

#ifdef CONFIG_OF
/**
 * struct of_clk_provider - Clock provider registration structure
 * @link: Entry in global list of clock providers
 * @node: Pointer to device tree node of clock provider
 * @get: Get clock callback.  Returns NULL or a struct clk for the
 *       given clock specifier
 * @data: context pointer to be passed into @get callback
 */
struct of_clk_provider {
	struct list_head link;

	struct device_node *node;
	struct clk *(*get)(struct of_phandle_args *clkspec, void *data);
	void *data;
};

static const struct of_device_id __clk_of_table_sentinel
	__used __section(__clk_of_table_end);

static LIST_HEAD(of_clk_providers);
static DEFINE_MUTEX(of_clk_mutex);

/* of_clk_provider list locking helpers */
void of_clk_lock(void)
{
	mutex_lock(&of_clk_mutex);
}

void of_clk_unlock(void)
{
	mutex_unlock(&of_clk_mutex);
}

struct clk *of_clk_src_simple_get(struct of_phandle_args *clkspec,
				     void *data)
{
	return data;
}
EXPORT_SYMBOL_GPL(of_clk_src_simple_get);

struct clk *of_clk_src_onecell_get(struct of_phandle_args *clkspec, void *data)
{
	struct clk_onecell_data *clk_data = data;
	unsigned int idx = clkspec->args[0];

	if (idx >= clk_data->clk_num) {
		pr_err("%s: invalid clock index %d\n", __func__, idx);
		return ERR_PTR(-EINVAL);
	}

	return clk_data->clks[idx];
}
EXPORT_SYMBOL_GPL(of_clk_src_onecell_get);

/**
 * of_clk_add_provider() - Register a clock provider for a node
 * @np: Device node pointer associated with clock provider
 * @clk_src_get: callback for decoding clock
 * @data: context pointer for @clk_src_get callback.
 */
int of_clk_add_provider(struct device_node *np,
			struct clk *(*clk_src_get)(struct of_phandle_args *clkspec,
						   void *data),
			void *data)
{
	struct of_clk_provider *cp;
	int ret;

	cp = kzalloc(sizeof(struct of_clk_provider), GFP_KERNEL);
	if (!cp)
		return -ENOMEM;

	cp->node = of_node_get(np);
	cp->data = data;
	cp->get = clk_src_get;

	mutex_lock(&of_clk_mutex);
	list_add(&cp->link, &of_clk_providers);
	mutex_unlock(&of_clk_mutex);
	pr_debug("Added clock from %s\n", np->full_name);

	ret = of_clk_set_defaults(np, true);
	if (ret < 0)
		of_clk_del_provider(np);

	return ret;
}
EXPORT_SYMBOL_GPL(of_clk_add_provider);

/**
 * of_clk_del_provider() - Remove a previously registered clock provider
 * @np: Device node pointer associated with clock provider
 */
void of_clk_del_provider(struct device_node *np)
{
	struct of_clk_provider *cp;

	mutex_lock(&of_clk_mutex);
	list_for_each_entry(cp, &of_clk_providers, link) {
		if (cp->node == np) {
			list_del(&cp->link);
			of_node_put(cp->node);
			kfree(cp);
			break;
		}
	}
	mutex_unlock(&of_clk_mutex);
}
EXPORT_SYMBOL_GPL(of_clk_del_provider);

struct clk *__of_clk_get_from_provider(struct of_phandle_args *clkspec)
{
	struct of_clk_provider *provider;
	struct clk *clk = ERR_PTR(-EPROBE_DEFER);

	/* Check if we have such a provider in our array */
	list_for_each_entry(provider, &of_clk_providers, link) {
		if (provider->node == clkspec->np)
			clk = provider->get(clkspec, provider->data);
		if (!IS_ERR(clk))
			break;
	}

	return clk;
}

struct clk *of_clk_get_from_provider(struct of_phandle_args *clkspec)
{
	struct clk *clk;

	mutex_lock(&of_clk_mutex);
	clk = __of_clk_get_from_provider(clkspec);
	mutex_unlock(&of_clk_mutex);

	return clk;
}

int of_clk_get_parent_count(struct device_node *np)
{
	return of_count_phandle_with_args(np, "clocks", "#clock-cells");
}
EXPORT_SYMBOL_GPL(of_clk_get_parent_count);

const char *of_clk_get_parent_name(struct device_node *np, int index)
{
	struct of_phandle_args clkspec;
	struct property *prop;
	const char *clk_name;
	const __be32 *vp;
	u32 pv;
	int rc;
	int count;

	if (index < 0)
		return NULL;

	rc = of_parse_phandle_with_args(np, "clocks", "#clock-cells", index,
					&clkspec);
	if (rc)
		return NULL;

	index = clkspec.args_count ? clkspec.args[0] : 0;
	count = 0;

	/* if there is an indices property, use it to transfer the index
	 * specified into an array offset for the clock-output-names property.
	 */
	of_property_for_each_u32(clkspec.np, "clock-indices", prop, vp, pv) {
		if (index == pv) {
			index = count;
			break;
		}
		count++;
	}

	if (of_property_read_string_index(clkspec.np, "clock-output-names",
					  index,
					  &clk_name) < 0)
		clk_name = clkspec.np->name;

	of_node_put(clkspec.np);
	return clk_name;
}
EXPORT_SYMBOL_GPL(of_clk_get_parent_name);

struct clock_provider {
	of_clk_init_cb_t clk_init_cb;
	struct device_node *np;
	struct list_head node;
};

static LIST_HEAD(clk_provider_list);

/*
 * This function looks for a parent clock. If there is one, then it
 * checks that the provider for this parent clock was initialized, in
 * this case the parent clock will be ready.
 */
static int parent_ready(struct device_node *np)
{
	int i = 0;

	while (true) {
		struct clk *clk = of_clk_get(np, i);

		/* this parent is ready we can check the next one */
		if (!IS_ERR(clk)) {
			clk_put(clk);
			i++;
			continue;
		}

		/* at least one parent is not ready, we exit now */
		if (PTR_ERR(clk) == -EPROBE_DEFER)
			return 0;

		/*
		 * Here we make assumption that the device tree is
		 * written correctly. So an error means that there is
		 * no more parent. As we didn't exit yet, then the
		 * previous parent are ready. If there is no clock
		 * parent, no need to wait for them, then we can
		 * consider their absence as being ready
		 */
		return 1;
	}
}

/**
 * of_clk_init() - Scan and init clock providers from the DT
 * @matches: array of compatible values and init functions for providers.
 *
 * This function scans the device tree for matching clock providers
 * and calls their initialization functions. It also does it by trying
 * to follow the dependencies.
 */
void __init of_clk_init(const struct of_device_id *matches)
{
	const struct of_device_id *match;
	struct device_node *np;
	struct clock_provider *clk_provider, *next;
	bool is_init_done;
	bool force = false;

	if (!matches)
		matches = &__clk_of_table;

	/* First prepare the list of the clocks providers */
	for_each_matching_node_and_match(np, matches, &match) {
		struct clock_provider *parent =
			kzalloc(sizeof(struct clock_provider),	GFP_KERNEL);

		parent->clk_init_cb = match->data;
		parent->np = np;
		list_add_tail(&parent->node, &clk_provider_list);
	}

	while (!list_empty(&clk_provider_list)) {
		is_init_done = false;
		list_for_each_entry_safe(clk_provider, next,
					&clk_provider_list, node) {
			if (force || parent_ready(clk_provider->np)) {

				clk_provider->clk_init_cb(clk_provider->np);
				of_clk_set_defaults(clk_provider->np, true);

				list_del(&clk_provider->node);
				kfree(clk_provider);
				is_init_done = true;
			}
		}

		/*
		 * We didn't manage to initialize any of the
		 * remaining providers during the last loop, so now we
		 * initialize all the remaining ones unconditionally
		 * in case the clock parent was not mandatory
		 */
		if (!is_init_done)
			force = true;
	}
}
#endif