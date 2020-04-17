/* SPDX-License-Identifier: GPL-2.0-only */
/* This file is part of the coreboot project. */

#include <arch/romstage.h>
#include <cbmem.h>
#include <console/console.h>
#include <southbridge/intel/i82801ix/i82801ix.h>
#include <device/pci_ops.h>

#define D0F0_PCIEXBAR_LO 0x60

static void mainboard_machine_check(void)
{
	printk(BIOS_ERR, "mainboard_machine_check says what\n");
}

void mainboard_romstage_entry(void)
{
//	i82801ix_early_init();

	mainboard_machine_check();

	cbmem_recovery(0);
}
