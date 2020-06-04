/* SPDX-License-Identifier: GPL-2.0-only */

#include <bootblock_common.h>
#include <console/console.h>
#include <cpu/x86/bist.h>

asmlinkage void bootblock_c_entry_bist(uint64_t base_timestamp, uint32_t bist)
{
	print_func_entry();
	post_code(0x55);

	/* Halt if there was a built in self test failure */
	if (0 && bist) {
		console_init();
		report_bist_failure(bist);
	}
	post_code(0x06);
	/* Call lib/bootblock.c main */
	bootblock_main_with_basetime(base_timestamp);
	post_code(0x66);
	while (1);
	print_func_exit();
}
