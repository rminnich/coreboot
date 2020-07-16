//* SPDX-License-Identifier: GPL-2.0-only */

#include <console/console.h>
#include <console/uart.h>
#include <string.h>
#include <ctype.h>
#include <arch/io.h>
#include <cpu/cpu.h>
#include <cpu/x86/msr.h>
#include <cpu/amd/msr.h>
#include <lib.h>

#include <device/device.h>
#include "serialice.h"

/* SIO functions */
/* While coreboot has some equivalent functions, we preserve
 * these to allow maximal compatibility between SerialICE
 * code and the original; and as an API security blanket,
 * as SerialICE expects slightly different IO behavior
 * than coreboot APIs provide (see: sio_getc).
 * coreboot serial APIs were never designed for interactive
 * use. */

void sio_putc(u8 data)
{
	uart_tx_byte(0, data);
}

u8 sio_getc(void)
{
	/* uart_rx_byte has a short timeout
	 * and will return a NULL. Serial
	 * ICE has little use for that,
	 * so this function will only return
	 * non-NULL characters. */
	u8 v;

	for (v = uart_rx_byte(0); !v; v = uart_rx_byte(0))
		;
	sio_putc(v);
	return v;
}

/* SIO helpers */

void sio_putstring(const char *string)
{
	/* Very simple, no %d, %x etc. */
	while (*string) {
		if (*string == '\n')
			sio_putc('\r');
		sio_putc(*string);
		string++;
	}
}

#define sio_put_nibble(nibble)                                                                 \
	if (nibble > 9)                                                                        \
		nibble += ('a' - 10);                                                          \
	else                                                                                   \
		nibble += '0';                                                                 \
	sio_putc(nibble)

void sio_put8(u8 data)
{
	u8 c;

	c = (data >> 4) & 0xf;
	sio_put_nibble(c);

	c = data & 0xf;
	sio_put_nibble(c);
}

void sio_put16(u16 data)
{
	int i;
	for (i = 12; i >= 0; i -= 4) {
		u8 c = (data >> i) & 0xf;
		sio_put_nibble(c);
	}
}

void sio_put32(u32 data)
{
	int i;
	for (i = 28; i >= 0; i -= 4) {
		u8 c = (data >> i) & 0xf;
		sio_put_nibble(c);
	}
}

u8 sio_get_nibble(void)
{
	u8 ret = 0;
	u8 nibble = sio_getc();

	if (nibble >= '0' && nibble <= '9') {
		ret = (nibble - '0');
	} else if (nibble >= 'a' && nibble <= 'f') {
		ret = (nibble - 'a') + 0xa;
	} else if (nibble >= 'A' && nibble <= 'F') {
		ret = (nibble - 'A') + 0xa;
	} else {
		sio_putstring("ERROR: parsing number\n");
	}
	return ret;
}

u8 sio_get8(void)
{
	u8 data;
	data = sio_get_nibble();
	data = data << 4;
	data |= sio_get_nibble();
	return data;
}

u16 sio_get16(void)
{
	u16 data;

	data = sio_get_nibble();
	data = data << 4;
	data |= sio_get_nibble();
	data = data << 4;
	data |= sio_get_nibble();
	data = data << 4;
	data |= sio_get_nibble();

	return data;
}

u32 sio_get32(void)
{
	u32 data;

	data = sio_get_nibble();
	data = data << 4;
	data |= sio_get_nibble();
	data = data << 4;
	data |= sio_get_nibble();
	data = data << 4;
	data |= sio_get_nibble();
	data = data << 4;
	data |= sio_get_nibble();
	data = data << 4;
	data |= sio_get_nibble();
	data = data << 4;
	data |= sio_get_nibble();
	data = data << 4;
	data |= sio_get_nibble();

	return data;
}
