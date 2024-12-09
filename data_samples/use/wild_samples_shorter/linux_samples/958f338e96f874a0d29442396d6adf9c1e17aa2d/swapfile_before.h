extern struct plist_head swap_active_head;
extern struct swap_info_struct *swap_info[];
extern int try_to_unuse(unsigned int, bool, unsigned long);

#endif /* _LINUX_SWAPFILE_H */