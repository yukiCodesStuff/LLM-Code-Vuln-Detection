static unsigned char * put16(unsigned char *cp, unsigned short x);
static unsigned short pull16(unsigned char **cpp);

/* Allocate compression data structure
 *	slots must be in range 0 to 255 (zero meaning no compression)
 * Returns pointer to structure or ERR_PTR() on error.
 */
struct slcompress *
slhc_init(int rslots, int tslots)
{
	register struct cstate *ts;
	struct slcompress *comp;

	if (rslots < 0 || rslots > 255 || tslots < 0 || tslots > 255)
		return ERR_PTR(-EINVAL);

	comp = kzalloc(sizeof(struct slcompress), GFP_KERNEL);
	if (! comp)
		goto out_fail;

	if (rslots > 0) {
		size_t rsize = rslots * sizeof(struct cstate);
		comp->rstate = kzalloc(rsize, GFP_KERNEL);
		if (! comp->rstate)
			goto out_free;
		comp->rslot_limit = rslots - 1;
	}

	if (tslots > 0) {
		size_t tsize = tslots * sizeof(struct cstate);
		comp->tstate = kzalloc(tsize, GFP_KERNEL);
		if (! comp->tstate)
			goto out_free2;
out_free:
	kfree(comp);
out_fail:
	return ERR_PTR(-ENOMEM);
}


/* Free a compression data structure */