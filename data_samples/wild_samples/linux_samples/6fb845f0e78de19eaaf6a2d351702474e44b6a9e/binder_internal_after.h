{
	return false;
}
#endif

#ifdef CONFIG_ANDROID_BINDERFS
extern int __init init_binderfs(void);
#else
static inline int __init init_binderfs(void)
{
	return 0;
}