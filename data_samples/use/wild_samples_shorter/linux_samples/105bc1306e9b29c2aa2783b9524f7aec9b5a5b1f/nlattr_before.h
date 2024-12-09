	     nla_ok(pos, rem); \
	     pos = nla_next(pos, &(rem)))

int nla_dump_errormsg(struct nlmsghdr *nlh);

#endif /* __NLATTR_H */