/* SPDX-License-Identifier: GPL-2.0-only */

#include <device/pci_ops.h>
#include <bootblock_common.h>
#include <southbridge/intel/i82801ix/i82801ix.h>
#include <console/console.h>

static void bootblock_northbridge_init(void)
{
	printk(BIOS_SPEW, "%s\n", __func__);
}

static void enable_spi_prefetch(void)
{
	printk(BIOS_SPEW, "%s\n", __func__);
}

static void bootblock_southbridge_init(void)
{
	printk(BIOS_SPEW, "%s\n", __func__);
	enable_spi_prefetch();
}

void bootblock_soc_init(void)
{
	printk(BIOS_SPEW, "%s\n", __func__);
	bootblock_northbridge_init();
	bootblock_southbridge_init();
}
