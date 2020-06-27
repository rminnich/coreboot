/*
 * SerialICE
 *
 * Copyright (C) 2009 coresystems GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */

#include <libpayload-config.h>
#include <libpayload.h>
#include <arch/apic.h>
#include <exception.h>
#include <arch/msr.h>
#include "linuxcheck.h"
/* SIO functions */

void sio_putc(u8 data)
{
	putchar(data);
}

u8 sio_getc(void)
{
	u8 v = getchar();
	putchar(v);
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

#define sio_put_nibble(nibble)	\
	if (nibble > 9)		\
		nibble += ('a' - 10);	\
	else			\
		nibble += '0';	\
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
	for (i=12; i >= 0; i -= 4) {
		u8 c = (data >> i) & 0xf;
		sio_put_nibble(c);
	}
}

void sio_put32(u32 data)
{
	int i;
	for (i=28; i >= 0; i -= 4) {
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
