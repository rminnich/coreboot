/* SPDX-License-Identifier: GPL-2.0-only */

#include <cbmem.h>
#include <arch/romstage.h>
#include <console/console.h>

static void mainboard_machine_check(void)
{
	print_func_entry();
	printk(BIOS_SPEW, "%s\n", __func__);
	print_func_exit();
}

void mainboard_romstage_entry(void)
{
	print_func_entry();

	mainboard_machine_check();

	cbmem_recovery(0);
	print_func_exit();
}
