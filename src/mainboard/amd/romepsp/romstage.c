/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/romstage.h>
#include <cbmem.h>
#include <console/console.h>
#include <southbridge/intel/i82801ix/i82801ix.h>
#include <device/pci_ops.h>

static void mainboard_machine_check(void)
{
	print_func_entry();
	printk(BIOS_SPEW, "%s\n", __func__);
	print_func_exit();
}

void mainboard_romstage_entry(void)
{
	print_func_entry();
	printk(BIOS_SPEW, "%s\n", __func__);

	mainboard_machine_check();

	cbmem_recovery(0);
	print_func_exit();
}
