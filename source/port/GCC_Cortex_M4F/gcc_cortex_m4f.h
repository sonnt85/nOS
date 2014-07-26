/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IAR_ARM7M_H
#define IAR_ARM7M_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif
  
typedef uint32_t            stack_t;

#define NOS_PORT_SCHED_USE_32_BITS
#define NOS_PORT_HAVE_CLZ

/* __NVIC_PRIO_BITS defined from CMSIS if used */
#if defined(__NVIC_PRIO_BITS)
 #define NOS_NVIC_PRIO_BITS                 __NVIC_PRIO_BITS
#else
 #define NOS_NVIC_PRIO_BITS                 4
#endif

#ifndef NOS_CONFIG_ISR_STACK_SIZE
 #define NOS_CONFIG_ISR_STACK_SIZE            128
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is not defined (default to 128)."
 #endif
#else
 #if NOS_CONFIG_ISR_STACK_SIZE == 0
  #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value."
 #endif
#endif

#ifndef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #define NOS_CONFIG_MAX_UNSAFE_ISR_PRIO       5
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is not defined (default to 5)."
 #endif
#else
 #if NOS_CONFIG_MAX_UNSAFE_ISR_PRIO == 0 || NOS_CONFIG_MAX_UNSAFE_ISR_PRIO >= (NOS_NVIC_PRIO_BITS*NOS_NVIC_PRIO_BITS)
  #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is set to invalid value."
 #endif
#endif

#define NOS_PORT_MAX_UNSAFE_BASEPRI         (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO << (8 - NOS_NVIC_PRIO_BITS))

__attribute__( ( always_inline ) ) static inline uint32_t nOS_PortCLZ(uint32_t n)
{
	uint32_t	r;
	__asm volatile (
		"CLZ	%0,	%1"
		: "=r" (r)
		: "r" (n)
	);
	return r;
}
  
#define nOS_CriticalEnter()                                     				\
{																				\
    uint32_t _basepri;					                        				\
    __asm volatile (															\
		"MRS	%0,			BASEPRI			\n"									\
		"MOV	R0,			%1				\n"									\
		"MSR	BASEPRI,	R0				\n"									\
		"DSB								\n"									\
		"ISB								\n"									\
		: "=r" (_basepri)														\
		: "I" (NOS_PORT_MAX_UNSAFE_BASEPRI)										\
	)
      
#define nOS_CriticalLeave()                                                     \
	__asm volatile (															\
		"MOV	R0,			%0				\n"									\
		"MSR	BASEPRI,	R0				\n"									\
		"DSB								\n"									\
		"ISB								\n"									\
		:																		\
		: "r"(_basepri)															\
	);																			\
}

/*
 * Request a context switch and enable interrupts to allow PendSV interrupt.
 */
#define nOS_ContextSwitch()                                                     \
    *(volatile uint32_t *)0xe000ed04 = 0x10000000;                              \
    __asm volatile (															\
		"MOV	R0,			#0				\n"									\
    	"MSR	BASEPRI,	R0				\n"									\
		"DSB								\n"									\
		"ISB								\n"									\
		"NOP								\n"									\
		"MOV	R0,			%0				\n"									\
		"MSR	BASEPRI,	R0				\n"									\
		"DSB								\n"									\
		"ISB								\n"									\
		:																		\
		: "I" (NOS_PORT_MAX_UNSAFE_BASEPRI)										\
	)


void    nOS_IsrEnter    (void);
void    nOS_IsrLeave    (void);
      
#define NOS_ISR(func)                                                           \
void func##_ISR(void);                                                          \
void func(void)                                                                 \
{                                                                               \
    nOS_IsrEnter();                                                             \
    func##_ISR();                                                               \
    nOS_IsrLeave();                                                             \
}                                                                               \
void func##_ISR(void)

#if defined(NOS_PRIVATE)
void        nOS_PortInit        (void);
#endif  /* NOS_PRIVATE */

void        nOS_ContextInit     (nOS_Thread *thread, stack_t *stack, size_t ssize, void(*func)(void*), void *arg);

#if defined(__cplusplus)
}
#endif

#endif /* GCC_AVR5_H_ */