/* SPDX-License-Identifier: GPL-2.0-only */

#include <bootblock_common.h>
#include <arch/io.h>
#include <arch/mmio.h>
#include <console/uart.h>
/*
#include <baseboard/variants.h>
*/
static uint32_t fuck = 0xaa5555aa;

static inline void postme(const char *s) {
	for(uint8_t f = *s++; f; f = *s++) {
	outb(f, 0x3f8);
		for(int j = 0; j < 100000000; j++)
			;
	}
}

void sio_put32(u32 data);

void bootblock_mainboard_early_init(void)
{
	uint8_t *cp = (void *)0xfedc9000;
	uint8_t x;
	void ice(void);

	outb('2', 0x3f8);
		for(int j = 0; j < 100000000; j++)
			;
	outb('3', 0x3f8);
		for(int j = 0; j < 100000000; j++)
			;
//		sio_put32((u32)&fuck);
	if (read32(&fuck) != 0xaa5555aa) postme("fuck is not 0xaa5555aa and should be\n"); else postme("f is 0xaa5555aa and should be\n");
	write32(&fuck, 0xdeadbeef);
	if (read32(&fuck) != 0xdeadbeef) postme("fuck is not 0xdeadbeef and should be\n"); else postme("f is 0xdeabeef and should be\n");
	if (read32(&fuck) == 0xaa5555aa) postme("fuck is 0xaa5555aa and should not be\n"); else postme("f is not 0xaa5555aa and should be\n");

	outb('4', 0x3f8);
		for(int j = 0; j < 100000000; j++)
			;
	outb('5', 0x3f8);
		for(int j = 0; j < 100000000; j++)
			;
	ice();
	for(uint8_t i = 0xbb; ; i++) {
		for(int j = 0; j < 100000000; j++)
			;
		outb(i, 0x80);
		outb('0', 0x3f8);
		write8(cp, '1');
		x = inb(0x3f8);
		if (x == 'f')
			break;
	}
	outb('2', 0x3f8);
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
