/****************************************************************************
 * arch/arm/src/common/arm_internal.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_COMMON_ARM_INTERNAL_H
#define __ARCH_ARM_SRC_COMMON_ARM_INTERNAL_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#ifndef __ASSEMBLY__
#  include <nuttx/compiler.h>
#  include <sys/types.h>
#  include <stdint.h>
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Determine which (if any) console driver to use.  If a console is enabled
 * and no other console device is specified, then a serial console is
 * assumed.
 */

#ifndef CONFIG_DEV_CONSOLE
#  undef  USE_SERIALDRIVER
#  undef  USE_EARLYSERIALINIT
#else
#  if defined(CONFIG_ARM_LWL_CONSOLE)
#    undef  USE_SERIALDRIVER
#    undef  USE_EARLYSERIALINIT
#  elif defined(CONFIG_CONSOLE_SYSLOG)
#    undef  USE_SERIALDRIVER
#    undef  USE_EARLYSERIALINIT
#  else
#    define USE_SERIALDRIVER 1
#    define USE_EARLYSERIALINIT 1
#  endif
#endif

/* If some other device is used as the console, then the serial driver may
 * still be needed.  Let's assume that if the upper half serial driver is
 * built, then the lower half will also be needed.  There is no need for
 * the early serial initialization in this case.
 */

#if !defined(USE_SERIALDRIVER) && defined(CONFIG_STANDARD_SERIAL)
#  define USE_SERIALDRIVER 1
#endif

/* Check if an interrupt stack size is configured */

#ifndef CONFIG_ARCH_INTERRUPTSTACK
#  define CONFIG_ARCH_INTERRUPTSTACK 0
#endif

/* Macros to handle saving and restoring interrupt state.  In the current ARM
 * model, the state is always copied to and from the stack and TCB.  In the
 * Cortex-M0/3 model, the state is copied from the stack to the TCB, but only
 * a referenced is passed to get the state from the TCB.  Cortex-M4 is the
 * same, but may have additional complexity for floating point support in
 * some configurations.
 */

#if defined(CONFIG_ARCH_CORTEXM0) || defined(CONFIG_ARCH_ARMV7M) || \
    defined(CONFIG_ARCH_ARMV8M)

  /* If the floating point unit is present and enabled, then save the
   * floating point registers as well as normal ARM registers.  This only
   * applies if "lazy" floating point register save/restore is used
   */

#  if defined(CONFIG_ARCH_FPU) && defined(CONFIG_ARMV7M_LAZYFPU)
#    define up_savestate(regs)  arm_copyarmstate(regs, (uint32_t*)CURRENT_REGS)
#  else
#    define up_savestate(regs)  arm_copyfullstate(regs, (uint32_t*)CURRENT_REGS)
#  endif
#  define up_restorestate(regs) (CURRENT_REGS = regs)

/* The Cortex-A and Cortex-R support the same mechanism, but only lazy
 * floating point register save/restore.
 */

#elif defined(CONFIG_ARCH_ARMV7A) || defined(CONFIG_ARCH_ARMV7R)

  /* If the floating point unit is present and enabled, then save the
   * floating point registers as well as normal ARM registers.
   */

#  if defined(CONFIG_ARCH_FPU)
#    define up_savestate(regs)  arm_copyarmstate(regs, (uint32_t*)CURRENT_REGS)
#  else
#    define up_savestate(regs)  arm_copyfullstate(regs, (uint32_t*)CURRENT_REGS)
#  endif
#  define up_restorestate(regs) (CURRENT_REGS = regs)

/* Otherwise, for the ARM7 and ARM9.  The state is copied in full from stack
 * to stack.  This is not very efficient and should be fixed to match
 * Cortex-A5.
 */

#else

  /* If the floating point unit is present and enabled, then save the
   * floating point registers as well as normal ARM registers.  Only "lazy"
   * floating point save/restore is supported.
   */

