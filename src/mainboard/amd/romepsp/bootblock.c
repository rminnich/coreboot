/* SPDX-License-Identifier: GPL-2.0-only */

#include <stdint.h>
#include <device/pci_ops.h>
#include <bootblock_common.h>
#include <console/console.h>
#include <cpu/x86/msr.h>
#include <cpu/amd/msr.h>
#include <cpu/x86/mtrr.h>
#include <cpu/amd/mtrr.h>
#include <console/console.h>
#include <console/uart.h>

#include <commonlib/helpers.h>
#include <device/mmio.h>
#include <soc/southbridge.h>
#if 0
static void bootblock_northbridge_init(void)
{
	print_func_entry();
	printk(BIOS_SPEW, "%s\n", __func__);
	print_func_exit();
}

static void enable_spi_prefetch(void)
{
	print_func_entry();
	printk(BIOS_SPEW, "%s\n", __func__);
	print_func_exit();
}

static void bootblock_southbridge_init(void)
{
	print_func_entry();
	printk(BIOS_SPEW, "%s\n", __func__);
	enable_spi_prefetch();
	print_func_exit();
}

void old_bootblock_soc_init(void)
{
	print_func_entry();
	printk(BIOS_SPEW, "%s\n", __func__);
	bootblock_northbridge_init();
	bootblock_southbridge_init();
	print_func_exit();
}
#endif
void bootblock_mainboard_early_init(void)
{
	post_code(0x1d);

	post_code(0x1e);

	post_code(0x1f);
}

// Another fucking weak symbol.

#if 0
static void set_caching(void)
{
	msr_t deftype = {0, 0};
	int mtrr;
	/* Disable fixed and variable MTRRs while we setup */
	wrmsr(MTRR_DEF_TYPE_MSR, deftype);

	clear_all_var_mtrr();

	mtrr = get_free_var_mtrr();
	if (mtrr >= 0)
		set_var_mtrr(mtrr, FLASH_BASE_ADDR, CONFIG_ROM_SIZE, MTRR_TYPE_WRPROT);

	mtrr = get_free_var_mtrr();
	if (mtrr >= 0)
		set_var_mtrr(mtrr, (unsigned int)_bootblock, REGION_SIZE(bootblock),
			     MTRR_TYPE_WRBACK);

	/* Enable variable MTRRs. Fixed MTRRs are left disabled since they are not used. */
	deftype.lo |= MTRR_DEF_TYPE_EN | MTRR_TYPE_UNCACHEABLE;
	wrmsr(MTRR_DEF_TYPE_MSR, deftype);

	enable_cache();
}
#endif

#if 0
asmlinkage void bootblock_c_entry(uint64_t base_timestamp)
{
  	set_caching();
	//  	enable_pci_mmconf();

	bootblock_main_with_basetime(base_timestamp);
}

void bootblock_soc_early_init(void)
{
	fch_pre_init();
	// OK to here.
}

void bootblock_soc_init(void)
{
	u32 val = cpuid_eax(1);
	printk(BIOS_DEBUG, "Family_Model: %08x\n", val);

	fch_early_init();
}

#endif
