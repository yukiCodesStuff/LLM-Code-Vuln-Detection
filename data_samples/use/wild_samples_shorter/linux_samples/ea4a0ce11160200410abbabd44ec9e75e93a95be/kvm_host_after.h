#ifndef __KVM_HOST_H
#define __KVM_HOST_H

#if IS_ENABLED(CONFIG_KVM)

/*
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */
}

#endif /* CONFIG_HAVE_KVM_CPU_RELAX_INTERCEPT */
#else
static inline void __guest_enter(void) { return; }
static inline void __guest_exit(void) { return; }
#endif /* IS_ENABLED(CONFIG_KVM) */
#endif