 * immediately unlinked.
 */
struct key_type key_type_dead = {
	.name = "dead",
};

/*
 * Schedule a garbage collection run.