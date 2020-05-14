/* SPDX-License-Identifier: GPL-2.0-only */

#include <device/pci_ops.h>
#include <bootblock_common.h>
#include <southbridge/intel/i82801ix/i82801ix.h>
#include <console/console.h>

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

void bootblock_soc_init(void)
{
	print_func_entry();
	printk(BIOS_SPEW, "%s\n", __func__);
	bootblock_northbridge_init();
	bootblock_southbridge_init();
	print_func_exit();
}
