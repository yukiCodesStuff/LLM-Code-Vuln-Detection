#define VMX_FEATURE_USR_WAIT_PAUSE	( 2*32+ 26) /* Enable TPAUSE, UMONITOR, UMWAIT in guest */
#define VMX_FEATURE_ENCLV_EXITING	( 2*32+ 28) /* "" VM-Exit on ENCLV (leaf dependent) */
#define VMX_FEATURE_BUS_LOCK_DETECTION	( 2*32+ 30) /* "" VM-Exit when bus lock caused */
#define VMX_FEATURE_NOTIFY_VM_EXITING	( 2*32+ 31) /* VM-Exit when no event windows after notify window */

/* Tertiary Processor-Based VM-Execution Controls, word 3 */
#define VMX_FEATURE_IPI_VIRT		( 3*32+  4) /* Enable IPI virtualization */
#endif /* _ASM_X86_VMXFEATURES_H */