
#define ELF_PLATFORM  (NULL)

#define SET_PERSONALITY(ex) \
	set_personality(PER_LINUX | (current->personality & (~PER_MASK)))

#define STACK_RND_MASK (0)

#ifdef CONFIG_METAG_USER_TCM