#  if defined(CONFIG_ARCH_FPU)
#    define up_savestate(regs)  arm_copyarmstate(regs, (uint32_t*)CURRENT_REGS)
#  else
#    define up_savestate(regs)  arm_copyfullstate(regs, (uint32_t*)CURRENT_REGS)
#  endif
#  define up_restorestate(regs) arm_copyfullstate((uint32_t*)CURRENT_REGS, regs)

#endif

/* Toolchain dependent, linker defined section addresses */

#if defined(__ICCARM__)
#  define _START_TEXT  __sfb(".text")
#  define _END_TEXT    __sfe(".text")
#  define _START_BSS   __sfb(".bss")
#  define _END_BSS     __sfe(".bss")
#  define _DATA_INIT   __sfb(".data_init")
#  define _START_DATA  __sfb(".data")
#  define _END_DATA    __sfe(".data")
#else
#  define _START_TEXT  &_stext
#  define _END_TEXT    &_etext
#  define _START_BSS   &_sbss
#  define _END_BSS     &_ebss
#  define _DATA_INIT   &_eronly
#  define _START_DATA  &_sdata
#  define _END_DATA    &_edata
#endif

/* This is the value used to mark the stack for subsequent stack monitoring
 * logic.
 */

#define STACK_COLOR    0xdeadbeef
#define INTSTACK_COLOR 0xdeadbeef
#define HEAP_COLOR     'h'

/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifndef __ASSEMBLY__
typedef void (*up_vector_t)(void);
#endif

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__
#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/* g_current_regs[] holds a references to the current interrupt level
 * register storage structure.  If is non-NULL only during interrupt
 * processing.  Access to g_current_regs[] must be through the macro
 * CURRENT_REGS for portability.
 */

#ifdef CONFIG_SMP
/* For the case of architectures with multiple CPUs, then there must be one
 * such value for each processor that can receive an interrupt.
 */

int up_cpu_index(void); /* See include/nuttx/arch.h */
EXTERN volatile uint32_t *g_current_regs[CONFIG_SMP_NCPUS];
#  define CURRENT_REGS (g_current_regs[up_cpu_index()])

#else

EXTERN volatile uint32_t *g_current_regs[1];
#  define CURRENT_REGS (g_current_regs[0])

#endif

/* This is the beginning of heap as provided from up_head.S.
 * This is the first address in DRAM after the loaded
 * program+bss+idle stack.  The end of the heap is
 * CONFIG_RAM_END
 */

EXTERN const uint32_t g_idle_topstack;

/* Address of the saved user stack pointer */

#if CONFIG_ARCH_INTERRUPTSTACK > 3
EXTERN uint32_t g_intstackalloc; /* Allocated stack base */
EXTERN uint32_t g_intstackbase;  /* Initial top of interrupt stack */
#endif

/* These 'addresses' of these values are setup by the linker script.  They
 * are not actual uint32_t storage locations! They are only used
 * meaningfully in the following way:
 *
 *  - The linker script defines, for example, the symbol_sdata.
 *  - The declareion extern uint32_t _sdata; makes C happy.  C will believe
 *    that the value _sdata is the address of a uint32_t variable _data (it
 *    is not!).
 *  - We can recoved the linker value then by simply taking the address of
 *    of _data.  like:  uint32_t *pdata = &_sdata;
 */

EXTERN uint32_t _stext;           /* Start of .text */
EXTERN uint32_t _etext;           /* End_1 of .text + .rodata */
EXTERN const uint32_t _eronly;    /* End+1 of read only section (.text + .rodata) */
EXTERN uint32_t _sdata;           /* Start of .data */
EXTERN uint32_t _edata;           /* End+1 of .data */
EXTERN uint32_t _sbss;            /* Start of .bss */
EXTERN uint32_t _ebss;            /* End+1 of .bss */

/* Sometimes, functions must be executed from RAM.  In this case, the
 * following macro may be used (with GCC!) to specify a function that will
 * execute from RAM.  For example,
 *
 *   int __ramfunc__ foo (void);
 *   int __ramfunc__ foo (void) { return bar; }
 *
 * will create a function named foo that will execute from RAM.
 */

