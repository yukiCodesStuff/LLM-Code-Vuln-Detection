{
	return atomic_read(&r->refs);
}

#ifdef CONFIG_REFCOUNT_FULL
extern __must_check bool refcount_add_not_zero(unsigned int i, refcount_t *r);
extern void refcount_add(unsigned int i, refcount_t *r);

extern __must_check bool refcount_inc_not_zero(refcount_t *r);
extern void refcount_inc(refcount_t *r);

extern __must_check bool refcount_sub_and_test(unsigned int i, refcount_t *r);

extern __must_check bool refcount_dec_and_test(refcount_t *r);
extern void refcount_dec(refcount_t *r);
#else
static inline __must_check bool refcount_add_not_zero(unsigned int i, refcount_t *r)
{
	return atomic_add_unless(&r->refs, i, 0);
}
{
	atomic_dec(&r->refs);
}
#endif /* CONFIG_REFCOUNT_FULL */

extern __must_check bool refcount_dec_if_one(refcount_t *r);
extern __must_check bool refcount_dec_not_one(refcount_t *r);
extern __must_check bool refcount_dec_and_mutex_lock(refcount_t *r, struct mutex *lock);
extern __must_check bool refcount_dec_and_lock(refcount_t *r, spinlock_t *lock);

#endif /* _LINUX_REFCOUNT_H */