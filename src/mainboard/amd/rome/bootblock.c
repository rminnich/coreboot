/* SPDX-License-Identifier: GPL-2.0-only */

#include <bootblock_common.h>
#include <arch/io.h>
#include <console/uart.h>
/*
#include <baseboard/variants.h>
*/

void bootblock_mainboard_early_init(void)
{
	for(uint8_t i = 0xbb; ; i++) {
		for(int j = 0; j < 100000000; j++)
			;
		outb(i, 0x80);
		outb('0', 0x3f8);
	}
	/*
	size_t num_gpios;
	const struct soc_amd_gpio *gpios;

	if (!CONFIG(VBOOT_STARTS_BEFORE_BOOTBLOCK)) {
		gpios = variant_early_gpio_table(&num_gpios);
		program_gpios(gpios, num_gpios);
	}

	variant_pcie_gpio_configure();
	*/
}