#ifdef CONFIG_ARCH_RAMFUNCS

#  define __ramfunc__ __attribute__ ((section(".ramfunc"),long_call,noinline))

/* Functions declared in the .ramfunc section will be packaged together
 * by the linker script and stored in FLASH.  During boot-up, the start
 * logic must include logic to copy the RAM functions from their storage
 * location in FLASH to their correct destination in SRAM.  The following
 * following linker-defined values provide the information to copy the
 * functions from flash to RAM.
 */

EXTERN const uint32_t _framfuncs; /* Copy source address in FLASH */
EXTERN uint32_t _sramfuncs;       /* Copy destination start address in RAM */
EXTERN uint32_t _eramfuncs;       /* Copy destination end address in RAM */

#else /* CONFIG_ARCH_RAMFUNCS */

/* Otherwise, a null definition is provided so that condition compilation is
 * not necessary in code that may operate with or without RAM functions.
 */

#  define __ramfunc__

#endif /* CONFIG_ARCH_RAMFUNCS */
#endif /* __ASSEMBLY__ */

/****************************************************************************
 * Inline Functions
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifndef __ASSEMBLY__

/* Low level initialization provided by board-level logic *******************/

void arm_boot(void);

/* Context switching */

void arm_copyfullstate(uint32_t *dest, uint32_t *src);
#ifdef CONFIG_ARCH_FPU
void arm_copyarmstate(uint32_t *dest, uint32_t *src);
#endif
void up_decodeirq(uint32_t *regs);
int  arm_saveusercontext(uint32_t *saveregs);
void arm_fullcontextrestore(uint32_t *restoreregs) noreturn_function;
void arm_switchcontext(uint32_t *saveregs, uint32_t *restoreregs);

/* Signal handling **********************************************************/

void up_sigdeliver(void);

/* Power management *********************************************************/

#ifdef CONFIG_PM
void up_pminitialize(void);
#else
#  define up_pminitialize()
#endif

/* Interrupt handling *******************************************************/

/* Exception handling logic unique to the Cortex-M family */

#if defined(CONFIG_ARCH_CORTEXM0) || defined(CONFIG_ARCH_ARMV7M) || \
    defined(CONFIG_ARCH_ARMV8M)

/* Interrupt acknowledge and dispatch */

void up_ack_irq(int irq);
uint32_t *up_doirq(int irq, uint32_t *regs);

/* Exception Handlers */

int  up_svcall(int irq, FAR void *context, FAR void *arg);
int  up_hardfault(int irq, FAR void *context, FAR void *arg);

#  if defined(CONFIG_ARCH_ARMV7M) || defined(CONFIG_ARCH_ARMV8M)

int  up_memfault(int irq, FAR void *context, FAR void *arg);

#  endif /* CONFIG_ARCH_CORTEXM3,4,7 */

/* Exception handling logic unique to the Cortex-A and Cortex-R families
* (but should be back-ported to the ARM7 and ARM9 families).
 */

#elif defined(CONFIG_ARCH_ARMV7A) || defined(CONFIG_ARCH_ARMV7R)

/* Interrupt acknowledge and dispatch */

uint32_t *arm_doirq(int irq, uint32_t *regs);

/* Paging support */

#ifdef CONFIG_PAGING
void arm_pginitialize(void);
uint32_t *arm_va2pte(uintptr_t vaddr);
#else /* CONFIG_PAGING */
# define up_pginitialize()
#endif /* CONFIG_PAGING */

/* Exception Handlers */

uint32_t *arm_dataabort(uint32_t *regs, uint32_t dfar, uint32_t dfsr);
uint32_t *arm_prefetchabort(uint32_t *regs, uint32_t ifar, uint32_t ifsr);
uint32_t *arm_syscall(uint32_t *regs);
uint32_t *arm_undefinedinsn(uint32_t *regs);

