/* SPDX-License-Identifier: GPL-2.0-only */
#include <console/console.h>
#include <cbmem.h>
#include <arch/io.h>
#include <arch/romstage.h>
#include <cpu/cpu.h>
#include <cpu/x86/msr.h>
#include <cpu/amd/msr.h>
#include "memory.h"

/* these are in mtrr.h and I'm not ready for that. */
#define TOP_MEM		0xC001001Aul
#define TOP_MEM2	0xC001001Dul

#define TOP_MEM_MASK			0x007fffff
#define TOP_MEM_MASK_KB			(TOP_MEM_MASK >> 10)

static uintptr_t tolmk(void) {
	msr_t msr;
	uintptr_t tomk;

	msr = rdmsr(TOP_MEM);
	printk(BIOS_ERR, "TOP %x:%x\n", msr.hi,msr.lo);
	tomk = (msr.lo>>10)&TOP_MEM_MASK;
	printk(BIOS_DEBUG, "QEMU: cmos: %lu MiB RAM below 4G.\n", tomk / 1024);
	return tomk;

}
void *cbmem_top_chipset(void)
{
	uintptr_t top = tolmk() * 1024;

	printk(BIOS_ERR, "CBMEM_TOP_CHIPSET returns %#lx\n", top);
	return (void *)top;
}

/* Nothing to do, MTRRs are no-op on QEMU. */
void fill_postcar_frame(struct postcar_frame *pcf)
{
}

