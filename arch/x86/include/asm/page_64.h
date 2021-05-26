/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_X86_PAGE_64_H
#define _ASM_X86_PAGE_64_H

#include <asm/page_64_types.h>

#ifndef __ASSEMBLY__
#include <asm/alternative.h>

/* duplicated to the one in bootmem.h */
extern unsigned long max_pfn;
extern unsigned long phys_base;

extern unsigned long page_offset_base;
extern unsigned long vmalloc_base;
extern unsigned long vmemmap_base;

static inline unsigned long __phys_addr_nodebug(unsigned long x)
{
	unsigned long y = x - __START_KERNEL_map;

	/* use the carry flag to determine if x was < __START_KERNEL_map */
	x = y + ((x > y) ? phys_base : (__START_KERNEL_map - PAGE_OFFSET));

	return x;
}

#ifdef CONFIG_DEBUG_VIRTUAL
extern unsigned long __phys_addr(unsigned long);
extern unsigned long __phys_addr_symbol(unsigned long);
#else
#define __phys_addr(x)		__phys_addr_nodebug(x)
#define __phys_addr_symbol(x) \
	((unsigned long)(x) - __START_KERNEL_map + phys_base)
#endif

#define __phys_reloc_hide(x)	(x)

#ifdef CONFIG_FLATMEM
#define pfn_valid(pfn)          ((pfn) < max_pfn)
#endif

void clear_page_orig(void *page);
void clear_page_rep(void *page);
void clear_page_erms(void *page);

static inline void clear_page(void *page)
{
	alternative_call_2(clear_page_orig,
			   clear_page_rep, X86_FEATURE_REP_GOOD,
			   clear_page_erms, X86_FEATURE_ERMS,
			   "=D" (page),
			   "0" (page)
			   : "cc", "memory", "rax", "rcx");
}

void copy_page(void *to, void *from);

#define __untagged_addr(addr, n)	\
	((__force __typeof__(addr))sign_extend64((__force u64)(addr), n))

#define untagged_addr(addr)	({					\
	u64 __addr = (__force u64)(addr);				\
	if (__addr >> 63 == 0) {					\
		if (test_thread_flag(TIF_LAM_U57))			\
			__addr &= __untagged_addr(__addr, 56);		\
		else if (test_thread_flag(TIF_LAM_U48))			\
			__addr &= __untagged_addr(__addr, 47);		\
	}								\
	(__force __typeof__(addr))__addr;				\
})

#define untagged_ptr(ptr)	({					\
	u64 __ptrval = (__force u64)(ptr);				\
	__ptrval = untagged_addr(__ptrval);				\
	(__force __typeof__(*(ptr)) *)__ptrval;				\
})
#endif	/* !__ASSEMBLY__ */

#ifdef CONFIG_X86_VSYSCALL_EMULATION
# define __HAVE_ARCH_GATE_AREA 1
#endif

#endif /* _ASM_X86_PAGE_64_H */
