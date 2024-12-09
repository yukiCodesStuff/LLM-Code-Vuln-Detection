	unsigned int      split_at_calls : 1;
	unsigned int      split_at_recv : 1;
	unsigned int      dynamic : 1;        /* accesses varables by name   */
	unsigned int      vararg : 1;         /* uses func_get_args()        */
} zend_cfg;

/* Build Flags */
#define ZEND_RT_CONSTANTS              (1<<31)