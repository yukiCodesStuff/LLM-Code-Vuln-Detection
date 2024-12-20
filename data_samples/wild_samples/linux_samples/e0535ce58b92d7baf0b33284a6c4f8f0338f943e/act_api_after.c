	list_for_each_entry(a, actions, list) {
		nest = nla_nest_start(skb, a->order);
		if (nest == NULL)
			goto nla_put_failure;
		err = tcf_action_dump_1(skb, a, bind, ref);
		if (err < 0)
			goto errout;
		nla_nest_end(skb, nest);
	}

	return 0;

nla_put_failure:
	err = -EINVAL;
errout:
	nla_nest_cancel(skb, nest);
	return err;
}

static struct tc_cookie *nla_memdup_cookie(struct nlattr **tb)
{
	struct tc_cookie *c = kzalloc(sizeof(*c), GFP_KERNEL);
	if (!c)
		return NULL;

	c->data = nla_memdup(tb[TCA_ACT_COOKIE], GFP_KERNEL);
	if (!c->data) {
		kfree(c);
		return NULL;
	}
{
	struct tc_action *a;
	struct tc_action_ops *a_o;
	struct tc_cookie *cookie = NULL;
	char act_name[IFNAMSIZ];
	struct nlattr *tb[TCA_ACT_MAX + 1];
	struct nlattr *kind;
	int err;

	if (name == NULL) {
		err = nla_parse_nested(tb, TCA_ACT_MAX, nla, NULL);
		if (err < 0)
			goto err_out;
		err = -EINVAL;
		kind = tb[TCA_ACT_KIND];
		if (kind == NULL)
			goto err_out;
		if (nla_strlcpy(act_name, kind, IFNAMSIZ) >= IFNAMSIZ)
			goto err_out;
		if (tb[TCA_ACT_COOKIE]) {
			int cklen = nla_len(tb[TCA_ACT_COOKIE]);

			if (cklen > TC_COOKIE_MAX_SIZE)
				goto err_out;

			cookie = nla_memdup_cookie(tb);
			if (!cookie) {
				err = -ENOMEM;
				goto err_out;
			}
		}
	} else {
		err = -EINVAL;
		if (strlcpy(act_name, name, IFNAMSIZ) >= IFNAMSIZ)
			goto err_out;
	}

	a_o = tc_lookup_action_n(act_name);
	if (a_o == NULL) {
#ifdef CONFIG_MODULES
		rtnl_unlock();
		request_module("act_%s", act_name);
		rtnl_lock();

		a_o = tc_lookup_action_n(act_name);

		/* We dropped the RTNL semaphore in order to
		 * perform the module load.  So, even if we
		 * succeeded in loading the module we have to
		 * tell the caller to replay the request.  We
		 * indicate this using -EAGAIN.
		 */
		if (a_o != NULL) {
			err = -EAGAIN;
			goto err_mod;
		}
#endif
		err = -ENOENT;
		goto err_out;
	}

	/* backward compatibility for policer */
	if (name == NULL)
		err = a_o->init(net, tb[TCA_ACT_OPTIONS], est, &a, ovr, bind);
	else
		err = a_o->init(net, nla, est, &a, ovr, bind);
	if (err < 0)
		goto err_mod;

	if (name == NULL && tb[TCA_ACT_COOKIE]) {
		if (a->act_cookie) {
			kfree(a->act_cookie->data);
			kfree(a->act_cookie);
		}
		a->act_cookie = cookie;
	}

	/* module count goes up only when brand new policy is created
	 * if it exists and is only bound to in a_o->init() then
	 * ACT_P_CREATED is not returned (a zero is).
	 */
	if (err != ACT_P_CREATED)
		module_put(a_o->owner);

	return a;

err_mod:
	module_put(a_o->owner);
err_out:
	if (cookie) {
		kfree(cookie->data);
		kfree(cookie);
	}
	return ERR_PTR(err);
}
	if (name == NULL) {
		err = nla_parse_nested(tb, TCA_ACT_MAX, nla, NULL);
		if (err < 0)
			goto err_out;
		err = -EINVAL;
		kind = tb[TCA_ACT_KIND];
		if (kind == NULL)
			goto err_out;
		if (nla_strlcpy(act_name, kind, IFNAMSIZ) >= IFNAMSIZ)
			goto err_out;
		if (tb[TCA_ACT_COOKIE]) {
			int cklen = nla_len(tb[TCA_ACT_COOKIE]);

			if (cklen > TC_COOKIE_MAX_SIZE)
				goto err_out;

			cookie = nla_memdup_cookie(tb);
			if (!cookie) {
				err = -ENOMEM;
				goto err_out;
			}
		}
		if (a_o != NULL) {
			err = -EAGAIN;
			goto err_mod;
		}
#endif
		err = -ENOENT;
		goto err_out;
	}

	/* backward compatibility for policer */
	if (name == NULL)
		err = a_o->init(net, tb[TCA_ACT_OPTIONS], est, &a, ovr, bind);
	else
		err = a_o->init(net, nla, est, &a, ovr, bind);
	if (err < 0)
		goto err_mod;

	if (name == NULL && tb[TCA_ACT_COOKIE]) {
		if (a->act_cookie) {
			kfree(a->act_cookie->data);
			kfree(a->act_cookie);
		}
		a->act_cookie = cookie;
	}
		if (a->act_cookie) {
			kfree(a->act_cookie->data);
			kfree(a->act_cookie);
		}
		a->act_cookie = cookie;
	}

	/* module count goes up only when brand new policy is created
	 * if it exists and is only bound to in a_o->init() then
	 * ACT_P_CREATED is not returned (a zero is).
	 */
	if (err != ACT_P_CREATED)
		module_put(a_o->owner);

	return a;

err_mod:
	module_put(a_o->owner);
err_out:
	if (cookie) {
		kfree(cookie->data);
		kfree(cookie);
	}