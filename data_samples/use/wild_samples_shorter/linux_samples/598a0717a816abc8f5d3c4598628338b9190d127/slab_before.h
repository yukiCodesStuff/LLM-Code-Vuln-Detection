static inline bool slab_equal_or_root(struct kmem_cache *s,
				      struct kmem_cache *p)
{
	return true;
}

static inline const char *cache_name(struct kmem_cache *s)
{
	 * will also be a constant.
	 */
	if (!memcg_kmem_enabled() &&
	    !unlikely(s->flags & SLAB_CONSISTENCY_CHECKS))
		return s;

	page = virt_to_head_page(x);
	cachep = page->slab_cache;
	if (slab_equal_or_root(cachep, s))
		return cachep;

	pr_err("%s: Wrong slab cache. %s but object is from %s\n",
	       __func__, s->name, cachep->name);
	WARN_ON_ONCE(1);
	return s;
}

static inline size_t slab_ksize(const struct kmem_cache *s)
{