/* Exception handling logic common to other ARM7 and ARM9 family. */

#else /* ARM7 | ARM9 */

/* Interrupt acknowledge and dispatch */

void up_ack_irq(int irq);
void up_doirq(int irq, uint32_t *regs);

/* Paging support (and exception handlers) */

#ifdef CONFIG_PAGING
void up_pginitialize(void);
uint32_t *up_va2pte(uintptr_t vaddr);
void up_dataabort(uint32_t *regs, uint32_t far, uint32_t fsr);
#else /* CONFIG_PAGING */
# define up_pginitialize()
void up_dataabort(uint32_t *regs);
#endif /* CONFIG_PAGING */

/* Exception handlers */

void up_prefetchabort(uint32_t *regs);
void up_syscall(uint32_t *regs);
void up_undefinedinsn(uint32_t *regs);

#endif /* CONFIG_ARCH_CORTEXM0,3,4,7 */

void up_vectorundefinsn(void);
void up_vectorswi(void);
void up_vectorprefetch(void);
void up_vectordata(void);
void up_vectoraddrexcptn(void);
void up_vectorirq(void);
void up_vectorfiq(void);

/* Floating point unit ******************************************************/

#ifdef CONFIG_ARCH_FPU
void up_savefpu(uint32_t *regs);
void up_restorefpu(const uint32_t *regs);
#else
#  define up_savefpu(regs)
#  define up_restorefpu(regs)
#endif

/* Low level serial output **************************************************/

void up_lowputc(char ch);
void up_puts(const char *str);
void up_lowputs(const char *str);

#ifdef USE_SERIALDRIVER
void up_serialinit(void);
#endif

#ifdef USE_EARLYSERIALINIT
void up_earlyserialinit(void);
#endif

#ifdef CONFIG_RPMSG_UART
void rpmsg_serialinit(void);
#endif

#ifdef CONFIG_ARM_LWL_CONSOLE
/* Defined in src/common/arm_lwl_console.c */

void lwlconsole_init(void);
#endif

/* DMA **********************************************************************/

#ifdef CONFIG_ARCH_DMA
void weak_function up_dma_initialize(void);
#endif

/* Cache control ************************************************************/

#ifdef CONFIG_ARCH_L2CACHE
void up_l2ccinitialize(void);
#else
#  define up_l2ccinitialize()
#endif

/* Memory management ********************************************************/

#if CONFIG_MM_REGIONS > 1
void up_addregion(void);
#else
# define up_addregion()
#endif

/* Watchdog timer ***********************************************************/

void up_wdtinit(void);

/* Networking ***************************************************************/

/* Defined in board/xyz_network.c for board-specific Ethernet
 * implementations, or chip/xyx_ethernet.c for chip-specific Ethernet
 * implementations, or common/arm_etherstub.c for a corner case where the
 * network is enabled yet there is no Ethernet driver to be initialized.
 *
 * Use of common/arm_etherstub.c is deprecated.  The preferred mechanism is
 * to use CONFIG_NETDEV_LATEINIT=y to suppress the call to up_netinitialize()
 * in up_initialize().  Then this stub would not be needed.
 */

#if defined(CONFIG_NET) && !defined(CONFIG_NETDEV_LATEINIT)
void up_netinitialize(void);
#else
# define up_netinitialize()
#endif

/* USB **********************************************************************/

#ifdef CONFIG_USBDEV
void up_usbinitialize(void);
void up_usbuninitialize(void);
#else
# define up_usbinitialize()
# define up_usbuninitialize()
#endif

/* Debug ********************************************************************/
#ifdef CONFIG_STACK_COLORATION
void up_stack_color(FAR void *stackbase, size_t nbytes);
#endif

#undef EXTERN
#ifdef __cplusplus
}
#endif
#endif /* __ASSEMBLY__ */

#endif /* __ARCH_ARM_SRC_COMMON_ARM_INTERNAL_H */